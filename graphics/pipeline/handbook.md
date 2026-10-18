# 渲染管线原理学习手册

> **写在前面的话**
>
> 如果你已经能写出 Vulkan 的三角形，但脑子里对"从 Draw Call 到像素上屏"这条链路还是一团模糊——这份手册就是为你写的。
>
> 很多人学 Vulkan 时陷入了"配一堆状态对象 → 终于出三角形 → 但不理解为什么"的困境。根源在于缺少一张**管线的完整地图**。有了这张地图，你就知道每个 Vulkan 配置对应管线的哪个环节，为什么必须配、不配会怎样。
>
> 这份手册的思路是：**先给你全景，再逐段深入**。建议先通读一遍建立框架，再回头细看每个阶段的细节。

---

## 目录

- [第一章 全景：从 Draw Call 到像素上屏](#第一章-全景从-draw-call-到像素上屏)
- [第二章 应用阶段：CPU 侧的准备工作](#第二章-应用阶段cpu-侧的准备工作)
- [第三章 几何阶段：从顶点到屏幕坐标](#第三章-几何阶段从顶点到屏幕坐标)
- [第四章 光栅化阶段：从三角形到片元](#第四章-光栅化阶段从三角形到片元)
- [第五章 输出合并阶段：最终的守门员](#第五章-输出合并阶段最终的守门员)
- [第六章 关键概念深入](#第六章-关键概念深入)
- [第七章 前向渲染 vs 延迟渲染](#第七章-前向渲染-vs-延迟渲染)
- [第八章 Vulkan 视角：管线状态对象映射](#第八章-vulkan-视角管线状态对象映射)
- [附录 渲染管线术语速查表](#附录-渲染管线术语速查表)

---

## 第一章 全景：从 Draw Call 到像素上屏

### 1.1 一句话概括

GPU 渲染的本质就是：**把一堆三角形的顶点数据，变成屏幕上每个像素的颜色值。**

### 1.2 完整流程图

```
┌─────────────────────────────────────────────────────────────────────┐
│                        应用阶段 (CPU)                               │
│                                                                     │
│  场景图遍历 → 可见性判断 → 设置渲染状态 → 提交 Draw Call            │
│                                                                     │
│  输入：场景数据（模型、材质、灯光、摄像机）                          │
│  输出：Draw Call 命令 + 顶点/索引数据 + 纹理绑定                    │
└─────────────────────────────┬───────────────────────────────────────┘
                              │ 通过命令缓冲区提交给 GPU
                              ▼
┌─────────────────────────────────────────────────────────────────────┐
│                      几何阶段 (GPU)                                 │
│                                                                     │
│  ┌──────────┐   ┌──────────┐   ┌──────────┐   ┌──────────┐       │
│  │ 顶点着色 │──→│ 图元装配 │──→│  裁  剪  │──→│ 屏幕映射 │       │
│  │  (可编程) │   │ (固定)   │   │ (固定)   │   │ (固定)   │       │
│  └──────────┘   └──────────┘   └──────────┘   └──────────┘       │
│                                                                     │
│  输入：顶点属性（位置、法线、UV 等）                                │
│  输出：屏幕空间中的二维图元 + 深度值                                │
└─────────────────────────────┬───────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────────┐
│                     光栅化阶段 (GPU)                                │
│                                                                     │
│  ┌──────────┐   ┌──────────┐   ┌──────────────┐                   │
│  │ 三角形   │──→│ 三角形   │──→│  片元着色器  │                   │
│  │  设置    │   │  遍历    │   │   (可编程)    │                   │
│  │ (固定)   │   │ (固定)   │   │              │                   │
│  └──────────┘   └──────────┘   └──────────────┘                   │
│                                                                     │
│  输入：屏幕空间的三角形                                             │
│  输出：每个片元（fragment）的颜色 + 深度                            │
└─────────────────────────────┬───────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────────┐
│                    输出合并阶段 (GPU)                               │
│                                                                     │
│  ┌──────────┐   ┌──────────┐   ┌──────────┐   ┌──────────┐       │
│  │ 深度测试 │──→│ 模板测试 │──→│   混合   │──→│ 写入帧缓 │       │
│  │ (固定)   │   │ (固定)   │   │ (可配置) │   │ (固定)   │       │
│  └──────────┘   └──────────┘   └──────────┘   └──────────┘       │
│                                                                     │
│  输入：片元的颜色 + 深度                                            │
│  输出：最终写入帧缓冲区的像素颜色                                   │
└─────────────────────────────┬───────────────────────────────────────┘
                              │
                              ▼
                    ┌──────────────────┐
                    │  帧缓冲区 (显存)  │
                    │  → 交换链 → 屏幕 │
                    └──────────────────┘
```

### 1.3 核心思想：两个可编程阶段

整个管线中，**只有两个阶段是你可以编程的**：

| 阶段 | 可编程部分 | 在 Vulkan 中 | 在 Unity 中 |
|------|-----------|--------------|-------------|
| 几何阶段 | **顶点着色器** | `VkShaderModule` (VK_SHADER_STAGE_VERTEX_BIT) | Vertex Shader / HLSL |
| 光栅化阶段 | **片元着色器** | `VkShaderModule` (VK_SHADER_STAGE_FRAGMENT_BIT) | Fragment Shader / HLSL |

其余所有阶段都是**固定功能**——你不能改变它们的算法，只能**配置参数**（开关、比较方式、混合因子等）。

> **关键认知**：学渲染管线，本质上就是学"两个可编程阶段能做什么"和"固定功能阶段怎么配置"。

### 1.4 数据流概览

```
顶点数据 (3D 坐标)
    │
    ▼ [顶点着色器：变换坐标、传递属性]
屏幕空间三角形 (2D + 深度)
    │
    ▼ [光栅化：找到三角形覆盖的所有像素]
片元集合 (每个像素一个片元)
    │
    ▼ [片元着色器：计算每个片元的颜色]
带颜色的片元
    │
    ▼ [深度测试 + 混合：决定是否写入、怎么写入]
最终像素颜色
```

---

## 第二章 应用阶段：CPU 侧的准备工作

### 2.1 这个阶段做什么

应用阶段完全运行在 **CPU** 上（也可以说在"GPU 之外"）。它的职责是：

1. **准备数据**：决定哪些物体要画、用什么材质、从哪个角度画
2. **提交命令**：把绘制指令打包发给 GPU

### 2.2 具体步骤

#### 步骤一：场景遍历与可见性判断

```
场景中有 10000 个物体，但摄像机只能看到其中 500 个
    → 不需要画看不到的东西
    → 用视锥体剔除（Frustum Culling）和遮挡剔除（Occlusion Culling）筛选
```

**视锥体剔除**：检查物体的包围盒/包围球是否和摄像机的视锥体相交。不在视锥体内的直接跳过。

**遮挡剔除**：如果一个物体完全被另一个大物体挡住了，也不画。

#### 步骤二：设置渲染状态

在画每个物体之前，CPU 需要告诉 GPU：
- 用哪个 Shader？
- 绑定哪些纹理？
- 深度测试开不开？怎么比？
- 混合模式是什么？

#### 步骤三：提交 Draw Call

```cpp
// Vulkan 示例
vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
vkCmdBindDescriptorSets(commandBuffer, ...);
vkCmdBindVertexBuffers(commandBuffer, ...);
vkCmdBindIndexBuffer(commandBuffer, ...);
vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);  // ← 这就是一个 Draw Call
```

一个 Draw Call 通常对应一个网格（Mesh）的一次绘制。

### 2.3 性能瓶颈

| 瓶颈 | 表现 | 解决方案 |
|------|------|----------|
| Draw Call 过多 | CPU 忙于提交命令，GPU 在等 | 合批（Batching）、间接绘制（Indirect Draw） |
| 状态切换频繁 | 切换 Shader/纹理/管线状态的开销 | 按材质排序绘制顺序，减少切换 |
| 剔除不充分 | 画了大量不可见物体 | 优化剔除算法，使用 GPU 剔除 |

### 2.4 输入与输出

| | 内容 |
|---|------|
| **输入** | 场景图、摄像机参数、材质列表、光源数据 |
| **输出** | 命令缓冲区（包含一系列 Draw Call 和状态绑定） |

### 2.5 在 Vulkan 中的体现

Vulkan 把应用阶段的工作显式化了：
- **命令缓冲区**（`VkCommandBuffer`）：你手动录制所有绘制命令
- **描述符集**（`VkDescriptorSet`）：显式绑定 Shader 资源
- **管线状态对象**（`VkPipeline`）：预先打包所有渲染状态

这和 DirectX 12 类似，都是"显式 API"的设计哲学——CPU 做更多工作，换取更少的运行时开销。

---

## 第三章 几何阶段：从顶点到屏幕坐标

### 3.1 总览

几何阶段在 **GPU** 上运行，接收顶点数据，输出屏幕空间的二维图元。

```
输入                          输出
┌──────────────┐             ┌──────────────────┐
│ 顶点属性      │             │ 屏幕空间三角形    │
│ (模型空间)    │             │ (2D 坐标 + 深度)  │
│              │  几何阶段   │                  │
│ • 位置 (x,y,z)│ ─────────→ │ • 屏幕 x, y      │
│ • 法线        │             │ • 深度 z          │
│ • UV         │             │ • 插值后的属性     │
│ • 颜色 ...   │             │                  │
└──────────────┘             └──────────────────┘
```

### 3.2 顶点着色器（Vertex Shader）—— 可编程

#### 做什么

对**每个顶点**执行一次，主要工作：

1. **坐标变换**：把顶点从模型空间变换到裁剪空间（MVP 变换）
2. **属性传递**：把法线、UV 等属性传给后续的片元着色器

#### 输入与输出

```glsl
// 输入：每个顶点的属性
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

// Uniform：所有顶点共享的数据
layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 normalMatrix;  // 逆转置矩阵，用于变换法线
} ubo;

// 输出：传递给片元着色器的数据
layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragUV;
layout(location = 2) out vec3 fragWorldPos;

void main() {
    // 1. 坐标变换
    vec4 worldPos = ubo.model * vec4(inPosition, 1.0);
    gl_Position = ubo.proj * ubo.view * worldPos;
    
    // 2. 属性传递（法线要用逆转置矩阵变换）
    fragNormal = mat3(ubo.normalMatrix) * inNormal;
    fragUV = inUV;
    fragWorldPos = worldPos.xyz;
}
```

#### 关键认知

- 顶点着色器**不能**创建或销毁顶点，它只能移动顶点的位置
- 每个顶点独立处理，顶点之间没有信息交换
- 输出 `gl_Position` 是**裁剪空间**坐标（齐次坐标，$w \neq 1$）

### 3.3 图元装配（Primitive Assembly）—— 固定功能

#### 做什么

把顶点着色器输出的顶点，按照你指定的图元类型（三角形、线、点）组装成图元。

```
顶点流：v0, v1, v2, v3, v4, v5

三角形列表 (VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)：
  → 组装成 (v0,v1,v2), (v3,v4,v5)

三角形条带 (VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP)：
  → 组装成 (v0,v1,v2), (v1,v2,v3), (v2,v3,v4), (v3,v4,v5)
```

#### 性能提示

三角形条带（Triangle Strip）比三角形列表（Triangle List）更省顶点数据，适合连续的表面。但现代 GPU 有顶点缓存，差异不大。

### 3.4 裁剪（Clipping）—— 固定功能

#### 做什么

裁剪掉**超出视锥体**的部分。

```
           视锥体
          /      \
         /   △   \      ← 完全在内部，保留
        /         \
       /           \
      /─────── △ ───\──  ← 一部分在外面，裁剪后变成更小的三角形
     /           /    
    /           /
        △ ← 完全在外面，丢弃
```

#### 裁剪的过程

1. 对每条边，检查两个顶点是否在视锥体内
2. 如果一个在内一个在外，计算边和视锥体边界的交点
3. 用交点替换外面的顶点，生成新的三角形

#### Sutherland-Hodgman 算法

这是最常用的裁剪算法。对每个裁剪平面（左、右、上、下、近、远）依次处理：

```
原始三角形和近裁剪面：

    近裁剪面
      |
  v0  |  v1
   ·  |   ·
    \ |  /
     \| /
      · v2  (在裁剪面后面)

裁剪后：
    近裁剪面
      |
  v0  |  v1'
   ·  |   ·
    \ |  /
     \| /
      · v2'
      
  原来的一个三角形变成了一个新的三角形（或更多）
```

### 3.5 透视除法与屏幕映射 —— 固定功能

#### 透视除法

把裁剪空间坐标变成 **NDC（归一化设备坐标）**：

$$x_{ndc} = \frac{x_{clip}}{w_{clip}}, \quad y_{ndc} = \frac{y_{clip}}{w_{clip}}, \quad z_{ndc} = \frac{z_{clip}}{w_{clip}}$$

NDC 的范围：
- **OpenGL**：$x, y, z \in [-1, 1]$
- **Vulkan / DirectX**：$x, y \in [-1, 1]$，$z \in [0, 1]$

#### 屏幕映射（视口变换）

把 NDC 映射到实际的屏幕像素坐标：

$$x_{screen} = \frac{x_{ndc} + 1}{2} \times \text{width} + x_{offset}$$

$$y_{screen} = \frac{y_{ndc} + 1}{2} \times \text{height} + y_{offset}$$

$$z_{screen} = \frac{z_{ndc} \times (\text{far} - \text{near}) + (\text{near} + \text{far})}{2}$$

在 Vulkan 中，视口参数通过 `vkCmdSetViewport` 或管线状态设置：

```cpp
VkViewport viewport = {};
viewport.x = 0.0f;
viewport.y = 0.0f;
viewport.width = (float)swapChainExtent.width;
viewport.height = (float)swapChainExtent.height;
viewport.minDepth = 0.0f;
viewport.maxDepth = 1.0f;
```

### 3.6 背面剔除（Backface Culling）—— 可配置

#### 做什么

如果一个三角形**背对摄像机**，就不画它。

#### 怎么判断背对

在屏幕空间中，看三角形的三个顶点是**顺时针**还是**逆时针**排列：

```
顺时针 (CW)：              逆时针 (CCW)：
    v0                         v0
   /  \                       /  \
  v1──→v2                   v2←──v1
  
  背对摄像机                正对摄像机
  （剔除）                  （保留）
```

> **注意**：这里的"顺/逆时针"是在**屏幕空间**中看的，不是在世界空间中。

#### Vulkan 配置

```cpp
VkPipelineRasterizationStateCreateInfo rasterizer = {};
rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;       // 剔除背面
rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;  // 逆时针为正面
```

#### 什么时候关闭背面剔除？

- 画半透明物体（两面都要看到）
- 粒子效果
- 某些特殊效果（如天空盒的内表面）

### 3.7 几何阶段的输入与输出

| | 内容 |
|---|------|
| **输入** | 顶点属性（模型空间位置、法线、UV 等） |
| **输出** | 屏幕空间三角形（2D 坐标 + 深度 + 插值属性） |

### 3.8 性能瓶颈

| 瓶颈 | 表现 | 解决方案 |
|------|------|----------|
| 顶点着色器太复杂 | GPU 顶点处理单元满载 | 简化 Shader，用 LOD（远处用低模） |
| 顶点数据过多 | 内存带宽瓶颈 | 压缩顶点数据，用实例化绘制 |
| 过度绘制（Overdraw） | 同一个像素被多个三角形覆盖 | 按深度排序（前向渲染），或延迟渲染 |

---

## 第四章 光栅化阶段：从三角形到片元

### 4.1 总览

光栅化阶段的任务是：**找到每个三角形覆盖了屏幕上的哪些像素，并为每个像素生成一个"片元"（fragment）。**

```
输入                          输出
┌──────────────┐             ┌──────────────────┐
│ 屏幕空间三角形│             │ 片元集合          │
│              │  光栅化阶段  │                  │
│ • 三个顶点的  │ ─────────→ │ • 每个片元有：    │
│   屏幕坐标    │             │   - 屏幕位置     │
│ • 顶点属性    │             │   - 深度         │
│              │             │   - 插值后的属性  │
└──────────────┘             └──────────────────┘
```

### 4.2 三角形设置（Triangle Setup）—— 固定功能

#### 做什么

计算三角形的**边方程**（edge equation），用于判断一个像素是否在三角形内部。

```
三角形三个顶点：v0, v1, v2

边方程：
  E0(x, y) = 0 表示像素在 v1→v2 这条边上
  E1(x, y) = 0 表示像素在 v2→v0 这条边上
  E2(x, y) = 0 表示像素在 v0→v1 这条边上

  如果 E0, E1, E2 都 > 0（或都 < 0，取决于绕序），
  则像素在三角形内部。
```

这一步还会计算**属性插值的系数**（重心坐标），后面会用到。

### 4.3 三角形遍历（Triangle Traversal）—— 固定功能

#### 做什么

找到三角形**包围盒（bounding box）**内的所有像素，用边方程判断每个像素是否在三角形内。

```
         v0
         /  \
        / △  \
       /      \
      /        \
     v1────────v2
     
  包围盒：包含三角形的最小矩形
  
  遍历包围盒内的每个像素 (x, y)：
    if (E0(x,y) > 0 && E1(x,y) > 0 && E2(x,y) > 0) {
        // 像素在三角形内，生成一个片元
    }
```

#### 优化：Tile-Based 渲染

移动 GPU（如 Mali、Adreno）常用 **Tile-Based** 架构：

```
屏幕被分成小块（Tile），比如 16×16 像素

  ┌───┬───┬───┬───┐
  │ T0│ T1│ T2│ T3│
  ├───┼───┼───┼───┤
  │ T4│ T5│ T6│ T7│
  ├───┼───┼───┼───┤
  │ T8│ T9│T10│T11│
  └───┴───┴───┴───┘

每个 Tile 独立处理，利用缓存局部性，减少显存带宽。
```

### 4.4 属性插值 —— 固定功能

#### 做什么

三角形的三个顶点各有属性（法线、UV、颜色等），片元需要**插值**出属于自己的属性。

#### 重心坐标插值

用**重心坐标（barycentric coordinates）** $(\lambda_0, \lambda_1, \lambda_2)$ 来插值：

$$\text{fragAttr} = \lambda_0 \cdot \text{attr}_0 + \lambda_1 \cdot \text{attr}_1 + \lambda_2 \cdot \text{attr}_2$$

其中 $\lambda_0 + \lambda_1 + \lambda_2 = 1$。

```
           v0 (attr = 红色)
           / \
          /   \
    λ₀=0.2   λ₁=0.3
        /   P   \
       / (λ₂=0.5)\
      v1─────────v2
  (蓝色)       (绿色)
  
  P 点的颜色 = 0.2×红 + 0.3×蓝 + 0.5×绿
```

#### 透视校正插值

**重要**：属性插值必须考虑**透视效果**。

```
没有透视校正（线性插值）：       有透视校正：
                                
  ┌──────────────┐               ┌──────────────┐
  │ ▓▓▓▓▓▓▓▓▓▓▓▓│               │ ▓▓▓▓▓▓▓▓▓▓▓▓│
  │ 纹理看起来    │               │ 纹理看起来    │
  │ 被拉伸了      │               │ 正确，近大远小│
  │              │               │              │
  └──────────────┘               └──────────────┘
```

透视校正插值的公式：

$$\text{fragAttr} = \frac{\frac{\lambda_0}{w_0} \cdot \text{attr}_0 + \frac{\lambda_1}{w_1} \cdot \text{attr}_1 + \frac{\lambda_2}{w_2} \cdot \text{attr}_2}{\frac{\lambda_0}{w_0} + \frac{\lambda_1}{w_1} + \frac{\lambda_2}{w_2}}$$

其中 $w_0, w_1, w_2$ 是三个顶点的 $w$ 分量（裁剪空间）。

> **关键认知**：GPU 自动帮你做透视校正插值，你不需要手动算。但理解原理有助于调试奇怪的纹理拉伸问题。

### 4.5 片元着色器（Fragment Shader）—— 可编程

#### 做什么

对**每个片元**执行一次，计算它的最终颜色。

这是渲染中最灵活、最复杂的阶段。你可以在这里实现：
- 纹理采样
- 光照计算
- 法线贴图
- 各种材质效果

#### 输入与输出

```glsl
// 输入：从顶点着色器插值而来的属性
layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragWorldPos;

// 输出：片元的颜色
layout(location = 0) out vec4 outColor;

// 资源绑定
layout(set = 1, binding = 0) uniform sampler2D albedoMap;
layout(set = 1, binding = 1) uniform LightInfo {
    vec3 lightPos;
    vec3 lightColor;
} light;

void main() {
    // 1. 采样纹理
    vec3 albedo = texture(albedoMap, fragUV).rgb;
    
    // 2. 计算光照（简化版 Lambert）
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(light.lightPos - fragWorldPos);
    float diffuse = max(dot(N, L), 0.0);
    
    // 3. 最终颜色
    vec3 color = albedo * light.lightColor * diffuse;
    outColor = vec4(color, 1.0);
}
```

#### 性能瓶颈

| 瓶颈 | 表现 | 解决方案 |
|------|------|----------|
| 纹理采样过多 | 纹理带宽瓶颈 | 用纹理数组、Mipmap、压缩纹理 |
| Shader 太复杂 | GPU 片元处理单元满载 | 简化 Shader，用 LOD，减少指令数 |
| 动态分支 | GPU 的 SIMD 架构不擅长分支 | 用 `step`/`mix` 代替 `if`，减少分支 |

### 4.6 光栅化阶段的输入与输出

| | 内容 |
|---|------|
| **输入** | 屏幕空间三角形（顶点坐标 + 属性） |
| **输出** | 片元（屏幕位置 + 深度 + 颜色） |

---

## 第五章 输出合并阶段：最终的守门员

### 5.1 总览

输出合并阶段（Output Merger, OM）是管线的**最后一站**。它决定：
- 这个片元能不能写入帧缓冲区？（深度测试、模板测试）
- 如果能写入，怎么和已有的像素颜色混合？（混合）

```
输入                          输出
┌──────────────┐             ┌──────────────────┐
│ 片元          │             │ 最终像素颜色      │
│              │  输出合并    │                  │
│ • 颜色        │ ─────────→ │ 写入帧缓冲区      │
│ • 深度        │             │                  │
│              │             │                  │
└──────────────┘             └──────────────────┘
```

### 5.2 深度测试（Depth Test）—— 固定功能

#### 做什么

检查片元的深度是否**通过测试**，决定它是否可见。

#### 测试规则

```
if (片元深度 比较运算符 缓冲区中已有的深度) {
    通过测试 → 继续到模板测试
} else {
    丢弃片元 → 不写入
}
```

常用的比较运算符：
- `VK_COMPARE_OP_LESS`：片元深度 < 缓冲区深度（默认，最常用的）
- `VK_COMPARE_OP_LEQUAL`：片元深度 ≤ 缓冲区深度
- `VK_COMPARE_OP_ALWAYS`：总是通过（关闭深度测试）

#### 深度缓冲区

深度缓冲区（Z-Buffer）和颜色缓冲区一样大，存储每个像素的深度值（通常是 24 位或 32 位浮点数）。

```
屏幕像素 (x, y)：
  颜色缓冲区：RGB = (0.8, 0.2, 0.1)
  深度缓冲区：Z = 0.65  (范围 [0, 1])
```

#### 深度写入

通过深度测试后，还要决定是否**更新**深度缓冲区：

```cpp
VkPipelineDepthStencilStateCreateInfo depthStencil = {};
depthStencil.depthTestEnable = VK_TRUE;       // 开启深度测试
depthStencil.depthWriteEnable = VK_TRUE;      // 通过测试后写入深度
depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
```

**什么时候关闭深度写入？**
- 画半透明物体（需要保留后面的物体的深度信息）
- 画天空盒（深度设为最远，不影响其他物体）

### 5.3 模板测试（Stencil Test）—— 固定功能

#### 做什么

用一个**模板缓冲区**（Stencil Buffer）来做更精细的可见性控制。

模板缓冲区通常和深度缓冲区绑在一起（如 24 位深度 + 8 位模板）。

#### 测试过程

```
对每个片元：
  1. 读取模板缓冲区中对应位置的值 ref
  2. 用配置的规则测试：(片元的参考值 比较运算符 ref)
  3. 根据测试结果，更新模板缓冲区
```

#### 典型用途

**轮廓线效果（Outline）**：
1. 第一遍：画物体，模板值设为 1
2. 第二遍：放大物体再画一次，模板测试只画模板值 ≠ 1 的部分（就是边缘）

**镜子反射**：
1. 第一遍：在模板缓冲区中标记镜子区域
2. 第二遍：只画镜子反射的物体（模板测试限制在镜子区域内）

#### Vulkan 配置

```cpp
VkStencilOpState stencilOp = {};
stencilOp.failOp = VK_STENCIL_OP_KEEP;
stencilOp.passOp = VK_STENCIL_OP_REPLACE;
stencilOp.compareOp = VK_COMPARE_OP_ALWAYS;
stencilOp.reference = 1;

depthStencil.stencilTestEnable = VK_TRUE;
depthStencil.front = stencilOp;
depthStencil.back = stencilOp;
```

### 5.4 混合（Blending）—— 可配置

#### 做什么

当片元通过深度测试和模板测试后，决定它的颜色怎么和帧缓冲区中已有的颜色**混合**。

#### 混合公式

$$\text{最终颜色} = \text{源因子} \times \text{片元颜色} \quad \text{混合操作} \quad \text{目标因子} \times \text{已有颜色}$$

常用的混合模式：

| 效果 | 源因子 | 目标因子 | 混合操作 | 公式 |
|------|--------|----------|----------|------|
| 不透明 | ONE | ZERO | ADD | $C_{src}$ |
| 透明（Alpha 混合） | SRC_ALPHA | ONE_MINUS_SRC_ALPHA | ADD | $C_{src} \cdot \alpha + C_{dst} \cdot (1-\alpha)$ |
| 叠加 | ONE | ONE | ADD | $C_{src} + C_{dst}$ |
| 正片叠底 | DST_COLOR | ZERO | ADD | $C_{src} \times C_{dst}$ |

#### Vulkan 配置

```cpp
VkPipelineColorBlendAttachmentState blendAttachment = {};
blendAttachment.blendEnable = VK_TRUE;
blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
```

### 5.5 写入帧缓冲区

通过所有测试和混合后，最终的颜色写入帧缓冲区（Frame Buffer），等待显示。

### 5.6 输出合并阶段的输入与输出

| | 内容 |
|---|------|
| **输入** | 片元（颜色 + 深度） |
| **输出** | 最终像素颜色（写入帧缓冲区） |

### 5.7 性能瓶颈

| 瓶颈 | 表现 | 解决方案 |
|------|------|----------|
| 深度测试失败率高 | 大量片元被丢弃，浪费计算 | 按深度排序（从近到远），Early-Z |
| 过度绘制 | 同一个像素被多次绘制 | 减少透明物体叠加，延迟渲染 |
| 混合计算复杂 | 混合单元成为瓶颈 | 减少半透明物体，用 OIT（Order Independent Transparency） |

---

## 第六章 关键概念深入

### 6.1 NDC（归一化设备坐标）

#### 是什么

NDC 是透视除法后的坐标空间，范围：
- **OpenGL**：$x, y, z \in [-1, 1]$
- **Vulkan / DirectX**：$x, y \in [-1, 1]$，$z \in [0, 1]$

#### 为什么需要 NDC

NDC 是一个**统一的坐标空间**，和屏幕分辨率无关。这样视口变换就可以把 NDC 映射到任意大小的屏幕。

```
裁剪空间 (x, y, z, w)
    │
    ▼ 透视除法 (÷w)
NDC (-1~1, -1~1, 0~1)
    │
    ▼ 视口变换 (×宽高 + 偏移)
屏幕空间 (像素坐标)
```

### 6.2 视口变换（Viewport Transform）

#### 公式

$$x_{screen} = \frac{x_{ndc} + 1}{2} \times \text{width} + x_{offset}$$

$$y_{screen} = \frac{y_{ndc} + 1}{2} \times \text{height} + y_{offset}$$

$$z_{screen} = z_{ndc} \times (\text{maxDepth} - \text{minDepth}) + \text{minDepth}$$

#### Vulkan 中的视口

```cpp
VkViewport viewport = {};
viewport.x = 0.0f;
viewport.y = 0.0f;
viewport.width = 1920.0f;      // 屏幕宽度
viewport.height = 1080.0f;     // 屏幕高度
viewport.minDepth = 0.0f;      // 深度范围最小值
viewport.maxDepth = 1.0f;      // 深度范围最大值
```

### 6.3 背面剔除（Backface Culling）

#### 原理

在屏幕空间中，根据三角形的绕序（顺时针或逆时针）判断它是否背对摄像机。

```
正面（CCW）：v0 → v1 → v2 逆时针
背面（CW）：v0 → v1 → v2 顺时针

Vulkan 默认：逆时针为正面
```

#### 为什么能节省性能

一个封闭的模型（如球体、角色），大约有一半的三角形是背对摄像机的。剔除它们可以直接减少 50% 的光栅化工作量。

### 6.4 早期 深度测试（Early-Z）

#### 是什么

在**片元着色器之前**就做深度测试，提前丢弃被遮挡的片元。

```
正常流程：
  光栅化 → 片元着色器 → 深度测试 → 写入
  
Early-Z 流程：
  光栅化 → 深度测试（提前） → 片元着色器 → 写入
```

#### 为什么有效

如果一个像素已经被前面的物体遮挡，Early-Z 可以直接丢弃它，**不需要执行片元着色器**。这在复杂场景中能节省大量计算。

#### Early-Z 失效场景

Early-Z 不是万能的，以下情况会失效：

| 场景 | 原因 |
|------|------|
| 片元着色器中写入深度 | GPU 无法提前知道深度值 |
| 透明物体 | 透明物体需要混合，不能简单丢弃 |
| Alpha Test（Alpha 裁剪） | 片元着色器中 `discard` 会改变深度 |
| 深度写入关闭 | 无法更新深度缓冲区 |

```glsl
// 这个 Shader 会导致 Early-Z 失效
void main() {
    vec4 color = texture(albedoMap, fragUV);
    if (color.a < 0.5) {
        discard;  // ← 在片元着色器中丢弃片元
    }
    outColor = color;
}
```

### 6.5 过度绘制（Overdraw）

#### 是什么

同一个像素被多个三角形覆盖，导致**多次绘制**。

```
场景：一堵墙前面有一片树叶

  摄像机 → 🍃 → 🧱
  
  像素 A 被画了两次：
    第一次：画墙
    第二次：画树叶（覆盖在墙上）
```

#### 危害

- 浪费 GPU 计算资源（片元着色器执行多次）
- 浪费内存带宽（多次读写帧缓冲区）

#### 解决方案

| 方案 | 原理 | 适用场景 |
|------|------|----------|
| 按深度排序（前向渲染） | 从近到远绘制，减少深度测试失败 | 不透明物体 |
| 延迟渲染 | 先写 G-Buffer，最后统一光照 | 大量光源 |
| 遮挡剔除 | 不画被完全遮挡的物体 | 室内场景 |
| Alpha to Coverage | 用 MSAA 减少透明物体的过度绘制 | 植被、铁丝网 |

### 6.6 深度精度问题（Z-Fighting）

#### 是什么

两个共面的三角形因为深度精度不足，出现闪烁。

```
  地面  ───────────────
   decals ───────────────  ← 两个面几乎重合
  
  深度值非常接近，浮点精度不够，导致某些像素深度测试忽前忽后
```

#### 解决方案

- **Polygon Offset**：给 decals 一个微小的深度偏移
- **增加近裁剪面距离**：减少深度范围
- **用 32 位深度缓冲区**：提高精度

```cpp
VkPipelineRasterizationStateCreateInfo rasterizer = {};
rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
rasterizer.depthBiasEnable = VK_TRUE;        // 开启深度偏移
rasterizer.depthBiasConstantFactor = 1.25f;
rasterizer.depthBiasSlopeFactor = 1.75f;
```

---

## 第七章 前向渲染 vs 延迟渲染

### 7.1 前向渲染（Forward Rendering）

#### 管线流程

```
对每个物体：
  1. 顶点着色器：变换坐标
  2. 片元着色器：对所有光源计算光照，输出最终颜色
  3. 深度测试 + 混合 → 写入帧缓冲区
```

#### 特点

| 优点 | 缺点 |
|------|------|
| 简单直观 | 每个片元要计算所有光源 |
| 天然支持透明 | 光源多时性能下降快 |
| 支持 MSAA（硬件抗锯齿） | 过度绘制严重时效率低 |

#### 适用场景

- 光源数量少（< 10 个）
- 移动平台（性能受限）
- 需要高质量抗锯齿

### 7.2 延迟渲染（Deferred Rendering）

#### 管线流程

```
第一遍：几何阶段（Geometry Pass）
  对每个物体：
    1. 顶点着色器：变换坐标
    2. 片元着色器：输出到 G-Buffer（多个纹理）
       - 颜色（Albedo）
       - 法线（Normal）
       - 深度（Depth）
       - 其他（如发光、金属度等）

第二遍：光照阶段（Lighting Pass）
  对每个像素：
    1. 从 G-Buffer 读取几何信息
    2. 对每个光源计算光照
    3. 输出最终颜色
```

#### G-Buffer 结构

```
G-Buffer（帧缓冲区 A）：
  Attachment 0: vec4 albedo      (RGB: 颜色, A: 发光强度)
  Attachment 1: vec4 normal      (RGB: 法线, A: 金属度)
  Attachment 2: vec4 specular    (RGB: 高光颜色, A: 粗糙度)
  Attachment 3: vec4 position    (RGB: 世界坐标, A: 深度)
```

#### 特点

| 优点 | 缺点 |
|------|------|
| 光照计算和几何复杂度解耦 | 不支持透明物体（需要额外 Pass） |
| 光源多时性能好 | G-Buffer 占用显存大 |
| 减少过度绘制 | 不支持硬件 MSAA |

#### 适用场景

- 光源数量多（> 50 个）
- PC / 主机平台（显存充足）
- 大量动态光源

### 7.3 对比总结

| 特性 | 前向渲染 | 延迟渲染 |
|------|----------|----------|
| 光照计算位置 | 片元着色器 | 独立的 Lighting Pass |
| 光源数量扩展性 | 差（O(物体×光源)） | 好（O(物体+光源)） |
| 透明物体 | 天然支持 | 需要额外 Pass |
| 抗锯齿 | 支持 MSAA | 需要后处理（FXAA/TAA） |
| 显存占用 | 低 | 高（G-Buffer） |
| 带宽需求 | 低 | 高（多次读写 G-Buffer） |

### 7.4 混合方案：Forward+

**Forward+**（也叫 Light Indexed Deferred）结合了两者的优点：

```
1. 把屏幕分成小块（Tile）
2. 对每个 Tile，找出影响它的光源列表
3. 前向渲染时，只计算影响当前片元的光源
```

这样既保留了前向渲染的简单性和对透明的支持，又获得了延迟渲染的光源扩展性。

---

## 第八章 Vulkan 视角：管线状态对象映射

### 8.1 Vulkan 管线状态对象（PSO）

在 Vulkan 中，管线的**固定功能阶段**通过 `VkGraphicsPipelineCreateInfo` 配置。这个结构体包含了所有阶段的状态。

```cpp
VkGraphicsPipelineCreateInfo pipelineInfo = {};
pipelineInfo.stageCount = 2;                      // 顶点 + 片元着色器
pipelineInfo.pStages = shaderStages;              // Shader 模块

pipelineInfo.pVertexInputState = &vertexInputInfo;     // 顶点输入布局
pipelineInfo.pInputAssemblyState = &inputAssembly;     // 图元装配
pipelineInfo.pViewportState = &viewportState;          // 视口和裁剪
pipelineInfo.pRasterizationState = &rasterizer;        // 光栅化
pipelineInfo.pMultisampleState = &multisampling;       // 多重采样
pipelineInfo.pDepthStencilState = &depthStencil;       // 深度和模板测试
pipelineInfo.pColorBlendState = &colorBlending;        // 颜色混合
pipelineInfo.pDynamicState = dynamicState;             // 动态状态

pipelineInfo.layout = pipelineLayout;              // 描述符集布局
pipelineInfo.renderPass = renderPass;              // 渲染通道
pipelineInfo.subPass = 0;                          // 子通道索引
```

### 8.2 各阶段对应的配置

| 管线阶段 | 对应的 Vulkan 配置 | 说明 |
|----------|-------------------|------|
| **顶点输入** | `VkPipelineVertexInputStateCreateInfo` | 顶点属性格式、绑定 |
| **图元装配** | `VkPipelineInputAssemblyStateCreateInfo` | 图元类型（三角形、线、点） |
| **顶点着色器** | `VkPipelineShaderStageCreateInfo` (VERTEX) | Shader 代码 |
| **视口和裁剪** | `VkPipelineViewportStateCreateInfo` | 视口矩形、裁剪矩形 |
| **光栅化** | `VkPipelineRasterizationStateCreateInfo` | 多边形模式、剔除、深度偏移 |
| **多重采样** | `VkPipelineMultisampleStateCreateInfo` | MSAA 采样数 |
| **深度/模板测试** | `VkPipelineDepthStencilStateCreateInfo` | 深度/模板测试配置 |
| **颜色混合** | `VkPipelineColorBlendStateCreateInfo` | 混合因子、操作 |
| **片元着色器** | `VkPipelineShaderStageCreateInfo` (FRAGMENT) | Shader 代码 |

### 8.3 顶点输入状态

```cpp
// 顶点属性描述
VkVertexInputAttributeDescription attributeDescriptions[3] = {};

// location 0: 位置
attributeDescriptions[0].binding = 0;
attributeDescriptions[0].location = 0;
attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;  // vec3
attributeDescriptions[0].offset = 0;

// location 1: 法线
attributeDescriptions[1].binding = 0;
attributeDescriptions[1].location = 1;
attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
attributeDescriptions[1].offset = sizeof(float) * 3;

// location 2: UV
attributeDescriptions[2].binding = 0;
attributeDescriptions[2].location = 2;
attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;     // vec2
attributeDescriptions[2].offset = sizeof(float) * 6;

// 顶点绑定
VkVertexInputBindingDescription bindingDescription = {};
bindingDescription.binding = 0;
bindingDescription.stride = sizeof(float) * 8;  // 3(位置) + 3(法线) + 2(UV)
bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
vertexInputInfo.vertexBindingDescriptionCount = 1;
vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
vertexInputInfo.vertexAttributeDescriptionCount = 3;
vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;
```

### 8.4 动态状态

有些状态可以在**命令缓冲区录制时**动态设置，而不需要创建新的管线：

```cpp
// 声明哪些状态是动态的
VkDynamicState dynamicStates[] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR,
};

VkPipelineDynamicStateCreateInfo dynamicState = {};
dynamicState.dynamicStateCount = 2;
dynamicState.pDynamicStates = dynamicStates;

// 在命令缓冲区中设置
vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
```

### 8.5 管线布局（Pipeline Layout）

管线布局描述了 Shader 需要哪些资源（Uniform Buffer、纹理等）：

```cpp
// 描述符集布局
VkDescriptorSetLayoutBinding bindings[2] = {};

// binding 0: Uniform Buffer (MVP 矩阵)
bindings[0].binding = 0;
bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
bindings[0].descriptorCount = 1;
bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

// binding 1: 纹理
bindings[1].binding = 1;
bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
bindings[1].descriptorCount = 1;
bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

// 创建管线布局
VkPipelineLayoutCreateInfo layoutInfo = {};
layoutInfo.setLayoutCount = 1;
layoutInfo.pSetLayouts = &descriptorSetLayout;
layoutInfo.pushConstantRangeCount = 0;

vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipelineLayout);
```

### 8.6 管线缓存

Vulkan 允许你缓存管线对象，避免重复创建：

```cpp
// 创建管线缓存
VkPipelineCacheCreateInfo cacheInfo = {};
vkCreatePipelineCache(device, &cacheInfo, nullptr, &pipelineCache);

// 创建管线时使用缓存
vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineInfo, nullptr, &pipeline);

// 保存缓存到文件（下次启动时加载）
size_t cacheSize;
vkGetPipelineCacheData(device, pipelineCache, &cacheSize, nullptr);
std::vector<char> cacheData(cacheSize);
vkGetPipelineCacheData(device, pipelineCache, &cacheSize, cacheData.data());
// 写入文件...
```

---

## 附录 渲染管线术语速查表

| 术语 | 英文 | 含义 | 所在阶段 |
|------|------|------|----------|
| 应用阶段 | Application Phase | CPU 侧的准备工作 | CPU |
| 几何阶段 | Geometry Phase | 顶点处理、图元装配、裁剪 | GPU |
| 光栅化阶段 | Rasterization Phase | 三角形转片元、片元着色 | GPU |
| 输出合并 | Output Merger (OM) | 深度测试、模板测试、混合 | GPU |
| 顶点着色器 | Vertex Shader | 变换顶点坐标、传递属性 | 几何阶段（可编程） |
| 片元着色器 | Fragment Shader | 计算片元颜色 | 光栅化阶段（可编程） |
| 图元装配 | Primitive Assembly | 把顶点组装成三角形/线/点 | 几何阶段（固定） |
| 裁剪 | Clipping | 裁剪掉视锥体外的部分 | 几何阶段（固定） |
| 透视除法 | Perspective Division | 除以 w，得到 NDC | 几何阶段（固定） |
| 屏幕映射 | Screen Mapping | NDC → 屏幕坐标 | 几何阶段（固定） |
| 背面剔除 | Backface Culling | 剔除背对摄像机的三角形 | 几何阶段（可配置） |
| 三角形设置 | Triangle Setup | 计算边方程 | 光栅化（固定） |
| 三角形遍历 | Triangle Traversal | 找到三角形覆盖的像素 | 光栅化（固定） |
| 属性插值 | Attribute Interpolation | 用重心坐标插值顶点属性 | 光栅化（固定） |
| 深度测试 | Depth Test | 检查片元是否可见 | 输出合并（固定） |
| 模板测试 | Stencil Test | 用模板缓冲区做精细控制 | 输出合并（固定） |
| 混合 | Blending | 片元颜色和已有颜色混合 | 输出合并（可配置） |
| Early-Z | Early Depth Test | 在片元着色器前做深度测试 | 优化技术 |
| 过度绘制 | Overdraw | 同一像素被多次绘制 | 性能问题 |
| Z-Fighting | Z-Fighting | 共面三角形深度闪烁 | 精度问题 |
| G-Buffer | Geometry Buffer | 延迟渲染的几何信息缓冲区 | 延迟渲染 |
| 前向渲染 | Forward Rendering | 在片元着色器中计算光照 | 渲染方案 |
| 延迟渲染 | Deferred Rendering | 先写 G-Buffer，再统一光照 | 渲染方案 |
| Forward+ | Forward+ | 前向渲染的改进版 | 渲染方案 |
| 管线状态对象 | Pipeline State Object (PSO) | Vulkan 中打包所有管线状态 | Vulkan |
| 描述符集 | Descriptor Set | Vulkan 中 Shader 资源的绑定 | Vulkan |

---

> **最后的话**
>
> 渲染管线是一个**流水线**，每个阶段都有明确的输入和输出。理解了这个框架，你就知道：
>
> - **顶点着色器**负责坐标变换
> - **片元着色器**负责颜色计算
> - **固定功能阶段**负责"连接"和"过滤"
>
> 当你在 Vulkan 中配置管线状态时，你其实是在配置这些固定功能阶段的行为。当你在写 Shader 时，你是在编程那两个可编程阶段。
>
> 现在，打开你的 Vulkan 项目，对照这份手册，看看每个配置对应管线的哪个环节。你会发现，一切都有了意义。
