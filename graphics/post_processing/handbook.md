# 后处理与渲染技术学习手册

> **写在前面的话**
>
> 如果你已经能写出 PBR 光照，但画面看起来总是"差一口气"——不够通透、不够电影感、边缘有锯齿——那问题往往不在光照本身，而在**后处理**。
>
> 后处理是现代游戏画面的"最后一公里"。从 HDR 到 Bloom，从色调映射到抗锯齿，这些效果决定了最终画面的质感。
>
> 这份手册的思路是：**先建立框架，再逐个击破**。每个效果都讲"为什么需要它"和"代价是什么"，而不是堆砌代码。理解了思路和取舍，实现只是时间问题。

---

## 目录

- [第一章 后处理基础框架](#第一章-后处理基础框架)
- [第二章 HDR 与色调映射](#第二章-hdr-与色调映射)
- [第三章 Bloom（辉光）](#第三章-bloom辉光)
- [第四章 景深（Depth of Field）](#第四章-景深depth-of-field)
- [第五章 运动模糊（Motion Blur）](#第五章-运动模糊motion-blur)
- [第六章 抗锯齿：MSAA / FXAA / TAA](#第六章-抗锯齿msaa--fxaa--taa)
- [第七章 色彩校正与 LUT](#第七章-色彩校正与-lut)
- [第八章 延迟渲染下的后处理](#第八章-延迟渲染下的后处理)
- [第九章 搭建一条最小可用后处理链](#第九章-搭建一条最小可用后处理链)
- [附录 后处理效果速查表](#附录-后处理效果速查表)

---

## 第一章 后处理基础框架

### 1.1 什么是后处理

**后处理（Post-Processing）**：在场景渲染完成后，对整张画面（一张纹理）做图像处理，得到最终输出。

```
场景渲染                    后处理
┌──────────┐              ┌──────────────────────────────┐
│ 3D 场景  │──渲染到纹理──→│ 全屏效果 1 → 效果 2 → ... → 输出 │
│          │              │                              │
└──────────┘              └──────────────────────────────┘
```

核心思想：**把 3D 渲染问题变成 2D 图像处理问题。**

### 1.2 渲染到纹理（Render Target）

#### 原理

不直接渲染到屏幕，而是渲染到一张**离屏纹理（Off-screen Texture）**：

```
正常渲染：
  GPU → 帧缓冲区 → 屏幕

渲染到纹理：
  GPU → 纹理 A（离屏） → 后处理 → 纹理 B → 屏幕
```

在 Vulkan 中，这通过 `VkFramebuffer` 实现：

```cpp
// 创建一个离屏纹理作为渲染目标
VkImageCreateInfo imageInfo = {};
imageInfo.imageType = VK_IMAGE_TYPE_2D;
imageInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;  // HDR 格式
imageInfo.extent.width = screenWidth;
imageInfo.extent.height = screenHeight;
imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT  // 可以作为渲染目标
                | VK_IMAGE_USAGE_SAMPLED_BIT;          // 可以作为纹理采样

// 创建帧缓冲区
VkFramebufferCreateInfo framebufferInfo = {};
framebufferInfo.renderPass = renderPass;
framebufferInfo.attachmentCount = 1;
framebufferInfo.pAttachments = &colorImageView;
framebufferInfo.width = screenWidth;
framebufferInfo.height = screenHeight;
framebufferInfo.layers = 1;
```

#### 为什么需要渲染到纹理

- 后处理需要**整张画面**作为输入
- 某些效果需要**多张纹理**（G-Buffer、深度缓冲、运动向量等）
- 可以**链式处理**：一个效果的输出是下一个效果的输入

### 1.3 全屏三角形/四边形

后处理 Shader 需要画一个覆盖整个屏幕的几何体，然后对每个像素做处理。

#### 全屏四边形 vs 全屏三角形

```
全屏四边形（2 个三角形）：      全屏三角形（1 个三角形）：
  ┌────────┬────────┐            ┌────────────────────┐
  │       /        │            │╲                   │
  │      /         │            │  ╲                 │
  │     /          │            │    ╲               │
  │    /           │            │      ╲             │
  │   /            │            │        ╲           │
  │  /             │            │          ╲         │
  │ /              │            │            ╲       │
  └────────────────┘            └────────────────────┘
  4 个顶点，6 个索引              3 个顶点，无索引
```

全屏三角形更简单高效（少一个顶点，少一次裁剪），是现代引擎的首选。

#### 顶点着色器

```glsl
// 全屏三角形：不需要顶点缓冲区，直接用顶点索引生成 UV
void main() {
    // gl_VertexIndex: 0, 1, 2
    vec2 positions[3] = vec2[](
        vec2(-1.0, -3.0),  // 左下（超出屏幕）
        vec2(-1.0,  1.0),  // 左上
        vec2( 3.0,  1.0)   // 右上（超出屏幕）
    );
    
    vec2 uvs[3] = vec2[](
        vec2(0.0, 0.0),
        vec2(0.0, 1.0),
        vec2(2.0, 2.0)
    );
    
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragUV = uvs[gl_VertexIndex];
}
```

#### 片元着色器（示例：简单的后处理框架）

```glsl
layout(binding = 0) uniform sampler2D sceneColor;

void main() {
    vec2 uv = fragUV;
    vec3 color = texture(sceneColor, uv).rgb;
    
    // 在这里做后处理
    // color = applyEffect(color);
    
    outColor = vec4(color, 1.0);
}
```

### 1.4 后处理链（Post-Processing Chain）

多个后处理效果按顺序串联，前一个的输出是后一个的输入：

```
场景渲染
    │
    ▼
┌──────────┐
│ 纹理 A   │ (HDR 场景颜色)
└────┬─────┘
     │
     ▼
┌──────────┐
│ Bloom    │ → 提取亮区，模糊，叠加
└────┬─────┘
     │
     ▼
┌──────────┐
│ 色调映射  │ → HDR → LDR
└────┬─────┘
     │
     ▼
┌──────────┐
│ 色彩校正  │ → LUT 调色
└────┬─────┘
     │
     ▼
┌──────────┐
│ 抗锯齿   │ → TAA / FXAA
└────┬─────┘
     │
     ▼
┌──────────┐
│ 输出到屏幕│
└──────────┘
```

#### 顺序很重要

不同效果的顺序会影响最终画面：

| 顺序 | 原因 |
|------|------|
| Bloom → 色调映射 → 色彩校正 → 抗锯齿 | Bloom 需要在 HDR 空间做，色调映射后变成 LDR |
| 景深 → Bloom | 景深模糊后，亮区会扩散，Bloom 更自然 |
| 运动模糊 → 色调映射 | 运动模糊需要在 HDR 空间做 |

**一般原则**：
- 需要 HDR 数据的效果（Bloom、运动模糊）在前面
- 色调映射在中间（HDR → LDR 的转折点）
- 色彩校正和抗锯齿在最后

### 1.5 性能代价

后处理不是免费的。每个效果都需要：
- **显存带宽**：读写全屏纹理
- **GPU 计算**：片元着色器执行
- **额外的渲染目标**：某些效果需要多张中间纹理

```
性能优化策略：
  1. 降分辨率：后处理纹理可以是半分辨率（1/2 宽高）
  2. 按需开关：移动端可以关掉昂贵的效果
  3. 合并 Pass：多个效果可以在一个 Shader 中完成
```

---

## 第二章 HDR 与色调映射

### 2.1 为什么需要 HDR

#### 问题：真实世界的光照范围远超显示器

真实世界的亮度范围（动态范围）非常大：

```
场景                 亮度 (cd/m²)
─────────────────────────────────
月光下的室外          0.1
室内客厅              100
正午太阳              100,000
直视太阳              1,000,000
```

但显示器的亮度范围通常只有 0~300 cd/m²。如果直接把场景颜色渲染到显示器，要么亮部过曝（全白），要么暗部欠曝（全黑）。

#### 解决方案：HDR 渲染 + 色调映射

```
HDR 渲染管线：
  1. 场景渲染到 HDR 纹理（每个通道 16 位浮点，范围 [0, +∞)）
  2. 后处理（Bloom 等）在 HDR 空间进行
  3. 色调映射：把 HDR 压缩到 LDR [0, 1]
  4. 输出到显示器
```

```
LDR 管线：
  场景 → 直接渲染到 LDR [0, 1] → 输出
  
  问题：亮部细节丢失，Bloom 无法正确提取亮区
```

### 2.2 HDR 纹理格式

| 格式 | 每通道位数 | 范围 | 用途 |
|------|-----------|------|------|
| `R8G8B8A8_UNORM` | 8 | [0, 1] | LDR，普通纹理 |
| `R16G16B16A16_SFLOAT` | 16 | [0, +∞) | HDR 渲染目标 |
| `R11G11B10_UFLOAT` | 11/11/10 | [0, +∞) | HDR，更省显存 |
| `R32G32B32A32_SFLOAT` | 32 | [0, +∞) | 高精度计算 |

游戏渲染目标通常用 `R16G16B16A16_SFLOAT` 或 `R11G11B10_UFLOAT`。

### 2.3 色调映射（Tone Mapping）

#### 核心问题

把 HDR 值 $[0, +\infty)$ 映射到 LDR 值 $[0, 1]$，同时尽量保留视觉信息。

#### 常见的色调映射算法

##### Reinhard（最简单）

$$L_{\text{out}} = \frac{L_{\text{in}}}{L_{\text{in}} + 1}$$

```glsl
vec3 reinhard(vec3 hdr) {
    return hdr / (hdr + 1.0);
}
```

```
  1.0 ┤                              ╭────────
      │                           ╱
  0.5 ┤                        ╱
      │                     ╱
      │                  ╱
  0.0 ┤───────────────╱───────────────
      0              1              2
      输入 HDR 值
      
  特点：简单，但对比度低，画面发灰
```

##### ACES（电影感，业界标准）

ACES（Academy Color Encoding System）是好莱坞用的色调映射，对比度高，色彩饱和。

```glsl
vec3 aces(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}
```

```
  1.0 ┤                          ╭────────
      │                        ╱
  0.5 ┤                     ╱╱
      │                   ╱
      │                ╱╱
  0.0 ┤─────────────╱────────────────
      0             1              2
      
  特点：对比度高，暗部压得低，亮部过渡自然
```

##### Unreal（游戏引擎常用）

```glsl
vec3 unreal(vec3 x) {
    return x / (x + 0.155) * 1.019;
}
```

#### 曝光控制

色调映射前，通常先做**曝光调整**，模拟相机的曝光：

```glsl
uniform float exposure;  // 曝光值，通常 0.5~2.0

void main() {
    vec3 hdrColor = texture(sceneColor, fragUV).rgb;
    
    // 曝光调整
    vec3 exposed = hdrColor * exposure;
    
    // 色调映射
    vec3 ldr = aces(exposed);
    
    // Gamma 校正
    ldr = pow(ldr, vec3(1.0 / 2.2));
    
    outColor = vec4(ldr, 1.0);
}
```

#### 自动曝光

让引擎自动调整曝光值，模拟人眼适应：

```cpp
// 每帧计算平均亮度
float avgLuminance = computeAverageLuminance(sceneColor);  // 对整张图求平均

// 目标曝光值
float targetExposure = 0.5 / avgLuminance;

// 平滑过渡（模拟人眼适应）
currentExposure = lerp(currentExposure, targetExposure, 0.05);
```

### 2.4 HDR 管线的完整流程

```
1. 场景渲染到 HDR 纹理 (R16G16B16A16)
   │
   ▼
2. Bloom（在 HDR 空间提取亮区，模糊，叠加）
   │
   ▼
3. 曝光调整 × 色调映射（HDR → LDR）
   │
   ▼
4. 色彩校正（LUT）
   │
   ▼
5. 抗锯齿（TAA/FXAA）
   │
   ▼
6. 输出到屏幕 (R8G8B8A8)
```

---

## 第三章 Bloom（辉光）

### 3.1 解决什么问题

真实世界中，非常亮的物体会"溢出"到周围区域，产生光晕效果。Bloom 模拟这种现象。

```
没有 Bloom：                  有 Bloom：
  ████                          ████
  ████                          ▓▓▓▓
  ████                          ░░░░
  亮区和暗区边界锐利             亮区周围有柔和的光晕
```

### 3.2 算法思路

```
步骤 1：提取亮区
  遍历每个像素，如果亮度 > 阈值，保留；否则丢弃
  
步骤 2：模糊亮区
  对提取出的亮区做高斯模糊（多级 Mipmap 模糊）
  
步骤 3：叠加回原图
  原图 + 模糊后的亮区
```

```
原图                    提取亮区                模糊后                 叠加
████░░░░     →         ████     →             ▓▓▓▓     →          ████
████░░░░                ████                    ░░░░                ▓▓▓▓
                                                ░░░░                ░░░░
```

### 3.3 关键步骤详解

#### 步骤 1：亮度提取

```glsl
uniform float bloomThreshold;  // 阈值，通常 0.8~1.5

void main() {
    vec3 color = texture(sceneColor, fragUV).rgb;
    
    // 计算亮度（人眼对绿色最敏感）
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    
    // 提取亮区
    if (luminance > bloomThreshold) {
        outColor = vec4(color, 1.0);
    } else {
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
```

#### 步骤 2：多级模糊（Kawase Blur / Dual Blur）

对亮区纹理做多次降采样 + 上采样，得到不同尺度的模糊：

```
原始亮区 (1024×1024)
    │
    ▼ 降采样 + 模糊
Mip 1 (512×512)      ← 小范围模糊
    │
    ▼ 降采样 + 模糊
Mip 2 (256×256)      ← 中等范围模糊
    │
    ▼ 降采样 + 模糊
Mip 3 (128×128)      ← 大范围模糊
    │
    ▼ 上采样 + 叠加
Mip 2 + Mip 3
    │
    ▼ 上采样 + 叠加
Mip 1 + (Mip 2 + Mip 3)
    │
    ▼ 上采样 + 叠加
最终模糊结果
```

```glsl
// 简化的 Kawase Blur（降采样）
vec4 kawaseBlurDown(sampler2D tex, vec2 uv, vec2 texelSize, float offset) {
    vec4 sum = vec4(0.0);
    sum += texture(tex, uv + vec2(-offset, -offset) * texelSize);
    sum += texture(tex, uv + vec2(-offset,  offset) * texelSize);
    sum += texture(tex, uv + vec2( offset, -offset) * texelSize);
    sum += texture(tex, uv + vec2( offset,  offset) * texelSize);
    return sum / 4.0;
}

// 上采样
vec4 kawaseBlurUp(sampler2D tex, vec2 uv, vec2 texelSize, float offset) {
    vec4 sum = vec4(0.0);
    sum += texture(tex, uv);
    sum += texture(tex, uv + vec2(-offset, -offset) * texelSize) * 0.5;
    sum += texture(tex, uv + vec2(-offset,  offset) * texelSize) * 0.5;
    sum += texture(tex, uv + vec2( offset, -offset) * texelSize) * 0.5;
    sum += texture(tex, uv + vec2( offset,  offset) * texelSize) * 0.5;
    return sum / 3.0;
}
```

#### 步骤 3：叠加

```glsl
uniform float bloomIntensity;  // Bloom 强度，通常 0.1~0.5

void main() {
    vec3 sceneColor = texture(sceneColor, fragUV).rgb;
    vec3 bloomColor = texture(bloomTexture, fragUV).rgb;
    
    vec3 result = sceneColor + bloomColor * bloomIntensity;
    outColor = vec4(result, 1.0);
}
```

### 3.4 性能代价

| 因素 | 影响 |
|------|------|
| 多级模糊 | 需要多次渲染到不同分辨率的纹理 |
| 纹理采样 | 每次模糊需要 4~9 次采样 |
| 显存 | 需要多张中间纹理 |

**优化**：
- 降分辨率：Bloom 纹理可以是半分辨率
- 减少 Mip 层级：3~4 级通常够用
- 用 Dual Blur 代替传统高斯模糊（更快）

### 3.5 典型参数

| 参数 | 典型值 | 说明 |
|------|--------|------|
| 阈值 | 0.8~1.5 | 低于这个亮度的不产生 Bloom |
| 强度 | 0.1~0.5 | Bloom 的强度 |
| 模糊半径 | 3~5 级 Mip | 控制 Bloom 的扩散范围 |

---

## 第四章 景深（Depth of Field）

### 4.1 解决什么问题

真实相机只能对焦在一个距离上，近处和远处都会模糊。景深（DOF）模拟这种效果。

```
相机对焦在中间的方块：
  
  近处模糊      中间清晰      远处模糊
  ░░░░          ████          ▒▒▒▒
  ░░░░          ████          ▒▒▒▒
```

### 4.2 算法思路

```
步骤 1：计算每个像素的模糊半径
  根据深度和焦距，计算 Circle of Confusion (CoC)
  
步骤 2：根据 CoC 做可变半径模糊
  CoC 大的地方模糊半径大
  
步骤 3：叠加清晰图和模糊图
```

### 4.3 Circle of Confusion (CoC)

CoC 表示一个点在成像平面上的模糊圆大小：

$$\text{CoC} = \frac{|d - f| \cdot A}{d \cdot (f - F)}$$

其中：
- $d$：像素深度
- $f$：焦距
- $F$：对焦距离
- $A$：光圈大小

简化版：

```glsl
uniform float focusDistance;  // 对焦距离
uniform float focusRange;     // 清晰范围
uniform float maxBlur;        // 最大模糊半径

float computeCoC(float depth) {
    float dist = abs(depth - focusDistance);
    float coc = clamp(dist / focusRange, 0.0, 1.0);
    return coc * maxBlur;
}
```

### 4.4 实现方法

#### 方法一：两遍高斯模糊（简单但效果一般）

```glsl
// 第一遍：水平模糊
void main() {
    float depth = texture(depthBuffer, fragUV).r;
    float coc = computeCoC(depth);
    
    vec3 sum = vec3(0.0);
    float weight = 0.0;
    
    for (int i = -4; i <= 4; i++) {
        float offset = float(i) * coc;
        vec2 uv = fragUV + vec2(offset, 0.0) * texelSize;
        sum += texture(sceneColor, uv).rgb;
        weight += 1.0;
    }
    
    outColor = vec4(sum / weight, 1.0);
}

// 第二遍：垂直模糊（类似）
```

#### 方法二：Hexagonal Bokeh（更真实，更贵）

用六边形核做模糊，产生六边形的光斑（Bokeh）：

```
传统高斯模糊：          Hexagonal Bokeh：
    ●                      ●
   ●●●                   ●●●●●
    ●                    ●●●●●●●
  圆形光斑                ●●●●●●●
                          ●●●●●
                           ●●●
                          六边形光斑
```

### 4.5 性能代价

| 因素 | 影响 |
|------|------|
| 可变半径模糊 | 每个像素的模糊半径不同，无法用固定的核 |
| 纹理采样 | 需要大量采样（16~64 次） |
| 深度纹理 | 需要读取深度缓冲 |

**优化**：
- 降分辨率：DOF 纹理可以是半分辨率
- 用 Tile-Based CoC：把屏幕分块，每块用最大 CoC
- 用 Bokeh Splatting：把每个像素当成一个光斑画出来

### 4.6 典型参数

| 参数 | 典型值 | 说明 |
|------|--------|------|
| 对焦距离 | 摄像机到目标角色的距离 | 通常自动计算 |
| 清晰范围 | 5~20 米 | 在这个范围内清晰 |
| 最大模糊半径 | 5~15 像素 | 控制模糊强度 |
| 光圈叶片数 | 6（六边形 Bokeh） | 影响光斑形状 |

---

## 第五章 运动模糊（Motion Blur）

### 5.1 解决什么问题

快速移动的物体在相机曝光期间会产生模糊。运动模糊模拟这种效果，增加速度感。

```
没有运动模糊：              有运动模糊：
  ████                        ░░████░░
  ████                        ░░████░░
  边缘锐利                    沿运动方向拖影
```

### 5.2 算法思路

```
步骤 1：计算每个像素的运动向量
  当前帧位置 - 上一帧位置 = 运动向量
  
步骤 2：沿运动向量做方向性模糊
  对每个像素，沿其运动向量采样多次，取平均
```

### 5.3 运动向量计算

```glsl
// 在顶点着色器中计算
out vec4 currentPos;   // 当前帧的裁剪空间位置
out vec4 previousPos;  // 上一帧的裁剪空间位置

void main() {
    vec4 worldPos = model * vec4(position, 1.0);
    
    currentPos = viewProj * worldPos;
    previousPos = previousViewProj * previousModel * vec4(position, 1.0);
    
    gl_Position = currentPos;
}

// 在片元着色器中输出运动向量
layout(location = 1) out vec2 motionVector;

void main() {
    // 透视除法
    vec2 ndcCurrent = (currentPos.xy / currentPos.w) * 0.5 + 0.5;
    vec2 ndcPrevious = (previousPos.xy / previousPos.w) * 0.5 + 0.5;
    
    motionVector = ndcCurrent - ndcPrevious;
}
```

### 5.4 方向性模糊

```glsl
uniform int sampleCount;  // 采样次数，通常 8~16

void main() {
    vec3 color = texture(sceneColor, fragUV).rgb;
    vec2 velocity = texture(motionVector, fragUV).rg;
    
    float speed = length(velocity);
    if (speed > 0.001) {
        vec2 dir = velocity / speed;
        
        vec3 sum = vec3(0.0);
        for (int i = 0; i < sampleCount; i++) {
            float t = float(i) / float(sampleCount - 1) - 0.5;
            vec2 uv = fragUV + dir * t * speed * motionBlurScale;
            sum += texture(sceneColor, uv).rgb;
        }
        color = sum / float(sampleCount);
    }
    
    outColor = vec4(color, 1.0);
}
```

### 5.5 性能代价

| 因素 | 影响 |
|------|------|
| 运动向量 | 需要额外的渲染 Pass 或 G-Buffer 通道 |
| 采样次数 | 每个像素需要 8~16 次采样 |
| 历史帧 | 需要缓存上一帧的数据 |

**优化**：
- 降分辨率：运动向量可以是半分辨率
- 自适应采样：速度快的地方多采样，慢的地方少采样
- Tile-Based：把屏幕分块，每块用最大速度

### 5.6 典型参数

| 参数 | 典型值 | 说明 |
|------|--------|------|
| 采样次数 | 8~16 | 越多越平滑，但越慢 |
| 模糊强度 | 0.5~1.0 | 控制拖影长度 |
| 最大速度限制 | 0.1~0.2 | 防止过度模糊 |

---

## 第六章 抗锯齿：MSAA / FXAA / TAA

### 6.1 为什么需要抗锯齿

光栅化时，三角形的边缘会产生**锯齿**（aliasing）：

```
没有抗锯齿：              有抗锯齿：
  ████░░░░                  ████▒▒░░
  ████░░░░                  ████▒▒░░
  ████░░░░                  ████▒▒░░
  边缘锐利，有锯齿           边缘平滑过渡
```

### 6.2 MSAA（Multi-Sample Anti-Aliasing）

#### 原理

在每个像素内取**多个采样点**，如果部分采样点在三角形内，部分在外，就取平均值。

```
MSAA 4x：每个像素 4 个采样点

  ┌───┬───┐
  │ ● │ ● │  ← 2 个在三角形内，2 个在外
  ├───┼───┤     → 颜色 = 50% 三角形颜色 + 50% 背景
  │ ● │ ░ │
  └───┴───┘
```

#### 优点与缺点

| 优点 | 缺点 |
|------|------|
| 硬件支持，效果好 | 只抗几何边缘，不抗 Shader 锯齿（如纹理、高光） |
| 不需要后处理 | 增加显存占用（4x = 4 倍） |
| | 延迟渲染不支持（G-Buffer 太大） |

#### Vulkan 配置

```cpp
VkPipelineMultisampleStateCreateInfo multisampling = {};
multisampling.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;  // 4x MSAA
multisampling.sampleShadingEnable = VK_FALSE;
```

### 6.3 FXAA（Fast Approximate Anti-Aliasing）

#### 原理

后处理抗锯齿。检测边缘（亮度差异大的区域），沿边缘方向做模糊。

```
步骤 1：检测边缘
  计算每个像素和周围像素的亮度差异
  
步骤 2：确定边缘方向
  水平边缘 or 垂直边缘
  
步骤 3：沿边缘方向模糊
```

```glsl
// 简化的 FXAA 思路
void main() {
    vec3 center = texture(sceneColor, fragUV).rgb;
    float lumCenter = dot(center, vec3(0.299, 0.587, 0.114));
    
    // 采样周围像素
    float lumN = dot(texture(sceneColor, fragUV + vec2(0, texelSize.y)).rgb, vec3(0.299, 0.587, 0.114));
    float lumS = dot(texture(sceneColor, fragUV - vec2(0, texelSize.y)).rgb, vec3(0.299, 0.587, 0.114));
    float lumE = dot(texture(sceneColor, fragUV + vec2(texelSize.x, 0)).rgb, vec3(0.299, 0.587, 0.114));
    float lumW = dot(texture(sceneColor, fragUV - vec2(texelSize.x, 0)).rgb, vec3(0.299, 0.587, 0.114));
    
    // 计算边缘强度
    float edgeH = abs(lumN - lumCenter) + abs(lumS - lumCenter);
    float edgeV = abs(lumE - lumCenter) + abs(lumW - lumCenter);
    
    // 根据边缘方向做模糊
    if (edgeH > edgeV) {
        // 水平边缘，垂直模糊
        outColor = mix(texture(sceneColor, fragUV), 
                       (texture(sceneColor, fragUV + vec2(0, texelSize.y)) +
                        texture(sceneColor, fragUV - vec2(0, texelSize.y))) / 2.0,
                       edgeStrength);
    } else {
        // 垂直边缘，水平模糊
        // ...
    }
}
```

#### 优点与缺点

| 优点 | 缺点 |
|------|------|
| 非常快，只需一次全屏 Pass | 会模糊纹理细节 |
| 支持延迟渲染 | 对快速运动的物体效果差 |
| 抗所有类型的锯齿 | |

### 6.4 TAA（Temporal Anti-Aliasing）

#### 原理

利用**时间信息**：当前帧和上一帧混合，利用历史采样提高质量。

```
步骤 1：Jitter（抖动）
  每帧的投影矩阵加一个微小的偏移（半个像素）
  
步骤 2：重投影
  把上一帧的像素重投影到当前帧
  
步骤 3：混合
  当前帧 + 历史帧 → 混合
```

```
帧 1：                    帧 2：                    混合后：
  ●                       ░●                        ▒●
  ●                       ●░                        ▒●
  采样点在 (0.5, 0.5)     采样点在 (0.0, 0.0)       利用两帧的信息
```

```glsl
uniform mat4 previousViewProj;
uniform sampler2D historyColor;  // 上一帧的颜色
uniform float blendFactor;       // 混合因子，通常 0.1~0.2

void main() {
    vec3 currentColor = texture(sceneColor, fragUV).rgb;
    
    // 计算运动向量
    float depth = texture(depthBuffer, fragUV).r;
    vec4 worldPos = inverseViewProj * vec4(fragUV * 2.0 - 1.0, depth, 1.0);
    vec4 previousPos = previousViewProj * worldPos;
    vec2 previousUV = (previousPos.xy / previousPos.w) * 0.5 + 0.5;
    
    // 采样历史帧
    vec3 historyColorSample = texture(historyColor, previousUV).rgb;
    
    // 混合
    vec3 result = mix(currentColor, historyColorSample, blendFactor);
    
    outColor = vec4(result, 1.0);
}
```

#### 优点与缺点

| 优点 | 缺点 |
|------|------|
| 效果最好，抗所有锯齿 | 需要历史帧，有 Ghosting（鬼影）问题 |
| 支持延迟渲染 | 快速运动时会出现拖影 |
| 比 MSAA 省显存 | 需要运动向量 |

#### Ghosting 问题

当物体快速移动时，历史帧的信息可能已经过时，导致拖影：

```
物体在帧 1 的位置：    物体在帧 2 的位置：    混合后：
  ████                    ████                  ░░████░░
  ████                    ████                  ░░████░░
                                                ↑ 鬼影
```

**解决方案**：
- 限制历史帧的权重（blendFactor 不要太大）
- 用深度/运动向量检测不连续区域，降低混合
- 用亮度钳制（clamp history color to current neighborhood）

### 6.5 三者对比

| 特性 | MSAA | FXAA | TAA |
|------|------|------|-----|
| 抗锯齿类型 | 几何边缘 | 所有 | 所有 |
| 性能 | 中等 | 快 | 中等 |
| 显存 | 高（4x = 4 倍） | 低 | 低 |
| 延迟渲染支持 | 否 | 是 | 是 |
| 模糊纹理 | 否 | 是 | 否 |
| Ghosting | 否 | 否 | 是 |
| 推荐场景 | 前向渲染 | 移动端 | PC/主机 |

### 6.6 典型参数

| 参数 | 典型值 | 说明 |
|------|--------|------|
| MSAA 采样数 | 2x / 4x | 8x 以上收益递减 |
| FXAA 边缘阈值 | 0.075~0.15 | 越小越敏感 |
| TAA 混合因子 | 0.1~0.2 | 越大越平滑，但 Ghosting 越严重 |
| TAA Jitter 幅度 | 0.5 像素 | 标准值 |

---

## 第七章 色彩校正与 LUT

### 7.1 解决什么问题

色调映射后，画面可能看起来"平淡"或"不够电影感"。色彩校正可以调整整体的色彩风格。

```
校正前：                  校正后：
  ████                      ████
  ████                      ▓▓▓▓
  ░░░░                      ▒▒▒▒
  对比度低，发灰              对比度高，色彩饱和
```

### 7.2 基本色彩调整

```glsl
uniform float contrast;    // 对比度，0.5~1.5
uniform float saturation;  // 饱和度，0.5~1.5
uniform float brightness;  // 亮度，-0.2~0.2

void main() {
    vec3 color = texture(sceneColor, fragUV).rgb;
    
    // 亮度
    color += brightness;
    
    // 对比度
    color = (color - 0.5) * contrast + 0.5;
    
    // 饱和度
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    color = mix(vec3(luminance), color, saturation);
    
    outColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
```

### 7.3 LUT（Look-Up Table）

#### 原理

LUT 是一张 3D 纹理，输入一个颜色，输出校正后的颜色。

```
3D LUT：
  
  R 轴 →
  │
  ├─ B=0 层：
  │   ┌────────┐
  │   │        │
  │   │  G 轴  │
  │   │  ↓     │
  │   └────────┘
  │
  ├─ B=0.5 层：
  │   ┌────────┐
  │   │        │
  │   │        │
  │   └────────┘
  │
  └─ B=1.0 层：
      ┌────────┐
      │        │
      │        │
      └────────┘
```

#### 使用

```glsl
uniform sampler3D lutTexture;  // 3D LUT 纹理

void main() {
    vec3 color = texture(sceneColor, fragUV).rgb;
    
    // 用颜色采样 LUT
    vec3 corrected = texture(lutTexture, color).rgb;
    
    outColor = vec4(corrected, 1.0);
}
```

#### LUT 的制作

1. 在 Photoshop / DaVinci Resolve 中调色一张标准 LUT 图
2. 导出为 `.cube` 或 `.png` 格式
3. 导入引擎，生成 3D 纹理

```
标准 LUT 图（256×16，每行是一个 B 值）：
  
  ┌────────────────────────────────┐
  │ ████████████████████ ← B=0     │
  │ ████████████████████ ← B=0.0625│
  │ ...                            │
  │ ████████████████████ ← B=1     │
  └────────────────────────────────┘
  每行内部：R 从左到右，G 从上到下
```

### 7.4 典型参数

| 参数 | 典型值 | 说明 |
|------|--------|------|
| 对比度 | 1.0~1.3 | 不要太高，会丢失细节 |
| 饱和度 | 0.9~1.2 | 根据美术风格调整 |
| LUT 强度 | 0.5~1.0 | 可以混合原始颜色和 LUT 颜色 |

---

## 第八章 延迟渲染下的后处理

### 8.1 延迟渲染的 G-Buffer

延迟渲染先渲染到 G-Buffer（多张纹理），然后再做光照计算：

```
G-Buffer：
  Attachment 0: Albedo (RGB) + 自发光 (A)
  Attachment 1: Normal (RGB) + 金属度 (A)
  Attachment 2: Specular (RGB) + 粗糙度 (A)
  Attachment 3: Depth (R32)
```

### 8.2 后处理的差异

#### 前向渲染

```
场景渲染 → HDR 颜色纹理 → 后处理 → 输出
```

#### 延迟渲染

```
几何 Pass → G-Buffer
                │
                ▼
光照 Pass（读取 G-Buffer）→ HDR 颜色纹理 → 后处理 → 输出
```

#### 关键差异

| 特性 | 前向渲染 | 延迟渲染 |
|------|----------|----------|
| MSAA | 支持 | 不支持（G-Buffer 太大） |
| 透明物体 | 天然支持 | 需要额外的前向 Pass |
| 后处理输入 | 只有颜色 | 可以用 G-Buffer 的额外信息（深度、法线等） |
| 性能 | 光照复杂时慢 | 光照复杂时快 |

### 8.3 延迟渲染的优势

某些后处理效果可以利用 G-Buffer 的信息：

#### 屏幕空间环境光遮蔽（SSAO）

```glsl
// 用深度和法线计算遮蔽
float depth = texture(gBufferDepth, fragUV).r;
vec3 normal = texture(gBufferNormal, fragUV).rgb;

// 在法线半球采样，检查是否被遮挡
float occlusion = 0.0;
for (int i = 0; i < SAMPLE_COUNT; i++) {
    vec3 samplePos = getSamplePosition(fragUV, depth, normal, i);
    float sampleDepth = texture(gBufferDepth, samplePos.xy).r;
    if (sampleDepth < samplePos.z) {
        occlusion += 1.0;
    }
}
occlusion /= SAMPLE_COUNT;
```

#### 屏幕空间反射（SSR）

```glsl
// 用深度做光线步进（Ray Marching）
vec3 viewDir = normalize(fragPos - viewPos);
vec3 reflectDir = reflect(-viewDir, normal);

vec3 hitPos = rayMarch(fragPos, reflectDir, depthBuffer);
vec3 reflectionColor = texture(sceneColor, projectToScreen(hitPos)).rgb;
```

### 8.4 透明物体的处理

延迟渲染不透明物体用 G-Buffer，透明物体需要额外的前向 Pass：

```
1. 几何 Pass（不透明）→ G-Buffer
2. 光照 Pass → HDR 颜色
3. 前向 Pass（透明）→ 叠加到 HDR 颜色
4. 后处理
```

---

## 第九章 搭建一条最小可用后处理链

### 9.1 最小后处理链

对于大多数游戏，以下 4 个效果就够了：

```
场景渲染 (HDR)
    │
    ▼
┌──────────┐
│ Bloom    │  ← 增加光晕，提升画面质感
└────┬─────┘
     │
     ▼
┌──────────┐
│ 色调映射  │  ← HDR → LDR，ACES 或 Unreal
└────┬─────┘
     │
     ▼
┌──────────┐
│ TAA      │  ← 抗锯齿（或 FXAA，如果性能紧张）
└────┬─────┘
     │
     ▼
┌──────────┐
│ 色彩校正  │  ← LUT 调色（可选）
└────┬─────┘
     │
     ▼
输出到屏幕
```

### 9.2 移动端精简版

移动端性能有限，可以精简为：

```
场景渲染 (HDR)
    │
    ▼
┌──────────┐
│ Bloom    │  ← 降分辨率，减少 Mip 层级
└────┬─────┘
     │
     ▼
┌──────────┐
│ 色调映射  │  ← 简单的 Reinhard 或自定义曲线
└────┬─────┘
     │
     ▼
┌──────────┐
│ FXAA     │  ← 比 TAA 快，不需要历史帧
└────┬─────┘
     │
     ▼
输出到屏幕
```

### 9.3 搭建步骤

#### 步骤 1：HDR 渲染目标

```cpp
// 创建 HDR 颜色纹理
VkFormat colorFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
createImage(screenWidth, screenHeight, colorFormat, 
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            &hdrColorImage, &hdrColorImageView);

// 创建帧缓冲区
createFramebuffer(renderPass, {hdrColorImageView}, &hdrFramebuffer);
```

#### 步骤 2：后处理 Pass

```cpp
// 每个后处理效果一个 Pass
for (auto& effect : postProcessChain) {
    vkCmdBeginRenderPass(commandBuffer, &effect.renderPassInfo, ...);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, effect.pipeline);
    vkCmdBindDescriptorSets(commandBuffer, ...);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);  // 全屏三角形
    vkCmdEndRenderPass(commandBuffer);
}
```

#### 步骤 3：输出到交换链

最后一个 Pass 的输出直接渲染到交换链图像：

```cpp
vkCmdBeginRenderPass(commandBuffer, &swapChainRenderPassInfo, ...);
// 最后一个后处理效果
vkCmdEndRenderPass(commandBuffer);
```

### 9.4 性能预算

| 效果 | 典型耗时 (1080p) | 说明 |
|------|-----------------|------|
| Bloom | 1~2 ms | 降分辨率后更快 |
| 色调映射 | 0.2~0.5 ms | 非常简单 |
| TAA | 1~2 ms | 需要历史帧 |
| FXAA | 0.5~1 ms | 比 TAA 快 |
| 色彩校正 | 0.2~0.5 ms | 3D LUT 采样 |
| **总计** | **3~6 ms** | 60 FPS 下占 18~36% |

### 9.5 调试技巧

| 问题 | 排查方法 |
|------|----------|
| 画面全黑 | 检查 HDR 纹理格式，是否支持浮点 |
| Bloom 太亮 | 降低阈值或强度 |
| TAA Ghosting | 降低混合因子，或检查运动向量 |
| 锯齿严重 | 检查 MSAA 是否开启，或 TAA Jitter 是否正确 |
| 色彩异常 | 检查 Gamma 校正，LUT 是否正确加载 |

---

## 附录 后处理效果速查表

| 效果 | 解决的问题 | 核心算法 | 性能 | 推荐度 |
|------|-----------|----------|------|--------|
| 色调映射 | HDR → LDR | ACES / Reinhard | 极低 | ★★★★★ |
| Bloom | 亮区光晕 | 亮度提取 + 模糊 | 中 | ★★★★☆ |
| 景深 | 对焦模糊 | CoC + 可变半径模糊 | 高 | ★★★☆☆ |
| 运动模糊 | 速度拖影 | 运动向量 + 方向模糊 | 中 | ★★★☆☆ |
| MSAA | 几何锯齿 | 多样本采样 | 中 | ★★★★☆（前向） |
| FXAA | 所有锯齿 | 边缘检测 + 模糊 | 低 | ★★★★☆（移动） |
| TAA | 所有锯齿 | 时间混合 | 中 | ★★★★★（PC/主机） |
| 色彩校正 | 色彩风格 | LUT / 曲线 | 低 | ★★★★☆ |
| SSAO | 接触阴影 | 深度/法线采样 | 中 | ★★★★☆ |
| SSR | 屏幕反射 | 光线步进 | 高 | ★★★☆☆ |

---

> **最后的话**
>
> 后处理是游戏画面的"最后一公里"。从 HDR 到 Bloom，从色调映射到抗锯齿，这些效果决定了最终画面的质感。
>
> 不要一开始就追求完美的后处理链。先搭建最小可用的 4 个效果（Bloom + 色调映射 + 抗锯齿 + 色彩校正），然后根据项目需求逐步添加。
>
> 记住：后处理不是万能的。如果基础光照不好，再多的后处理也救不回来。先把光照和材质做好，再用后处理锦上添花。
