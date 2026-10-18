# Shader 语言学习手册：从 CPU 到 GPU 的思维转换

> **作者的话**：这份手册不是 Shader 语法大全，而是一个渲染工程师带你入门 GPU 编程的笔记。我会用 C++ 的视角解释 Shader，用游戏实例展示应用，用踩坑经验分享教训。学完这份手册，你就能编写 Unity/Unreal 中的常用 Shader。

---

## 目录

- [第一章：Shader 在渲染管线中的位置](#第一章shader-在渲染管线中的位置)
  - [1.1 渲染管线概览](#11-渲染管线概览)
  - [1.2 可编程阶段](#12-可编程阶段)
  - [1.3 GPU 编程 vs CPU 编程](#13-gpu-编程-vs-cpu-编程)
- [第二章：Shader 语法基础](#第二章shader-语法基础)
  - [2.1 数据类型](#21-数据类型)
  - [2.2 变量限定符](#22-变量限定符)
  - [2.3 内置函数](#23-内置函数)
  - [2.4 GLSL vs HLSL 对比](#24-glsl-vs-hlsl-对比)
- [第三章：第一个 Shader - 纯色输出](#第三章第一个-shader---纯色输出)
  - [3.1 顶点着色器](#31-顶点着色器)
  - [3.2 片元着色器](#32-片元着色器)
  - [3.3 游戏应用场景](#33-游戏应用场景)
- [第四章：纹理采样](#第四章纹理采样)
  - [4.1 纹理坐标](#41-纹理坐标)
  - [4.2 采样器配置](#42-采样器配置)
  - [4.3 游戏应用场景](#43-游戏应用场景)
- [第五章：基础光照](#第五章基础光照)
  - [5.1 光照模型](#51-光照模型)
  - [5.2 漫反射](#52-漫反射)
  - [5.3 高光反射](#53-高光反射)
  - [5.4 游戏应用场景](#54-游戏应用场景)
- [第六章：法线贴图](#第六章法线贴图)
  - [6.1 切线空间](#61-切线空间)
  - [6.2 TBN 矩阵](#62-tbn-矩阵)
  - [6.3 游戏应用场景](#63-游戏应用场景)
- [第七章：后处理效果](#第七章后处理效果)
  - [7.1 全屏四边形](#71-全屏四边形)
  - [7.2 灰度效果](#72-灰度效果)
  - [7.3 游戏应用场景](#73-游戏应用场景)
- [第八章：常见坑与调试](#第八章常见坑与调试)
  - [8.1 精度问题](#81-精度问题)
  - [8.2 坐标系差异](#82-坐标系差异)
  - [8.3 法线空间](#83-法线空间)
  - [8.4 性能陷阱](#84-性能陷阱)
- [第九章：Shader 常用内置函数速查表](#第九章shader-常用内置函数速查表)
  - [9.1 数学函数](#91-数学函数)
  - [9.2 几何函数](#92-几何函数)
  - [9.3 纹理函数](#93-纹理函数)

---

## 第一章：Shader 在渲染管线中的位置

### 1.1 渲染管线概览

**类比**：渲染管线像工厂流水线。

```
CPU 准备数据                    GPU 处理
    ↓                              ↓
顶点数据 ──► 顶点着色器 ──► 光栅化 ──► 片元着色器 ──► 输出合并
(VBO)      (VS)         (Rasterizer)   (PS)        (OM)
```

**各阶段职责**：

| 阶段 | 输入 | 输出 | 职责 |
|------|------|------|------|
| **顶点着色器** | 顶点属性（位置、法线、UV） | 裁剪空间位置 + 插值属性 | 坐标变换、顶点动画 |
| **光栅化** | 三角形图元 | 片元（像素） | 三角形遍历、深度测试 |
| **片元着色器** | 插值后的属性 | 像素颜色 | 光照计算、纹理采样 |
| **输出合并** | 片元颜色 | 帧缓冲 | 深度测试、混合、写入 |

**关键概念**：
- **顶点着色器**：每个顶点执行一次（如 3 个顶点的三角形执行 3 次）
- **片元着色器**：每个像素执行一次（如 1080p 屏幕执行 200 万次）
- **插值**：顶点属性在三角形内自动插值到每个片元

### 1.2 可编程阶段

**固定管线 vs 可编程管线**：

**OpenGL 1.x（固定管线）**：
```cpp
// CPU 设置状态，GPU 按固定流程处理
glEnable(GL_LIGHTING);
glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
glMaterialfv(GL_FRONT, GL_DIFFUSE, materialColor);
// GPU 内部执行固定的光照计算
```

**现代图形 API（可编程管线）**：
```cpp
// CPU 编写 Shader，GPU 执行自定义逻辑
glUseProgram(shaderProgram);
glDrawArrays(GL_TRIANGLES, 0, 3);
// GPU 执行你写的 Shader 代码
```

**优势**：
- 自定义光照模型（PBR、卡通渲染）
- 自定义特效（溶解、描边、全息）
- 自定义后处理（Bloom、DOF、Motion Blur）

### 1.3 GPU 编程 vs CPU 编程

**核心差异**：

| 方面 | CPU | GPU |
|------|-----|-----|
| **架构** | 少量强核心 | 大量弱核心 |
| **执行模式** | 顺序执行 | SIMD 并行 |
| **内存** | 统一内存 | 分层内存（寄存器/共享/全局） |
| **控制流** | 自由分支 | 分支代价高（warp divergence） |
| **状态** | 全局状态 | 无全局状态（纯函数） |
| **数据流** | 任意访问 | 输入→处理→输出 |

**思维转换**：

**CPU 思维（顺序）**：
```cpp
// 处理 100 万个顶点
for (int i = 0; i < 1000000; i++) {
    vertices[i].position = transform(vertices[i].position);
}
```

**GPU 思维（并行）**：
```glsl
// 每个顶点独立执行
void main() {
    gl_Position = transform(position);  // 100 万个核心同时执行
}
```

**关键原则**：
1. **无全局状态**：每个 Shader 实例独立，不能互相通信
2. **数据流驱动**：输入→处理→输出，像纯函数
3. **避免分支**：`if-else` 会导致 warp divergence（部分核心等待）
4. **向量化**：用 `vec3/vec4` 而非 `float[3]`（SIMD 优化）

**工程师的建议**：
> 写 Shader 时，不要想"我在处理一个顶点"，而要想"100 万个核心同时在处理各自的顶点"。这就是 GPU 编程的本质。

---

## 第二章：Shader 语法基础

### 2.1 数据类型

**标量类型**：

| 类型 | GLSL | HLSL | 说明 |
|------|------|------|------|
| 浮点 | `float` | `float` | 32 位浮点 |
| 整数 | `int` | `int` | 32 位整数 |
| 布尔 | `bool` | `bool` | 布尔值 |

**向量类型**：

| 类型 | GLSL | HLSL | 说明 |
|------|------|------|------|
| 2D 向量 | `vec2` | `float2` | (x, y) |
| 3D 向量 | `vec3` | `float3` | (x, y, z) |
| 4D 向量 | `vec4` | `float4` | (x, y, z, w) |

**矩阵类型**：

| 类型 | GLSL | HLSL | 说明 |
|------|------|------|------|
| 2x2 矩阵 | `mat2` | `float2x2` | 2 行 2 列 |
| 3x3 矩阵 | `mat3` | `float3x3` | 3 行 3 列 |
| 4x4 矩阵 | `mat4` | `float4x4` | 4 行 4 列 |

**访问分量**：

```glsl
vec4 position = vec4(1.0, 2.0, 3.0, 1.0);

// 方式 1：xyzw
float x = position.x;
vec3 xyz = position.xyz;

// 方式 2：rgba（颜色）
float r = position.r;
vec3 rgb = position.rgb;

// 方式 3：stpq（纹理坐标）
float s = position.s;
vec2 st = position.st;

// 方式 4：索引
float first = position[0];
```

**构造函数**：

```glsl
// 标量扩展
vec3 v1 = vec3(1.0);  // (1.0, 1.0, 1.0)

// 分量组合
vec4 v2 = vec4(vec3(1.0), 1.0);  // (1.0, 1.0, 1.0, 1.0)
vec4 v3 = vec4(1.0, 2.0, 3.0, 1.0);

// 矩阵构造
mat3 m1 = mat3(1.0);  // 单位矩阵
mat3 m2 = mat3(vec3(1,0,0), vec3(0,1,0), vec3(0,0,1));
```

### 2.2 变量限定符

**GLSL 限定符**：

| 限定符 | 说明 | 生命周期 |
|--------|------|----------|
| `in` | 输入变量 | 从上一阶段接收 |
| `out` | 输出变量 | 传递到下一阶段 |
| `uniform` | 全局常量 | 整个绘制调用不变 |
| `const` | 编译时常量 | 编译时确定 |

**顶点着色器示例**：

```glsl
// 输入：顶点属性（每个顶点不同）
in vec3 position;
in vec3 normal;
in vec2 uv;

// 输出：传递给片元着色器（自动插值）
out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragUV;

// Uniform：全局常量（整个绘制调用不变）
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec4 worldPos = model * vec4(position, 1.0);
    fragPosition = worldPos.xyz;
    fragNormal = mat3(model) * normal;
    fragUV = uv;
    
    gl_Position = projection * view * worldPos;
}
```

**片元着色器示例**：

```glsl
// 输入：从顶点着色器接收（自动插值）
in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragUV;

// 输出：片元颜色
out vec4 outColor;

// Uniform
uniform vec3 lightPosition;
uniform vec3 lightColor;

void main() {
    vec3 normal = normalize(fragNormal);
    vec3 lightDir = normalize(lightPosition - fragPosition);
    float diff = max(dot(normal, lightDir), 0.0);
    
    vec3 diffuse = diff * lightColor;
    outColor = vec4(diffuse, 1.0);
}
```

**HLSL 限定符**：

```hlsl
// 输入结构
struct VSInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

// 输出结构
struct VSOutput {
    float4 position : SV_POSITION;
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float2 uv : TEXCOORD2;
};

// Uniform（常量缓冲）
cbuffer PerFrame : register(b0) {
    float4x4 model;
    float4x4 view;
    float4x4 projection;
};

// 顶点着色器
VSOutput VSMain(VSInput input) {
    VSOutput output;
    float4 worldPos = mul(float4(input.position, 1.0), model);
    output.worldPos = worldPos.xyz;
    output.normal = mul(input.normal, (float3x3)model);
    output.uv = input.uv;
    output.position = mul(worldPos, mul(view, projection));
    return output;
}
```

### 2.3 内置函数

**数学函数**：

```glsl
// 常用数学
float r = radians(180.0);  // 角度转弧度
float s = sin(angle);
float c = cos(angle);
float p = pow(base, exponent);
float e = exp(x);
float l = log(x);

// 插值
float m = mix(a, b, t);  // 线性插值 a*(1-t) + b*t
float cl = clamp(x, minVal, maxVal);
float st = step(edge, x);  // x < edge ? 0 : 1
float sm = smoothstep(edge0, edge1, x);  // 平滑阶梯

// 向量运算
float d = dot(a, b);  // 点积
vec3 c = cross(a, b);  // 叉积
float l = length(v);  // 长度
vec3 n = normalize(v);  // 归一化
float dist = distance(a, b);  // 距离
```

**纹理函数**：

```glsl
// 采样纹理
vec4 color = texture(tex, uv);

// 指定 Mipmap 级别
vec4 color = textureLod(tex, uv, lod);

// 梯度采样（用于导数计算）
vec4 color = textureGrad(tex, uv, ddx, ddy);
```

### 2.4 GLSL vs HLSL 对比

**语法差异**：

| 功能 | GLSL | HLSL |
|------|------|------|
| 向量类型 | `vec3` | `float3` |
| 矩阵类型 | `mat4` | `float4x4` |
| 纹理采样 | `texture(tex, uv)` | `tex.Sample(sampler, uv)` |
| 矩阵乘法 | `mat * vec` | `mul(vec, mat)` |
| 入口点 | `void main()` | `float4 PSMain() : SV_TARGET` |
| 输出 | `out vec4 color` | `float4 color : SV_TARGET` |

**矩阵乘法差异**：

```glsl
// GLSL：列主序，矩阵在左
vec4 clipPos = projection * view * model * vec4(position, 1.0);
```

```hlsl
// HLSL：行主序，矩阵在右（或转置后矩阵在左）
float4 clipPos = mul(float4(position, 1.0), model * view * projection);
// 或
float4 clipPos = mul(mul(mul(float4(position, 1.0), model), view), projection);
```

**工程师的建议**：
> GLSL 和 HLSL 本质相同，只是语法不同。Unity 用 HLSL（但早期用 GLSL），Unreal 用 HLSL。学一种后，另一种很快就能上手。

---

## 第三章：第一个 Shader - 纯色输出

### 3.1 顶点着色器

**目标**：将顶点从模型空间变换到裁剪空间。

```glsl
// vertex_shader.glsl
#version 330 core

// 输入：顶点属性
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;

// 输出：传递给片元着色器
out vec3 vColor;

// Uniform：MVP 矩阵
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main() {
    // 坐标变换：模型 → 世界 → 观察 → 裁剪
    vec4 worldPos = uModel * vec4(aPosition, 1.0);
    vec4 viewPos = uView * worldPos;
    vec4 clipPos = uProjection * viewPos;
    
    gl_Position = clipPos;
    vColor = aColor;
}
```

**逐行解释**：
- `#version 330 core`：指定 GLSL 版本
- `layout(location = 0)`：顶点属性位置（与 CPU 端对应）
- `in vec3 aPosition`：输入顶点位置
- `out vec3 vColor`：输出颜色（自动插值到片元）
- `uniform mat4 uModel`：模型矩阵（CPU 传入）
- `gl_Position`：内置输出（裁剪空间位置）

### 3.2 片元着色器

**目标**：输出纯色。

```glsl
// fragment_shader.glsl
#version 330 core

// 输入：从顶点着色器接收（自动插值）
in vec3 vColor;

// 输出：片元颜色
out vec4 FragColor;

void main() {
    // 直接输出顶点颜色
    FragColor = vec4(vColor, 1.0);
}
```

**逐行解释**：
- `in vec3 vColor`：接收顶点颜色（三角形内自动插值）
- `out vec4 FragColor`：输出片元颜色
- `vec4(vColor, 1.0)`：RGB + Alpha（不透明）

### 3.3 游戏应用场景

**纯色 Shader 的用途**：
1. **调试可视化**：显示法线方向、UV 坐标、深度
2. **低多边形风格**：复古游戏、像素艺术
3. **UI 元素**：简单色块、进度条
4. **碰撞体可视化**：编辑器中显示碰撞盒

**调试示例 - 显示法线**：

```glsl
// 顶点着色器
out vec3 vNormal;

void main() {
    vNormal = normalize(mat3(uModel) * aNormal);
    gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
}

// 片元着色器
in vec3 vNormal;

void main() {
    // 法线映射到 [0, 1] 范围显示
    vec3 color = vNormal * 0.5 + 0.5;
    FragColor = vec4(color, 1.0);
}
```

**效果**：
- X 轴（右）= 红色
- Y 轴（上）= 绿色
- Z 轴（前）= 蓝色

---

## 第四章：纹理采样

### 4.1 纹理坐标

**概念**：UV 坐标定义纹理上的位置。

```
(0,1) ┌────────┐ (1,1)
      │        │
      │  纹理   │
      │        │
(0,0) └────────┘ (1,0)
```

**顶点着色器传递 UV**：

```glsl
// 输入
layout(location = 2) in vec2 aUV;

// 输出
out vec2 vUV;

void main() {
    vUV = aUV;
    gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
}
```

### 4.2 采样器配置

**定义采样器**：

```glsl
// 纹理和采样器
uniform sampler2D uTexture;

// 片元着色器
in vec2 vUV;

void main() {
    // 采样纹理
    vec4 texColor = texture(uTexture, vUV);
    FragColor = texColor;
}
```

**采样器参数**：

```cpp
// CPU 端配置（OpenGL 示例）
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
```

**寻址模式**：
- `GL_REPEAT`：重复（平铺）
- `GL_CLAMP_TO_EDGE`：边缘拉伸
- `GL_MIRRORED_REPEAT`：镜像重复

**滤波模式**：
- `GL_NEAREST`：最近邻（像素风）
- `GL_LINEAR`：双线性（平滑）
- `GL_LINEAR_MIPMAP_LINEAR`：三线性（Mipmap 插值）

### 4.3 游戏应用场景

**纹理 Shader 的用途**：
1. **漫反射贴图**：物体表面颜色
2. **细节纹理**：墙面砖块、地面纹理
3. **UI 贴图**：图标、字体
4. **遮罩贴图**：控制混合区域

**平铺纹理示例**：

```glsl
uniform sampler2D uTexture;
uniform float uTiling;

void main() {
    // 平铺 5 次
    vec2 tiledUV = vUV * uTiling;
    vec4 color = texture(uTexture, tiledUV);
    FragColor = color;
}
```

**纹理混合示例**：

```glsl
uniform sampler2D uTexture1;
uniform sampler2D uTexture2;
uniform sampler2D uMask;

void main() {
    vec4 tex1 = texture(uTexture1, vUV);
    vec4 tex2 = texture(uTexture2, vUV);
    float mask = texture(uMask, vUV).r;
    
    // 根据遮罩混合
    vec4 color = mix(tex1, tex2, mask);
    FragColor = color;
}
```

**游戏应用**：
- 地形混合（草地 + 岩石 + 雪地）
- 角色换装（基础皮肤 + 服装）
- 溶解效果（遮罩控制消失区域）

---

## 第五章：基础光照

### 5.1 光照模型

**Phong 光照模型**：
```
最终颜色 = 环境光 + 漫反射 + 高光
```

**组成部分**：
- **环境光（Ambient）**：模拟间接光照（常数）
- **漫反射（Diffuse）**：Lambert 余弦定律
- **高光（Specular）**：Blinn-Phong 模型

### 5.2 漫反射

**Lambert 漫反射**：

```glsl
// 顶点着色器
out vec3 vNormal;
out vec3 vWorldPos;

void main() {
    vNormal = normalize(mat3(uModel) * aNormal);
    vWorldPos = (uModel * vec4(aPosition, 1.0)).xyz;
    gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
}

// 片元着色器
in vec3 vNormal;
in vec3 vWorldPos;

uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform vec3 uObjectColor;

void main() {
    // 环境光
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * uLightColor;
    
    // 漫反射
    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(uLightPos - vWorldPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor;
    
    // 最终颜色
    vec3 result = (ambient + diffuse) * uObjectColor;
    FragColor = vec4(result, 1.0);
}
```

**关键公式**：
```
diffuse = max(dot(N, L), 0.0)
```
- `N`：法线向量（归一化）
- `L`：光照方向（归一化）
- `dot(N, L)`：余弦值（-1 ~ 1）
- `max(..., 0.0)`：只取正值（背面不受光）

### 5.3 高光反射

**Blinn-Phong 高光**：

```glsl
// 片元着色器
in vec3 vNormal;
in vec3 vWorldPos;

uniform vec3 uLightPos;
uniform vec3 uViewPos;
uniform vec3 uLightColor;

void main() {
    // 环境光
    vec3 ambient = 0.1 * uLightColor;
    
    // 漫反射
    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(uLightPos - vWorldPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor;
    
    // 高光
    float specularStrength = 0.5;
    vec3 viewDir = normalize(uViewPos - vWorldPos);
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfDir), 0.0), 32);
    vec3 specular = specularStrength * spec * uLightColor;
    
    // 最终颜色
    vec3 result = (ambient + diffuse + specular) * uObjectColor;
    FragColor = vec4(result, 1.0);
}
```

**关键公式**：
```
specular = pow(max(dot(N, H), 0.0), shininess)
```
- `H`：半程向量 `normalize(L + V)`
- `shininess`：高光指数（越大越锐利）

### 5.4 游戏应用场景

**基础光照的用途**：
1. **角色光照**：NPC、怪物
2. **场景光照**：静态物体
3. **道具光照**：武器、装备
4. **特效光照**：技能、魔法

**卡通渲染示例**：

```glsl
void main() {
    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(uLightPos - vWorldPos);
    float diff = dot(norm, lightDir);
    
    // 量化为 3 个级别
    vec3 color;
    if (diff > 0.5) {
        color = vec3(1.0, 1.0, 1.0);  // 亮
    } else if (diff > 0.0) {
        color = vec3(0.6, 0.6, 0.6);  // 中
    } else {
        color = vec3(0.3, 0.3, 0.3);  // 暗
    }
    
    FragColor = vec4(color * uObjectColor, 1.0);
}
```

**效果**：类似《塞尔达传说：荒野之息》的卡通风格。

---

## 第六章：法线贴图

### 6.1 切线空间

**概念**：法线贴图存储的是相对于表面的法线偏移。

**为什么需要切线空间**：
- 世界空间法线贴图：模型旋转后失效
- 切线空间法线贴图：随表面变换，始终正确

**切线空间基向量**：
- **T（Tangent）**：切线方向（UV 的 U 方向）
- **B（Bitangent）**：副切线方向（UV 的 V 方向）
- **N（Normal）**：法线方向

### 6.2 TBN 矩阵

**构建 TBN 矩阵**：

```glsl
// 顶点着色器
in vec3 aPosition;
in vec3 aNormal;
in vec2 aUV;
in vec3 aTangent;

out vec3 vWorldPos;
out mat3 vTBN;

uniform mat4 uModel;

void main() {
    vWorldPos = (uModel * vec4(aPosition, 1.0)).xyz;
    
    // 计算 TBN 矩阵
    vec3 T = normalize(mat3(uModel) * aTangent);
    vec3 B = normalize(mat3(uModel) * cross(aNormal, aTangent));
    vec3 N = normalize(mat3(uModel) * aNormal);
    
    vTBN = mat3(T, B, N);
    
    gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
}
```

**采样法线贴图**：

```glsl
// 片元着色器
in vec3 vWorldPos;
in mat3 vTBN;

uniform sampler2D uNormalMap;

void main() {
    // 采样法线贴图（[0, 1] → [-1, 1]）
    vec3 tangentNormal = texture(uNormalMap, vUV).xyz * 2.0 - 1.0;
    
    // 转换到世界空间
    vec3 worldNormal = normalize(vTBN * tangentNormal);
    
    // 光照计算
    vec3 lightDir = normalize(uLightPos - vWorldPos);
    float diff = max(dot(worldNormal, lightDir), 0.0);
    
    FragColor = vec4(vec3(diff), 1.0);
}
```

**关键步骤**：
1. 采样法线贴图（RGB → XYZ）
2. 从 [0, 1] 映射到 [-1, 1]
3. 用 TBN 矩阵转换到世界空间
4. 进行光照计算

### 6.3 游戏应用场景

**法线贴图的用途**：
1. **表面细节**：砖墙、岩石、木纹
2. **角色细节**：皮肤毛孔、布料褶皱
3. **武器细节**：划痕、磨损
4. **地形细节**：地面凹凸

**强度控制示例**：

```glsl
uniform sampler2D uNormalMap;
uniform float uNormalStrength;

void main() {
    vec3 tangentNormal = texture(uNormalMap, vUV).xyz * 2.0 - 1.0;
    
    // 控制法线强度
    tangentNormal.xy *= uNormalStrength;
    tangentNormal = normalize(tangentNormal);
    
    vec3 worldNormal = normalize(vTBN * tangentNormal);
    
    // 光照计算...
}
```

**游戏应用**：
- LOD 切换：远处降低法线强度
- 材质混合：不同区域不同强度
- 动态效果：雨水湿润（增强法线）

---

## 第七章：后处理效果

### 7.1 全屏四边形

**概念**：后处理是全屏特效，渲染一个覆盖整个屏幕的四边形。

**顶点着色器**：

```glsl
// 全屏四边形顶点（无需顶点缓冲）
out vec2 vUV;

void main() {
    // 生成全屏四边形（-1 到 1）
    float x = float((gl_VertexID & 1) << 2);  // 0 或 4
    float y = float((gl_VertexID & 2) << 1);  // 0 或 2
    vUV = vec2(x * 0.5, y * 0.5);
    gl_Position = vec4(x - 1.0, y - 1.0, 0.0, 1.0);
}
```

**简化版（使用顶点缓冲）**：

```glsl
// 顶点数据：两个三角形组成的四边形
// (-1, -1), (3, -1), (-1, 3)

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aUV;

out vec2 vUV;

void main() {
    vUV = aUV;
    gl_Position = vec4(aPosition, 0.0, 1.0);
}
```

### 7.2 灰度效果

**片元着色器**：

```glsl
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uScreenTexture;

void main() {
    vec3 color = texture(uScreenTexture, vUV).rgb;
    
    // 灰度转换（加权平均）
    float gray = dot(color, vec3(0.299, 0.587, 0.114));
    
    FragColor = vec4(vec3(gray), 1.0);
}
```

**权重解释**：
- 红色：0.299（人眼对红色敏感度中等）
- 绿色：0.587（人眼对绿色最敏感）
- 蓝色：0.114（人眼对蓝色最不敏感）

### 7.3 游戏应用场景

**后处理的用途**：
1. **色调映射**：HDR → LDR
2. **Bloom**：高亮泛光
3. **DOF**：景深模糊
4. **Motion Blur**：运动模糊
5. **色彩校正**：调整亮度/对比度/饱和度

**色彩校正示例**：

```glsl
uniform float uBrightness;
uniform float uContrast;
uniform float uSaturation;

void main() {
    vec3 color = texture(uScreenTexture, vUV).rgb;
    
    // 亮度
    color += uBrightness;
    
    // 对比度
    color = (color - 0.5) * uContrast + 0.5;
    
    // 饱和度
    float gray = dot(color, vec3(0.299, 0.587, 0.114));
    color = mix(vec3(gray), color, uSaturation);
    
    FragColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
```

**Bloom 效果流程**：
1. 提取高亮区域（亮度 > 阈值）
2. 高斯模糊（多次降采样 + 模糊 + 上采样）
3. 叠加到原图

**游戏应用**：
- 受伤效果（屏幕变红）
- 中毒效果（屏幕变绿）
- 醉酒效果（模糊 + 扭曲）
- 梦境效果（色彩偏移）

---

## 第八章：常见坑与调试

### 8.1 精度问题

**问题 1：深度精度**

```glsl
// 错误：直接用 gl_FragCoord.z
float depth = gl_FragCoord.z;  // 非线性，近处精度高，远处精度低

// 正确：线性化深度
float depth = gl_FragCoord.z;
float near = 0.1;
float far = 1000.0;
float linearDepth = (2.0 * near) / (far + near - depth * (far - near));
```

**问题 2：浮点精度**

```glsl
// 问题：大坐标下精度丢失
vec3 worldPos = vec3(100000.0, 0.0, 0.0);
// 小数部分被截断

// 解决：使用相对坐标
vec3 relativePos = worldPos - cameraPos;
```

**问题 3：移动端精度**

```glsl
// 移动端需要指定精度
precision highp float;  // 高精度（默认）
precision mediump float;  // 中精度（性能更好）
precision lowp float;  // 低精度（颜色值足够）
```

### 8.2 坐标系差异

**问题 1：NDC 的 Z 范围**

```glsl
// OpenGL：Z ∈ [-1, 1]
// Vulkan/DirectX：Z ∈ [0, 1]

// 解决：检查 API，调整深度计算
#ifdef VULKAN
    float depth = clipPos.z / clipPos.w;  // [0, 1]
#else
    float depth = (clipPos.z / clipPos.w) * 0.5 + 0.5;  // [-1, 1] → [0, 1]
#endif
```

**问题 2：纹理坐标 V 轴方向**

```glsl
// OpenGL：V 轴向上（0 在下，1 在上）
// DirectX：V 轴向下（0 在上，1 在下）

// 解决：翻转 V 坐标
vec2 uv = vec2(vUV.x, 1.0 - vUV.y);
```

**问题 3：背面剔除方向**

```glsl
// OpenGL：逆时针为正面
// DirectX：顺时针为正面

// 解决：统一约定，或在引擎层处理
glFrontFace(GL_CCW);  // OpenGL
```

### 8.3 法线空间

**问题 1：法线贴图方向错误**

```glsl
// 症状：光照方向反转，表面凹陷变凸起

// 原因：法线贴图的绿色通道方向不一致
// OpenGL：绿色向上
// DirectX：绿色向下

// 解决：翻转绿色通道
vec3 tangentNormal = texture(uNormalMap, vUV).xyz * 2.0 - 1.0;
tangentNormal.y = -tangentNormal.y;  // 翻转 Y
```

**问题 2：非均匀缩放破坏法线**

```glsl
// 错误：直接用模型矩阵变换法线
vec3 wrongNormal = mat3(uModel) * aNormal;

// 正确：用逆转置矩阵
mat3 normalMatrix = transpose(inverse(mat3(uModel)));
vec3 correctNormal = normalMatrix * aNormal;
```

**原因**：非均匀缩放会破坏法线的垂直性。

### 8.4 性能陷阱

**陷阱 1：过度使用 if-else**

```glsl
// 差：分支导致 warp divergence
if (condition) {
    color = vec3(1.0, 0.0, 0.0);
} else {
    color = vec3(0.0, 1.0, 0.0);
}

// 好：用 mix 替代
color = mix(vec3(0.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0), float(condition));
```

**陷阱 2：循环展开**

```glsl
// 差：动态循环
for (int i = 0; i < numLights; i++) {
    // ...
}

// 好：固定循环（编译器可优化）
for (int i = 0; i < 4; i++) {  // 最多 4 个光源
    if (i < numLights) {
        // ...
    }
}
```

**陷阱 3：过度采样**

```glsl
// 差：多次采样同一纹理
vec4 color1 = texture(uTexture, uv1);
vec4 color2 = texture(uTexture, uv2);
vec4 color3 = texture(uTexture, uv3);

// 好：减少采样次数，或用纹理数组
```

**陷阱 4：高精度计算**

```glsl
// 差：所有计算用 highp
precision highp float;

// 好：按需选择精度
precision highp float;  // 位置、法线
precision mediump float;  // 颜色、UV
precision lowp float;  // 常量、开关
```

**调试技巧**：

```glsl
// 1. 可视化中间结果
FragColor = vec4(vNormal * 0.5 + 0.5, 1.0);  // 显示法线
FragColor = vec4(vec3(vUV, 0.0), 1.0);  // 显示 UV
FragColor = vec4(vec3(gl_FragCoord.z), 1.0);  // 显示深度

// 2. 检查 NaN/Inf
if (isnan(color.r) || isinf(color.r)) {
    FragColor = vec4(1.0, 0.0, 1.0, 1.0);  // 品红色标记错误
}

// 3. RenderDoc 调试
// - 捕获帧
// - 查看每个 Pass 的输入输出
// - 检查纹理内容
// - 分析性能瓶颈
```

---

## 第九章：Shader 常用内置函数速查表

### 9.1 数学函数

| 函数 | 说明 | 示例 |
|------|------|------|
| `radians(d)` | 角度转弧度 | `radians(180.0)` → 3.14159 |
| `degrees(r)` | 弧度转角度 | `degrees(3.14159)` → 180.0 |
| `sin(a)` | 正弦 | `sin(0.0)` → 0.0 |
| `cos(a)` | 余弦 | `cos(0.0)` → 1.0 |
| `tan(a)` | 正切 | `tan(0.0)` → 0.0 |
| `asin(x)` | 反正弦 | `asin(0.0)` → 0.0 |
| `acos(x)` | 反余弦 | `acos(1.0)` → 0.0 |
| `atan(y, x)` | 反正切 | `atan(1.0, 1.0)` → 0.785 |
| `pow(b, e)` | 幂运算 | `pow(2.0, 3.0)` → 8.0 |
| `exp(x)` | 自然指数 | `exp(1.0)` → 2.718 |
| `log(x)` | 自然对数 | `log(2.718)` → 1.0 |
| `sqrt(x)` | 平方根 | `sqrt(4.0)` → 2.0 |
| `inversqrt(x)` | 平方根倒数 | `inversqrt(4.0)` → 0.5 |
| `abs(x)` | 绝对值 | `abs(-1.0)` → 1.0 |
| `sign(x)` | 符号 | `sign(-5.0)` → -1.0 |
| `floor(x)` | 向下取整 | `floor(1.7)` → 1.0 |
| `ceil(x)` | 向上取整 | `ceil(1.2)` → 2.0 |
| `fract(x)` | 小数部分 | `fract(1.7)` → 0.7 |
| `mod(x, y)` | 取模 | `mod(5.0, 2.0)` → 1.0 |
| `min(x, y)` | 最小值 | `min(1.0, 2.0)` → 1.0 |
| `max(x, y)` | 最大值 | `max(1.0, 2.0)` → 2.0 |
| `clamp(x, a, b)` | 限制范围 | `clamp(1.5, 0.0, 1.0)` → 1.0 |
| `mix(a, b, t)` | 线性插值 | `mix(0.0, 1.0, 0.5)` → 0.5 |
| `step(e, x)` | 阶梯函数 | `step(0.5, 0.3)` → 0.0 |
| `smoothstep(a, b, x)` | 平滑阶梯 | `smoothstep(0.0, 1.0, 0.5)` → 0.5 |

### 9.2 几何函数

| 函数 | 说明 | 示例 |
|------|------|------|
| `length(v)` | 向量长度 | `length(vec3(3,4,0))` → 5.0 |
| `distance(a, b)` | 两点距离 | `distance(vec3(0), vec3(1))` → 1.732 |
| `dot(a, b)` | 点积 | `dot(vec3(1,0,0), vec3(1,0,0))` → 1.0 |
| `cross(a, b)` | 叉积 | `cross(vec3(1,0,0), vec3(0,1,0))` → vec3(0,0,1) |
| `normalize(v)` | 归一化 | `normalize(vec3(3,4,0))` → vec3(0.6,0.8,0) |
| `faceforward(N, I, Nref)` | 法线方向 | 确保法线朝向观察者 |
| `reflect(I, N)` | 反射向量 | 入射向量关于法线的反射 |
| `refract(I, N, eta)` | 折射向量 | 斯涅尔定律 |

### 9.3 纹理函数

| 函数 | 说明 | 示例 |
|------|------|------|
| `texture(s, uv)` | 采样纹理 | `texture(uTex, vec2(0.5))` |
| `textureLod(s, uv, lod)` | 指定 LOD | `textureLod(uTex, uv, 0.0)` |
| `textureProj(s, p)` | 投影纹理 | 阴影贴图 |
| `textureSize(s, lod)` | 纹理尺寸 | `textureSize(uTex, 0)` → vec2(1024) |
| `texelFetch(s, p, lod)` | 整数坐标采样 | `texelFetch(uTex, ivec2(0), 0)` |

---

> **工程师的结语**：Shader 编程是一门实践性很强的技能。不要只看书，要多写、多调试、多观察效果。从简单的纯色开始，逐步添加纹理、光照、特效。遇到问题时，用 RenderDoc 调试，用可视化中间结果排查。记住，每个炫酷的游戏效果，都是由这些基础 Shader 组合而成的。加油！
