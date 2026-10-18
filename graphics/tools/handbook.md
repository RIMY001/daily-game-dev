# 渲染调试工具使用手册

> **写在前面的话**
>
> 如果你写了一段 Shader，跑出来是黑屏，然后盯着代码看了两小时还没找到问题——这份手册就是为你写的。
>
> 渲染调试的核心能力不是"一眼看出 bug"，而是**有一套系统的排查方法**。看到画面异常时，能快速判断问题出在管线的哪个阶段，然后用工具验证。
>
> 这份手册不讲"软件说明书"式的操作步骤，而是讲**诊断思路**：看到什么现象 → 怀疑什么 → 用什么工具 → 看什么数据 → 确认问题。
>
> 建议先把 RenderDoc 装好，跟着手册抓一帧你自己的项目，实际操作一遍。

---

## 目录

- [第一章 渲染调试的基本思路](#第一章-渲染调试的基本思路)
- [第二章 RenderDoc：渲染调试的瑞士军刀](#第二章-renderdoc渲染调试的瑞士军刀)
- [第三章 GPU 性能分析工具](#第三章-gpu-性能分析工具)
- [第四章 常见问题诊断流程](#第四章-常见问题诊断流程)
- [第五章 调试习惯与最佳实践](#第五章-调试习惯与最佳实践)
- [附录 现象 → 工具 → 数据速查表](#附录-现象--工具--数据速查表)

---

## 第一章 渲染调试的基本思路

### 1.1 核心原则：从现象反推管线阶段

渲染管线的每个阶段都有典型的"出错症状"。看到画面异常时，第一步是判断问题出在哪个阶段。

```
渲染管线各阶段的典型错误：

  应用阶段错误：
    → 模型不显示、Draw Call 缺失、物体位置错误
    
  几何阶段错误：
    → 模型变形、顶点位置错、法线方向错
    
  光栅化阶段错误：
    → 纹理错位、UV 拉伸、光照异常
    
  输出合并阶段错误：
    → 深度问题（Z-fighting）、混合异常（透明物体排序）、模板测试错误
```

### 1.2 四步排查法

```
第一步：观察现象
  → 画面哪里不对？全黑？花屏？模型变形？

第二步：缩小范围
  → 是几何问题还是着色问题？
  → 是所有物体都有问题还是只有特定物体？
  → 是静态的还是动态的？

第三步：用工具验证
  → 打开 RenderDoc / Frame Debugger
  → 查看具体的 Draw Call、纹理、状态

第四步：定位并修复
  → 确认是哪个参数/Shader/状态出了问题
  → 修复并验证
```

### 1.3 快速分类表

| 现象 | 最可能的阶段 | 首先检查 |
|------|-------------|----------|
| 全黑 | 应用/输出合并 | 清屏颜色、深度测试、混合模式 |
| 全白 | 着色 | 纹理是否加载、Shader 输出 |
| 模型不显示 | 应用/几何 | 剔除、视锥体、MVP 矩阵 |
| 模型变形 | 几何 | 顶点属性、MVP 矩阵、法线 |
| 纹理错位 | 光栅化 | UV 坐标、纹理绑定、采样器 |
| 闪烁/Z-fighting | 输出合并 | 深度精度、深度偏移 |
| 透明物体排序错 | 输出合并 | 混合模式、绘制顺序 |
| 花屏/条纹 | 同步/资源 | 资源同步、格式匹配 |
| 性能骤降 | 全局 | Draw Call 数、过度绘制、带宽 |

### 1.4 二分法排查

当问题范围不明确时，用**二分法**快速缩小：

```
方法一：逐个关闭效果
  1. 关闭所有后处理 → 问题还在？→ 不是后处理的问题
  2. 关闭光照 → 问题还在？→ 不是光照的问题
  3. 关闭纹理 → 问题还在？→ 不是纹理的问题
  
方法二：替换法
  1. 用一个已知正确的 Shader 替换 → 问题消失？→ 原 Shader 有问题
  2. 用一个已知正确的模型替换 → 问题消失？→ 原模型有问题
  
方法三：隔离法
  1. 只画一个最简单的三角形 → 正常？→ 问题在场景数据
  2. 只画一个最简单的模型 + 纯色 Shader → 正常？→ 问题在复杂 Shader
```

---

## 第二章 RenderDoc：渲染调试的瑞士军刀

### 2.1 为什么是 RenderDoc

RenderDoc 是目前最流行的开源渲染调试工具，支持 Vulkan、OpenGL、DirectX。它能：

- **抓取一帧**：记录这一帧所有的 Draw Call、状态、资源
- **回放**：在工具中重新执行这一帧，逐步查看
- **检查**：查看每个 Draw Call 的输入/输出、Shader 代码、管线状态
- **调试**：在 Shader 中设断点，逐像素调试

### 2.2 安装与启动

```
安装：
  1. 从 https://renderdoc.org 下载
  2. 安装（Windows 直接解压即可）

启动方式一：从 RenderDoc 启动程序
  1. 打开 RenderDoc
  2. File → Launch Program
  3. 选择你的可执行文件
  4. 设置 Working Directory、Command Line 等
  5. Launch

启动方式二：注入到已运行的程序
  1. 运行你的程序
  2. 在 RenderDoc 中 File → Attach to Running Process
  3. 选择你的进程
  4. Inject
```

### 2.3 抓帧

```
抓帧方法：
  1. 在 RenderDoc 窗口中，按 F12（或点击 Capture Frame）
  2. 程序会卡顿一下（正在抓取）
  3. 抓取完成后，自动打开帧分析界面
  
注意：
  - 抓帧会暂停程序，可能需要几秒到几十秒
  - 帧越复杂，抓取时间越长
  - 确保有足够的磁盘空间（一帧可能有几百 MB）
```

### 2.4 帧分析界面概览

```
RenderDoc 主界面：

┌─────────────────────────────────────────────────────────┐
│  Event Browser（事件列表）                                │
│  ┌─────────────────────────────────────────────────────┐│
│  │ Draw Call 列表（按时间顺序）                         ││
│  │  1. vkCmdBindPipeline                               ││
│  │  2. vkCmdBindDescriptorSets                         ││
│  │  3. vkCmdDrawIndexed (1000 vertices)                ││
│  │  4. ...                                             ││
│  └─────────────────────────────────────────────────────┘│
├─────────────────────────────────────────────────────────┤
│  Texture Viewer（纹理查看器）                             │
│  ┌─────────────────────────────────────────────────────┐│
│  │ 显示当前 Draw Call 的输入/输出纹理                   ││
│  │ 可以查看颜色、深度、法线等                           ││
│  └─────────────────────────────────────────────────────┘│
├─────────────────────────────────────────────────────────┤
│  Pipeline State（管线状态）                               │
│  ┌─────────────────────────────────────────────────────┐│
│  │ 显示当前 Draw Call 的完整管线状态                    ││
│  │ 包括：Shader、混合、深度、模板、光栅化等             ││
│  └─────────────────────────────────────────────────────┘│
├─────────────────────────────────────────────────────────┤
│  Mesh Viewer（网格查看器）                                │
│  ┌─────────────────────────────────────────────────────┐│
│  │ 显示当前 Draw Call 的网格                           ││
│  │ 可以查看顶点属性、线框、法线等                       ││
│  └─────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────┘
```

### 2.5 查看 Draw Call

```
操作：
  1. 在 Event Browser 中，点击任意 Draw Call
  2. 右侧的 Texture Viewer、Pipeline State、Mesh Viewer 会更新
  
看什么：
  - Draw Call 的数量和顺序
  - 每个 Draw Call 的顶点数、实例数
  - 绑定的是什么 Pipeline、Descriptor Set
  
诊断思路：
  - 如果某个物体没画 → 找到对应的 Draw Call，看是否被剔除
  - 如果 Draw Call 数量异常多 → 检查合批是否生效
  - 如果 Draw Call 顺序不对 → 检查渲染顺序（透明物体排序）
```

### 2.6 查看纹理（Texture Viewer）

```
操作：
  1. 选中一个 Draw Call
  2. 在 Texture Viewer 中，可以查看：
     - 输入纹理（绑定的纹理、Uniform Buffer）
     - 输出纹理（颜色附件、深度附件）
  
看什么：
  - 颜色附件：最终画了什么
  - 深度附件：深度值是否正确
  - 输入纹理：纹理是否正确加载、UV 是否正确
  
诊断思路：
  - 如果颜色附件全黑 → 检查 Shader 输出、混合模式
  - 如果深度附件全白/全黑 → 检查深度测试、清屏值
  - 如果纹理看起来不对 → 检查纹理格式、采样器设置
```

### 2.7 查看管线状态（Pipeline State）

```
操作：
  1. 选中一个 Draw Call
  2. 切换到 Pipeline State 标签
  
看什么：
  - Vertex Input：顶点属性格式、绑定
  - Input Assembly：图元类型（三角形、线、点）
  - Rasterization：剔除模式、多边形模式
  - Depth/Stencil：深度测试、模板测试配置
  - Color Blend：混合模式
  
诊断思路：
  - 如果模型不显示 → 检查剔除模式（是否剔除了正面）
  - 如果透明物体异常 → 检查混合模式
  - 如果深度问题 → 检查深度测试配置
```

### 2.8 查看网格（Mesh Viewer）

```
操作：
  1. 选中一个 Draw Call
  2. 切换到 Mesh Viewer 标签
  3. 可以查看：
     - 顶点位置（VS Input / VS Output）
     - 线框模式
     - 法线可视化
  
看什么：
  - 顶点位置是否正确（和预期一致）
  - 法线方向是否正确（蓝色/紫色）
  - UV 是否正确（展开后没有拉伸）
  
诊断思路：
  - 如果模型变形 → 检查 MVP 矩阵、顶点属性
  - 如果法线方向错 → 检查法线变换（逆转置矩阵）
  - 如果 UV 拉伸 → 检查 UV 坐标、纹理坐标
```

### 2.9 Shader 调试

```
操作：
  1. 选中一个 Draw Call
  2. 在 Pipeline State 中，点击 Shader 旁边的编辑图标
  3. 可以看到 Shader 的 SPIR-V / GLSL / HLSL 代码
  4. 在 Texture Viewer 中，右键点击像素 → Debug Pixel
  
看什么：
  - Shader 代码是否正确编译
  - 每个像素的 Shader 执行过程
  - 中间变量的值
  
诊断思路：
  - 如果 Shader 输出异常 → 逐行检查变量值
  - 如果某个像素颜色不对 → 调试该像素的 Shader 执行
  - 如果纹理采样不对 → 检查采样器设置、UV 坐标
```

### 2.10 常见操作速查

| 想做什么 | 在哪里操作 |
|----------|-----------|
| 看某一帧的所有 Draw Call | Event Browser |
| 看某个 Draw Call 的输出 | Texture Viewer → Output Color |
| 看某个 Draw Call 的深度 | Texture Viewer → Output Depth |
| 看某个 Draw Call 的输入纹理 | Texture Viewer → Input |
| 看管线状态 | Pipeline State 标签 |
| 看网格 | Mesh Viewer 标签 |
| 调试某个像素的 Shader | Texture Viewer → 右键 → Debug Pixel |
| 查看顶点数据 | Mesh Viewer → VS Input / VS Output |
| 对比两帧 | File → Load Capture → 对比 |

---

## 第三章 GPU 性能分析工具

### 3.1 工具概览

| 工具 | 平台 | 定位 | 适用场景 |
|------|------|------|----------|
| **RenderDoc** | 全平台 | 渲染调试 | 画面问题、Shader 调试 |
| **NVIDIA Nsight Graphics** | NVIDIA GPU | 性能分析 + 渲染调试 | PC 端性能优化 |
| **AMD Radeon GPU Profiler** | AMD GPU | 性能分析 | PC 端 AMD 显卡优化 |
| **Windows PIX** | DirectX | 渲染调试 + 性能分析 | DirectX 游戏 |
| **Unity Frame Debugger** | Unity | 渲染调试 | Unity 项目 |
| **Unreal Insights** | Unreal | 性能分析 | Unreal 项目 |

### 3.2 NVIDIA Nsight Graphics

#### 定位

NVIDIA 官方的 GPU 性能分析工具，功能比 RenderDoc 更强，但只支持 NVIDIA GPU。

#### 入口

```
安装：
  从 NVIDIA 官网下载 Nsight Graphics

启动：
  1. 打开 Nsight Graphics
  2. File → Launch Application
  3. 选择你的程序
  4. Launch
  
抓帧：
  和 RenderDoc 类似，按 F12 抓帧
```

#### 核心功能

```
Frame Graph：
  → 可视化整帧的 Draw Call 和 Pass
  → 快速定位性能瓶颈
  
GPU Trace：
  → 查看每个 Draw Call 的 GPU 耗时
  → 分析 Vertex / Fragment / Bandwidth 瓶颈
  
Metrics Counter：
  → 收集 GPU 硬件计数器
  → 分析 Shader 效率、缓存命中率
  
API Debugging：
  → 检查 API 使用错误
  → 验证资源同步
```

#### 什么时候用 Nsight 而不是 RenderDoc

```
用 RenderDoc：
  - 画面问题（黑屏、花屏、纹理错）
  - Shader 调试
  - 跨平台（AMD / Intel / NVIDIA）
  
用 Nsight Graphics：
  - 性能优化（需要精确的 GPU 耗时）
  - 分析 GPU 瓶颈类型（Vertex / Fragment / Bandwidth）
  - 只针对 NVIDIA GPU
```

### 3.3 AMD Radeon GPU Profiler (RGP)

#### 定位

AMD 官方的 GPU 性能分析工具，类似 Nsight，但针对 AMD GPU。

#### 入口

```
安装：
  从 AMD 官网下载 Radeon Developer Tool Suite

启动：
  1. 打开 RGP
  2. Launch Application
  3. 选择你的程序
  
分析：
  RGP 会记录整帧的 GPU 活动
  生成详细的性能报告
```

#### 核心功能

```
Frame Timeline：
  → 可视化 CPU 和 GPU 的时间线
  → 分析 CPU-GPU 同步
  
Shader Analysis：
  → 分析 Shader 的指令数、寄存器使用
  → 优化 Shader 效率
  
Memory Analysis：
  → 分析显存使用
  → 优化带宽
```

### 3.4 Windows PIX

#### 定位

微软官方的 DirectX 调试工具，适合 DirectX 12 游戏。

#### 入口

```
安装：
  从 Windows SDK 或 Xbox GDK 安装 PIX

启动：
  1. 打开 PIX
  2. Capture GPU → Capture Application
  3. 选择你的 DirectX 程序
  
抓帧：
  按 F12 或点击 Capture
```

#### 核心功能

```
GPU Timeline：
  → 可视化 GPU 的执行时间线
  → 分析 GPU 瓶颈
  
Draw Call Timing：
  → 精确测量每个 Draw Call 的 GPU 耗时
  
Shader Debugger：
  → 调试 DirectX Shader
```

### 3.5 Unity Frame Debugger

#### 定位

Unity 内置的帧分析工具，适合 Unity 项目。

#### 入口

```
在 Unity 编辑器中：
  Window → Analysis → Frame Debugger
  
或：
  在 Game 视图中，点击 Frame Debug 按钮
```

#### 核心功能

```
Draw Call 列表：
  → 按顺序列出所有 Draw Call
  → 每个 Draw Call 的名称、材质、Shader
  
逐步查看：
  → 可以逐步查看每个 Draw Call 的效果
  → 快速定位哪个 Draw Call 有问题
  
Shader 查看：
  → 查看每个 Draw Call 使用的 Shader
  → 查看 Shader 的属性
```

#### 什么时候用 Unity Frame Debugger

```
适用场景：
  - Unity 项目的快速调试
  - 不需要深入的 GPU 性能分析
  - 快速定位哪个 Draw Call 有问题
  
不适用：
  - 需要精确的 GPU 耗时（用 RenderDoc 或 Profiler）
  - 需要 Shader 调试（用 RenderDoc）
  - 非 Unity 项目
```

### 3.6 工具选择建议

```
画面问题：
  → RenderDoc（通用、强大）
  → Unity Frame Debugger（Unity 项目快速定位）
  
性能问题（PC）：
  → NVIDIA Nsight（NVIDIA GPU）
  → AMD RGP（AMD GPU）
  → RenderDoc（通用，但性能分析不如专用工具）
  
性能问题（移动端）：
  → Snapdragon Profiler（Qualcomm）
  → ARM Streamline（Mali）
  → Xcode GPU Frame Capture（iOS）
  
引擎内调试：
  → Unity Frame Debugger（Unity）
  → Unreal Insights（Unreal）
```

---

## 第四章 常见问题诊断流程

### 4.1 黑屏

#### 可能的原因

```
1. 清屏颜色就是黑色，且没有画任何东西
2. 深度测试配置错误（所有片元都被丢弃）
3. 混合模式错误（Alpha = 0）
4. Shader 输出全黑
5. MVP 矩阵错误（所有顶点都在视锥体外）
6. 纹理未加载（采样返回黑色）
```

#### 诊断流程

```
第一步：检查清屏颜色
  → 在 RenderDoc 中，查看第一个 Clear 命令
  → 清屏颜色是什么？
  → 如果清屏是黑色 → 可能是没画东西
  
第二步：检查 Draw Call
  → 在 Event Browser 中，查看是否有 Draw Call
  → 如果没有 → 应用阶段问题（剔除、场景遍历）
  → 如果有 → 继续
  
第三步：检查输出纹理
  → 选中最后一个 Draw Call
  → 查看 Output Color
  → 如果是黑色 → 继续
  
第四步：检查深度测试
  → 在 Pipeline State 中，查看 Depth Test
  → 深度测试是否开启？比较方式是什么？
  → 如果深度测试配置错误 → 修复
  
第五步：检查 Shader
  → 在 Texture Viewer 中，调试一个像素
  → 查看 Shader 的输出
  → 如果输出是黑色 → Shader 问题
  
第六步：检查纹理
  → 在 Texture Viewer 中，查看输入纹理
  → 纹理是否正确加载？
  → 如果纹理是黑色 → 纹理加载问题
```

### 4.2 花屏 / 条纹

#### 可能的原因

```
1. 资源同步错误（CPU 还在写，GPU 已经在读）
2. 纹理格式不匹配（数据格式和采样格式不一致）
3. 内存越界（Shader 读取了未初始化的数据）
4. 交换链问题（帧缓冲未正确同步）
```

#### 诊断流程

```
第一步：观察花屏模式
  → 是随机的？还是有规律的条纹？
  → 随机 → 可能是同步错误或内存越界
  → 有规律 → 可能是格式不匹配
  
第二步：检查资源同步
  → 在 RenderDoc 中，查看资源的使用历史
  → 是否有资源在同一帧中被写入和读取？
  → 如果有 → 添加同步（Fence / Semaphore）
  
第三步：检查纹理格式
  → 在 Pipeline State 中，查看纹理的格式
  → 创建时的格式和采样时的格式是否一致？
  → 如果不一致 → 修复格式
  
第四步：检查 Shader
  → 调试 Shader，查看读取的数据
  → 是否有未初始化的变量？
  → 如果有 → 初始化变量
```

### 4.3 Z-Fighting（深度闪烁）

#### 可能的原因

```
1. 两个面共面（深度值非常接近）
2. 深度精度不足（深度缓冲区位数不够）
3. 近远裁剪面比例过大（深度精度分布不均）
```

#### 诊断流程

```
第一步：确认是 Z-Fighting
  → 在 RenderDoc 中，查看深度纹理
  → 闪烁区域的深度值是否非常接近？
  → 如果是 → Z-Fighting
  
第二步：检查是否共面
  → 在 Mesh Viewer 中，查看两个面的顶点位置
  → 是否共面？
  → 如果共面 → 添加深度偏移（Polygon Offset）
  
第三步：检查深度精度
  → 深度缓冲区的格式是什么？
  → 是 16 位还是 24 位？
  → 如果是 16 位 → 改用 24 位
  
第四步：检查近远裁剪面
  → near 和 far 的值是多少？
  → far / near 是否 > 10000？
  → 如果是 → 减小 far 或增大 near
```

### 4.4 模型缺失

#### 可能的原因

```
1. 被剔除了（视锥剔除、背面剔除）
2. MVP 矩阵错误（模型在视锥体外）
3. 没有提交 Draw Call
4. 深度测试失败
```

#### 诊断流程

```
第一步：检查 Draw Call
  → 在 RenderDoc 中，查看是否有该模型的 Draw Call
  → 如果没有 → 应用阶段问题
  
第二步：检查剔除
  → 在 Pipeline State 中，查看 Cull Mode
  → 是否剔除了正面？
  → 如果是 → 修改剔除模式
  
第三步：检查 MVP 矩阵
  → 在 Mesh Viewer 中，查看顶点位置
  → 顶点位置是否在合理范围内？
  → 如果不在 → 检查 MVP 矩阵
  
第四步：检查深度测试
  → 在 Texture Viewer 中，查看深度纹理
  → 该模型的深度值是否合理？
  → 如果不合理 → 检查深度测试配置
```

### 4.5 纹理错位

#### 可能的原因

```
1. UV 坐标错误
2. 纹理绑定错误（绑定了错误的纹理）
3. 采样器配置错误（地址模式、过滤模式）
4. UV 变换错误（Tiling / Offset）
```

#### 诊断流程

```
第一步：检查 UV 坐标
  → 在 Mesh Viewer 中，查看顶点的 UV 坐标
  → UV 是否在 [0, 1] 范围内？
  → 如果不合理 → 检查模型的 UV 展开
  
第二步：检查纹理绑定
  → 在 Pipeline State 中，查看绑定的纹理
  → 是否绑定了正确的纹理？
  → 如果绑定了错误的纹理 → 修复绑定
  
第三步：检查采样器
  → 在 Pipeline State 中，查看采样器配置
  → 地址模式是什么？（Repeat / Clamp / Mirror）
  → 过滤模式是什么？（Nearest / Linear）
  → 如果配置错误 → 修复
  
第四步：调试 Shader
  → 在 Texture Viewer 中，调试一个像素
  → 查看 UV 坐标和采样结果
  → 如果 UV 错误 → 检查 Shader 中的 UV 计算
```

### 4.6 性能突然下降

#### 可能的原因

```
1. Draw Call 数量激增
2. 过度绘制严重
3. 新加了复杂的效果（后处理、光照）
4. 纹理/缓冲区过大（带宽瓶颈）
5. CPU 瓶颈（场景遍历、物理、AI）
```

#### 诊断流程

```
第一步：测量帧时间
  → CPU 帧时间是多少？
  → GPU 帧时间是多少？
  → CPU >> GPU → CPU 瓶颈
  → GPU >> CPU → GPU 瓶颈
  
第二步：如果是 CPU 瓶颈
  → 检查 Draw Call 数量
  → 检查场景遍历开销
  → 检查物理 / AI 开销
  
第三步：如果是 GPU 瓶颈
  → 用 Nsight / RGP 分析瓶颈类型
  → Vertex Bound？→ 检查顶点数、LOD
  → Fragment Bound？→ 检查过度绘制、Shader 复杂度
  → Bandwidth Bound？→ 检查纹理大小、渲染目标
  
第四步：对比变化
  → 最近改了什么？
  → 新加了什么效果？
  → 新加了多少模型？
  → 定位到具体的变化
```

---

## 第五章 调试习惯与最佳实践

### 5.1 日常开发中的调试习惯

#### 习惯一：频繁抓帧

```
不要等到出了问题再抓帧
每完成一个功能，抓一帧，保存下来
→ 可以作为基准，对比后续的变化
→ 可以快速定位"什么时候开始出问题的"
```

#### 习惯二：用可视化调试

```
在 Shader 中添加可视化输出：

  // 可视化法线
  outColor = vec4(normal * 0.5 + 0.5, 1.0);
  
  // 可视化 UV
  outColor = vec4(uv, 0.0, 1.0);
  
  // 可视化深度
  outColor = vec4(vec3(depth), 1.0);
  
  // 可视化世界坐标
  outColor = vec4(worldPos * 0.1, 1.0);
```

#### 习惯三：逐步验证

```
不要一次写完所有代码再测试
每写一个功能，就测试一下：
  1. 写完顶点着色器 → 测试顶点位置
  2. 写完片元着色器 → 测试颜色输出
  3. 加纹理 → 测试纹理采样
  4. 加光照 → 测试光照计算
```

#### 习惯四：记录问题

```
遇到问题时，记录：
  - 现象是什么？
  - 怎么复现？
  - 最后解决了？
  - 根因是什么？
  
→ 积累经验，下次遇到类似问题可以快速定位
```

### 5.2 调试工具的使用建议

#### 建议一：先 RenderDoc，再性能工具

```
画面问题 → 先用 RenderDoc
性能问题 → 先用引擎内置工具（Frame Debugger / Insights）
         → 再用 RenderDoc 深入
         → 最后用 Nsight / RGP 精确分析
```

#### 建议二：学会看纹理

```
纹理是最直观的信息：
  - 颜色纹理 → 看最终输出
  - 深度纹理 → 看深度分布
  - 法线纹理 → 看法线方向
  - UV 纹理 → 看 UV 坐标
  
在 RenderDoc 中，经常查看这些纹理
→ 可以快速发现问题
```

#### 建议三：学会看 Draw Call

```
Draw Call 列表是管线的缩影：
  - Draw Call 数量 → 合批是否生效
  - Draw Call 顺序 → 渲染顺序是否正确
  - Draw Call 状态 → 管线配置是否正确
  
在 RenderDoc 中，经常浏览 Draw Call 列表
→ 可以快速定位问题
```

### 5.3 常见误区

#### 误区一：只看代码，不看工具

```
错误：
  盯着代码看两小时，找不到问题
  
正确：
  先用工具抓帧，看实际数据
  → 可能一眼就看出问题
```

#### 误区二：盲目优化

```
错误：
  "我觉得 Draw Call 太多了，合批一下"
  → 但实际瓶颈是 Shader 太复杂
  
正确：
  先测量，找到瓶颈
  → 再针对性优化
```

#### 误区三：不记录问题

```
错误：
  遇到问题，解决了就忘了
  → 下次遇到类似问题，又要重新排查
  
正确：
  记录问题和解决方案
  → 积累经验，提高效率
```

---

## 附录 现象 → 工具 → 数据速查表

| 现象 | 首先用什么工具 | 看什么数据 | 可能的原因 |
|------|---------------|-----------|-----------|
| 黑屏 | RenderDoc | Output Color、Draw Call 列表 | 深度测试、混合、Shader 输出 |
| 花屏 | RenderDoc | 资源历史、纹理格式 | 同步错误、格式不匹配 |
| Z-fighting | RenderDoc | 深度纹理、Mesh Viewer | 共面、深度精度、近远裁剪面 |
| 模型缺失 | RenderDoc | Draw Call 列表、Pipeline State | 剔除、MVP 矩阵、深度测试 |
| 纹理错位 | RenderDoc | Mesh Viewer（UV）、Pipeline State | UV 坐标、纹理绑定、采样器 |
| 性能下降 | Nsight / RGP | GPU Trace、Metrics | Draw Call、过度绘制、带宽 |
| 模型变形 | RenderDoc | Mesh Viewer（VS Input/Output） | MVP 矩阵、顶点属性 |
| 透明排序错 | RenderDoc | Draw Call 列表、Output Color | 绘制顺序、混合模式 |
| 光照异常 | RenderDoc | Shader Debug、输入纹理 | 法线、光方向、Shader 计算 |
| 后处理异常 | RenderDoc | 后处理 Pass 的输入/输出 | 后处理 Shader、纹理绑定 |

---

> **最后的话**
>
> 渲染调试是一门**实践技能**。看再多教程，不如自己抓一帧、看一遍。
>
> 现在，打开 RenderDoc，抓一帧你自己的项目。看看 Draw Call 列表，看看纹理输出，看看管线状态。你会发现，很多"神秘"的问题，其实一眼就能看出来。
>
> 记住三个原则：
> 1. **从现象反推管线阶段**：看到问题，先判断是哪个阶段
> 2. **用工具验证猜测**：不要猜，要看数据
> 3. **记录问题和解决方案**：积累经验，提高效率
>
> 最后，调试能力是**练出来的**。多抓帧、多看、多思考，你会越来越快。
