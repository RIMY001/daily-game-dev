# Piccolo 游戏引擎源码学习手册

> 版本：基于 Piccolo 0.0.9 源码  
> 定位：先骨架后细节的源码阅读指南  
> 原则：不复制大段源码，不逐行解析函数实现；只讲类职责、调用关系、数据流

---

## 前言

Piccolo 是一个教学向的 C++ 游戏引擎，覆盖反射、物理、动画、粒子、Vulkan 渲染、编辑器等完整模块。它的架构设计清晰、代码量适中（runtime 约 200 个文件），非常适合作为"从零理解游戏引擎架构"的学习素材。

**阅读本手册的建议**：

1. 严格按照章节顺序学习——每一章都建立在前一章的理解之上。
2. 每章先读"核心骨架知识点"，建立全局印象；再打开对应源文件验证。
3. 遇到【后面精读，现阶段跳过】标记的函数，先理解它的输入输出和调用位置即可，不要深入实现。
4. 每章末尾的"课后思考题"用来检验理解——如果答不上来，说明骨架还没搭好，回头重读。

**你需要具备的前置知识**：
- C++ 基础（类继承、智能指针、模板、虚函数）
- 了解 JSON 格式
- 了解 Vulkan 的基本概念（Instance、Device、Swapchain、CommandBuffer、RenderPass）——第 9-11 章需要
- 了解游戏引擎的基本概念（ECS 或 Component 模式、游戏循环、坐标系）

---

## 环境与构建说明

### 项目目录总览

```
Piccolo-0.0.9/
├── CMakeLists.txt              # 顶层 CMake
├── build_windows.bat           # Windows 构建脚本
├── build_linux.sh              # Linux 构建脚本
├── build_macos.sh              # macOS 构建脚本
│
└── engine/
    ├── CMakeLists.txt          # 引擎构建入口（定义所有构建目标）
    ├── configs/                # 引擎配置（deployment/ + development/）
    ├── asset/                  # 运行时资产（JSON 格式的世界/关卡/物体/纹理）
    ├── shader/                 # GLSL 着色器 + 着色器编译 CMake
    ├── jolt-asset/             # Jolt 物理引擎资产
    ├── template/               # Mustache 代码生成模板
    │
    └── source/
        ├── precompile/         # 构建时代码生成配置
        ├── meta_parser/        # 反射/序列化代码生成工具（编译期）
        ├── runtime/            # 引擎核心运行时库
        │   ├── engine.h/.cpp   # PiccoloEngine 顶层类
        │   ├── core/           # 基础设施（数学、日志、反射、序列化）
        │   ├── function/       # 功能子系统（渲染、物理、动画、粒子、输入...）
        │   ├── platform/       # 平台抽象（文件系统、路径）
        │   └── resource/       # 资源管理（AssetManager、ConfigManager、数据结构）
        ├── editor/             # 编辑器应用
        └── test/               # 测试（当前为空）
```

### 构建目标链

```
PiccoloEditor (可执行文件)
    └── PiccoloRuntime (库)
            └── PiccoloPreCompile (代码生成步骤)
                    └── PiccoloParser (代码生成工具, 依赖 libclang)
```

### 构建步骤（Windows）

```bash
# 1. 运行构建脚本
build_windows.bat

# 2. 构建过程会自动：
#    a. 编译 PiccoloParser（反射代码生成工具）
#    b. 运行 PiccoloParser 扫描头文件，生成反射/序列化代码
#    c. 编译 PiccoloRuntime
#    d. 编译 PiccoloEditor
#    e. 编译着色器
```

### 关键依赖

| 依赖 | 用途 |
|------|------|
| Vulkan SDK | 渲染后端 |
| GLFW | 窗口管理 |
| Jolt Physics | 物理引擎 |
| libclang (LLVM) | 编译期头文件解析（meta_parser） |
| spdlog | 日志 |
| ImGui | 编辑器 UI |
| VMA (Vulkan Memory Allocator) | Vulkan 内存管理 |

---

## 第1章 顶层入口：PiccoloEngine + RuntimeGlobalContext

### 学习目标

读完本章需要理解：
- `main()` 函数在哪里，启动到主循环经历了哪些步骤
- 12 个子系统以什么顺序创建、以什么顺序销毁
- 一帧主循环的完整时序：从 `calculateDeltaTime` 到 `pollEvents`
- `RuntimeGlobalContext` 作为全局系统容器的角色

### 需要阅读的文件列表

- `engine/source/editor/source/main.cpp`
- `engine/source/runtime/engine.h`
- `engine/source/runtime/engine.cpp`
- `engine/source/runtime/function/global/global_context.h`
- `engine/source/runtime/function/global/global_context.cpp`

### 核心骨架知识点

#### 1.1 程序入口 main()

位置：`editor/source/main.cpp:15`

启动流程（6 步）：

```
main()
  ├─ 1. new PiccoloEngine()
  ├─ 2. engine->startEngine(config_path)     // 注册反射类型 + 创建12个子系统
  ├─ 3. engine->initialize()                  // 当前为空实现
  ├─ 4. new PiccoloEditor() → editor->initialize(engine)
  ├─ 5. editor->run()                         // ★ 进入主循环（编辑器模式）
  └─ 6. 清理：editor->clear() → engine->clear() → engine->shutdownEngine()
```

**关键理解**：编辑器模式下，主循环由 `PiccoloEditor::run()` 驱动，而非 `PiccoloEngine::run()`。`Engine::run()` 是给纯游戏模式用的。

#### 1.2 PiccoloEngine 职责

| 方法 | 职责 |
|------|------|
| `startEngine(config_path)` | 调用 `TypeMetaRegister::Register()` 注册反射类型，然后调用 `g_runtime_global_context.startSystems()` |
| `shutdownEngine()` | 逆序销毁所有子系统，注销反射类型 |
| `run()` | 纯游戏模式的主循环（while + tickOneFrame） |
| `tickOneFrame(delta_time)` | 一帧的完整时序（见下方） |
| `calculateDeltaTime()` | 用 `steady_clock` 计算帧间隔 |
| `logicalTick(dt)` | 逻辑更新：WorldManager.tick() + InputSystem.tick() |
| `rendererTick(dt)` | 渲染更新：RenderSystem.tick() |
| `calculateFPS(dt)` | 指数滑动平均计算帧率 |

#### 1.3 RuntimeGlobalContext — 全局系统容器

位置：`function/global/global_context.h`

全局单例：`g_runtime_global_context`

持有 12 个子系统的 `std::shared_ptr`：

| 序号 | 子系统 | 初始化顺序 |
|------|--------|-----------|
| 1 | `ConfigManager` | 最先——解析配置，后续系统需要读取路径 |
| 2 | `FileSystem` | 文件服务 |
| 3 | `LogSystem` | 日志——后续系统需要 LOG_INFO 等 |
| 4 | `AssetManager` | 资产 I/O |
| 5 | `PhysicsManager` | Jolt 物理初始化 |
| 6 | `WorldManager` | 世界/关卡管理 |
| 7 | `WindowSystem` | GLFW 窗口创建 |
| 8 | `InputSystem` | 输入处理 |
| 9 | `ParticleManager` | 粒子系统 |
| 10 | `RenderSystem` | Vulkan 渲染（依赖 WindowSystem） |
| 11 | `DebugDrawManager` | 调试绘制 |
| 12 | `RenderDebugConfig` | 调试配置开关 |

**销毁顺序**：严格逆序（12 → 1）。

**设计要点**：初始化顺序体现了依赖关系——`RenderSystem` 依赖 `WindowSystem`（需要 GLFW 窗口来创建 Vulkan Surface），`WorldManager` 依赖 `AssetManager`（需要加载关卡），等等。

#### 1.4 一帧完整时序（tickOneFrame）

```
tickOneFrame(delta_time)
  │
  ├─ ① logicalTick(dt)
  │    ├─ WorldManager.tick(dt)     → Level.tick(dt) → GObject/Component 更新
  │    └─ InputSystem.tick()        → 轮询键鼠状态
  │
  ├─ ② calculateFPS(dt)            → 更新帧率显示
  │
  ├─ ③ RenderSystem.swapLogicRenderData()  → 逻辑/渲染数据双缓冲交换
  │
  ├─ ④ rendererTick(dt)
  │    └─ RenderSystem.tick(dt)     → 处理交换数据 → 可见性剔除 → 渲染管线执行
  │
  ├─ ⑤ [可选] PhysicsManager.renderPhysicsWorld()  → 物理调试绘制
  │
  ├─ ⑥ WindowSystem.pollEvents()   → GLFW 事件分发
  │
  └─ ⑦ WindowSystem.setTitle()     → 更新窗口标题（含 FPS）
```

### 在引擎时序中的位置

本章内容就是引擎的"骨架中的骨架"——所有其他模块都挂在这个时序框架上。

### 课后思考题

1. 为什么 `RenderSystem` 的初始化必须在 `WindowSystem` 之后？如果调换顺序会发生什么？
2. `PiccoloEngine::run()` 和 `PiccoloEditor::run()` 的区别是什么？为什么编辑器要自己驱动主循环？
3. 在 `tickOneFrame` 中，为什么 `swapLogicRenderData()` 必须在 `logicalTick()` 之后、`rendererTick()` 之前？
4. 如果要在引擎中新增一个子系统（比如音频系统），你需要修改哪些文件、遵循什么顺序？
5. `shutdownSystems()` 为什么要按初始化的逆序销毁？哪些子系统之间存在依赖？

### 后续精读标记

- 【跳过】`PiccoloEngine::calculateFPS()` 的指数滑动平均算法细节
- 【跳过】`TypeMetaRegister::Register()` 内部注册了哪些类型（第 3 章会讲）
- 【跳过】`editor->run()` 内部编辑器独有的 tick 顺序（第 12 章会讲）

---

## 第2章 游戏对象框架：WorldManager、Level、GObject、Component

### 学习目标

读完本章需要理解：
- 引擎如何从 JSON 加载一个世界 → 关卡 → 游戏对象
- `GObject` 和 `Component` 的关系（类 ECS 架构）
- 一帧中逻辑更新的完整调用链：WorldManager → Level → GObject → Component
- 7 种 Component 各自负责什么

### 需要阅读的文件列表

- `engine/source/runtime/function/framework/world/world_manager.h`
- `engine/source/runtime/function/framework/world/world_manager.cpp`
- `engine/source/runtime/function/framework/level/level.h`
- `engine/source/runtime/function/framework/level/level.cpp`
- `engine/source/runtime/function/framework/object/object.h`
- `engine/source/runtime/function/framework/object/object.cpp`
- `engine/source/runtime/function/framework/component/component.h`
- `engine/source/runtime/function/framework/component/transform/transform_component.h`
- `engine/source/runtime/function/framework/component/camera/camera_component.h`
- `engine/source/runtime/function/framework/component/mesh/mesh_component.h`
- `engine/source/runtime/function/framework/component/rigidbody/rigidbody_component.h`
- `engine/source/runtime/function/framework/component/motor/motor_component.h`
- `engine/source/runtime/function/framework/component/animation/animation_component.h`
- `engine/source/runtime/function/framework/component/particle/particle_component.h`
- `engine/source/runtime/function/character/character.h`
- `engine/source/runtime/function/controller/character_controller.h`

### 核心骨架知识点

#### 2.1 WorldManager — 世界管理器

**职责**：管理世界/关卡的生命周期，驱动当前活跃关卡的 tick。

**关键成员**：
- `m_current_world_url` — 当前世界资源路径
- `m_current_world_resource` — 当前世界资源数据（`WorldRes`）
- `m_loaded_levels` — 已加载关卡的 map（url → Level）
- `m_current_active_level` — 当前活跃关卡（`weak_ptr<Level>`）
- `m_level_debugger` — 调试器

**tick 流程**：
```
WorldManager.tick(dt)
  ├─ [首帧] loadWorld() → 读取 WorldRes JSON → loadLevel() → Level.load()
  └─ [每帧] m_current_active_level->tick(dt)
            m_level_debugger->tick(active_level)
```

**当前限制**：只支持一个活跃世界 + 一个活跃关卡。

#### 2.2 Level — 关卡

**职责**：管理关卡内所有 GObject，驱动物体 tick 和物理场景 tick。

**关键成员**：
- `m_gobjects` — `unordered_map<GObjectID, shared_ptr<GObject>>`
- `m_current_active_character` — 当前活跃角色
- `m_physics_scene` — 关卡对应的物理场景

**Level.load() 流程**：
```
Level.load(level_url)
  ├─ AssetManager.loadAsset() → 读取 LevelRes JSON
  ├─ PhysicsManager.createPhysicsScene(gravity) → 创建物理场景
  ├─ 遍历 level_res.m_objects → createObject() → GObject.load()
  └─ 查找角色名 → 创建 Character
```

**Level.tick() 时序**：
```
Level.tick(dt)
  ├─ 遍历所有 GObject → gobject.tick(dt)
  ├─ [非编辑器模式] Character.tick(dt)
  └─ PhysicsScene.tick(dt)    ← 物理步进
```

**关键理解**：物理步进在所有 GObject tick 之后执行。这意味着组件的状态更新先于物理模拟。

#### 2.3 GObject — 游戏实体

**职责**：游戏世界中的一个实体，持有一组 Component。

**关键成员**：
- `m_id` — `GObjectID`（由 `ObjectIDAllocator` 分配）
- `m_name` — 实体名称
- `m_definition_url` — 物体定义资源路径
- `m_components` — `vector<ReflectionPtr<Component>>`（反射指针数组，支持多态）

**GObject.tick()**：
```
GObject.tick(dt)
  └─ 遍历 m_components → 如果 shouldComponentTick(type_name) → component.tick(dt)
```

**编辑器模式过滤**：编辑器模式下只有 `TransformComponent` 和 `MeshComponent` 会 tick（在 `editor.cpp` 构造函数中注册）。游戏模式下所有组件都 tick。

**GObject.load()**：
```
GObject.load(ObjectInstanceRes)
  ├─ 加载实例化组件（m_instanced_components）
  ├─ 加载物体定义（ObjectDefinitionRes JSON）
  └─ 合并：定义中的组件如果与实例组件类型不重复，则追加
```

**设计要点**：GObject 本身不包含逻辑，所有行为都委托给 Component。这是典型的"组合优于继承"设计。

#### 2.4 Component — 组件基类

**职责**：所有组件的基类。提供 tick、dirty 标记、父对象引用。

**关键成员**：
- `m_parent_object` — `weak_ptr<GObject>`，指向所属的 GObject
- `m_is_dirty` — 脏标记（用于标记需要更新渲染数据等）
- `m_is_scale_dirty` — 缩放脏标记
- `m_tick_in_editor_mode` — 是否在编辑器模式下 tick

**关键方法**：
- `tick(dt)` — 虚函数，子类重写实现各自逻辑
- `postLoadResource(parent)` — 加载后回调，设置父对象引用

#### 2.5 七种 Component

| 组件 | 文件 | 职责 |
|------|------|------|
| `TransformComponent` | `component/transform/transform_component.h` | 位置/旋转/缩放，双缓冲变换矩阵 |
| `CameraComponent` | `component/camera/camera_component.h` | 相机参数（FOV、远近裁剪面），支持第一/三/自由视角 |
| `MeshComponent` | `component/mesh/mesh_component.h` | 持有网格资源描述和材质描述，向渲染系统提交渲染数据 |
| `RigidBodyComponent` | `component/rigidbody/rigidbody_component.h` | 桥接游戏对象与物理刚体，同步变换 |
| `MotorComponent` | `component/motor/motor_component.h` | 角色运动控制（行走、跳跃、冲刺），读取 InputSystem 命令 |
| `AnimationComponent` | `component/animation/animation_component.h` | 骨骼动画状态，驱动 Skeleton 更新 |
| `ParticleComponent` | `component/particle/particle_component.h` | 粒子发射器，向 ParticleManager 提交粒子数据 |

#### 2.6 Character 与 Controller

- `Character` — 封装一个 GObject 作为玩家角色，管理相机模式切换
- `CharacterController` — 基于物理的角色控制器（胶囊体扫掠检测）

### 在引擎时序中的位置

```
logicalTick(dt)
  └─ WorldManager.tick(dt)
       └─ Level.tick(dt)
            ├─ 遍历 GObject.tick(dt)
            │    └─ 遍历 Component.tick(dt)    ← ★ 本章核心
            ├─ Character.tick(dt)
            └─ PhysicsScene.tick(dt)
```

### 课后思考题

1. `GObject` 的 `m_components` 为什么使用 `ReflectionPtr<Component>` 而不是普通的 `Component*` 或 `unique_ptr<Component>`？这与序列化有什么关系？
2. `Level.tick()` 中，为什么物理步进（`PhysicsScene.tick()`）放在所有 `GObject.tick()` 之后？如果放在之前会有什么问题？
3. 编辑器模式下只让 `TransformComponent` 和 `MeshComponent` tick，其他组件不 tick——这样做的理由是什么？
4. `GObject.load()` 中"实例化组件"和"定义组件"的合并策略是什么？如果一个组件同时存在于实例和定义中，谁优先？
5. `RigidBodyComponent` 需要在物理步进前后分别做什么操作？（提示：变换同步）

### 后续精读标记

- 【跳过】`TransformComponent` 的双缓冲变换矩阵具体如何实现
- 【跳过】`MotorComponent` 的跳跃/冲刺物理计算
- 【跳过】`MeshComponent` 如何向 `RenderSwapContext` 提交渲染数据的具体字段
- 【跳过】`RigidBodyComponent` 与 `PhysicsScene` 的刚体 ID 映射细节
- 【跳过】`CharacterController` 的胶囊体扫掠算法

---

## 第3章 编译期反射与序列化

### 学习目标

读完本章需要理解：
- C++ 没有原生反射，Piccolo 如何用编译期工具实现反射
- `meta_parser` 工具的工作流程：libclang 解析 → AST 遍历 → Mustache 模板生成
- 反射系统运行时的核心类：`TypeMeta`、`FieldAccessor`、`ReflectionPtr`
- 序列化器 `PSerializer` 如何利用反射实现 JSON ↔ 对象转换
- 整个流程在构建过程中的位置

### 需要阅读的文件列表

**编译期工具（meta_parser）**：
- `engine/source/meta_parser/parser/main.cpp`
- `engine/source/meta_parser/parser/parser/parser.h`
- `engine/source/meta_parser/parser/parser/parser.cpp`
- `engine/source/meta_parser/parser/cursor/cursor.h`
- `engine/source/meta_parser/parser/language_types/type_info.h`
- `engine/source/meta_parser/parser/language_types/class.h`
- `engine/source/meta_parser/parser/language_types/field.h`
- `engine/source/meta_parser/parser/generator/reflection_generator.h`
- `engine/source/meta_parser/parser/generator/serializer_generator.h`
- `engine/source/meta_parser/parser/template_manager/template_manager.h`
- `engine/template/allReflectionFile.mustache`
- `engine/template/allSerializer.h.mustache`

**运行时反射系统**：
- `engine/source/runtime/core/meta/reflection/reflection.h`
- `engine/source/runtime/core/meta/reflection/reflection_register.h`
- `engine/source/runtime/core/meta/serializer/serializer.h`
- `engine/source/runtime/core/meta/json.h`

### 核心骨架知识点

#### 3.1 为什么需要反射

Piccolo 的资产全部是 JSON 格式。要将 JSON 自动转换为 C++ 对象，需要知道：
- 一个类有哪些字段
- 每个字段的类型名
- 如何根据类型名创建对象实例
- 如何按字段名读写字段值

这些信息在 C++ 运行时是不可用的，所以 Piccolo 在编译期用工具自动生成。

#### 3.2 宏标记系统

在头文件中用宏标记需要反射的类：

```cpp
// 声明一个可反射的类
REFLECTION_TYPE(Component)
CLASS(Component, WhiteListFields)
{
    REFLECTION_BODY(Component)
    // ...
};
```

| 宏 | 作用 |
|----|------|
| `REFLECTION_TYPE(ClassName)` | 前向声明反射操作类 |
| `CLASS(ClassName, ...)` | 标记类需要反射，附带注解（如 WhiteListFields） |
| `REFLECTION_BODY(ClassName)` | 声明 friend 类，允许反射系统访问私有字段 |
| `META(...)` | 字段级注解（编译期通过 `__attribute__((annotate()))` 传递） |

**关键理解**：这些宏在正常编译时几乎为空（`META(...)` 展开为空），它们的主要作用是为 `meta_parser` 工具提供 libclang 可识别的 AST 注解。

#### 3.3 meta_parser 工作流程

```
构建时触发 PiccoloPreCompile
  │
  ├─ 1. 收集所有需要扫描的头文件列表（precompile.json.in）
  │
  ├─ 2. PiccoloParser 运行
  │     ├─ libclang 解析头文件 → CXTranslationUnit
  │     ├─ Cursor 遍历 AST → 提取 Class、Field 信息
  │     ├─ 读取 META(...) 注解 → MetaInfo（键值对元数据）
  │     └─ 按 SchemaModule 组织解析结果
  │
  ├─ 3. 代码生成
  │     ├─ ReflectionGenerator → 生成反射注册代码
  │     ├─ SerializerGenerator → 生成序列化/反序列化代码
  │     └─ TemplateManager + Mustache 模板 → 渲染输出文件
  │
  └─ 4. 输出到 _generated/ 目录
        ├─ _generated/reflection/   ← 反射注册代码
        └─ _generated/serializer/   ← 序列化代码
```

#### 3.4 运行时反射核心类

**`TypeMeta`** — 类型元数据入口
- `newMetaFromName(type_name)` — 根据类型名查找已注册的类型信息
- `getFieldByName(name)` — 获取字段访问器
- `getFieldsList()` — 获取所有字段列表
- `newFromNameAndPJson(type_name, json)` — 从 JSON 创建对象实例
- `writeByName(type_name, instance)` — 将对象序列化为 JSON

**`FieldAccessor`** — 字段访问器
- `get(instance)` — 获取字段值（返回 `void*`）
- `set(instance, value)` — 设置字段值
- `getFieldName()` / `getFieldTypeName()` — 字段名/类型名
- `isArrayType()` — 是否为数组类型
- `getTypeMeta(out_type)` — 获取字段类型的 TypeMeta（用于嵌套反射）

**`ArrayAccessor`** — 数组字段访问器（因为 C++ 无法反射 `std::vector` 的内部结构）
- `get(index, instance)` / `set(index, instance, value)` — 按索引读写
- `getSize(instance)` — 获取数组长度
- `getElementTypeName()` — 元素类型名

**`ReflectionPtr<T>`** — 带类型名的智能指针
- 持有 `m_type_name`（字符串）和 `m_instance`（裸指针）
- 支持类型转换（`operator ReflectionPtr<U>()`）
- 用于 `GObject::m_components`，使得组件数组可以存储多态对象并在序列化时保留类型信息

**`ReflectionInstance`** — 类型元数据 + 实例指针的组合

#### 3.5 注册机制

**`TypeMetaRegisterinterface`** — 注册接口
- `registerToClassMap(name, ClassFunctionTuple*)` — 注册类的构造/序列化函数
- `registerToFieldMap(name, FieldFunctionTuple*)` — 注册字段的 get/set 函数
- `registerToArrayMap(name, ArrayFunctionTuple*)` — 注册数组类型的访问函数

**`TypeMetaRegister`**（`reflection_register.h`）
- `Register()` — 调用所有生成代码的注册函数
- `Unregister()` — 注销所有注册

**`ClassFunctionTuple`** = `(GetBaseClassList, ConstructorWithPJson, WritePJsonByName)`  
**`FieldFunctionTuple`** = `(SetFunction, GetFunction, GetNameFunction, GetTypeNameFunction, GetFieldNameFunction, GetBoolFunction)`

**关键理解**：这些 tuple 存储的是 `std::function`，由生成的代码填充。运行时通过类型名查表找到对应的函数，实现"根据字符串创建对象/读写字段"。

#### 3.6 序列化器 PSerializer

位置：`core/meta/serializer/serializer.h`

- 利用反射系统，对任意注册过的类型进行 JSON ↔ 对象转换
- `loadAsset(json_str, out_object)` → 通过 `TypeMeta::newFromNameAndPJson()` 递归构造
- `saveAsset(object, out_json)` → 通过 `TypeMeta::writeByName()` 递归序列化

### 在引擎时序中的位置

```
构建期：
  PiccoloParser → 扫描头文件 → 生成 _generated/reflection/ + _generated/serializer/

启动期：
  PiccoloEngine::startEngine()
    └─ TypeMetaRegister::Register()    ← 将所有类型的反射函数注册到全局 map

运行期：
  AssetManager.loadAsset() → PSerializer → TypeMeta → FieldAccessor → 递归构造对象

关闭期：
  PiccoloEngine::shutdownEngine()
    └─ TypeMetaRegister::Unregister()  ← 清空全局 map
```

### 课后思考题

1. 为什么 Piccolo 选择在编译期生成反射代码，而不是用运行时宏或手动注册？这种方案的优缺点是什么？
2. `ReflectionPtr<T>` 为什么要同时保存 `m_type_name` 和 `m_instance`？只用 `m_instance` 不够吗？（提示：考虑序列化场景）
3. `FieldAccessor` 的 `get()` 返回 `void*`，调用者如何知道实际类型？这个信息从哪里来？
4. 如果你新增了一个 C++ 类想要支持 JSON 序列化，你需要做哪些步骤？
5. `meta_parser` 生成的代码放在 `_generated/` 目录——这个目录应该在版本控制中吗？为什么？

### 后续精读标记

- 【跳过】`meta_parser` 中 libclang Cursor 遍历 AST 的具体实现
- 【跳过】Mustache 模板的具体语法和生成代码的格式
- 【跳过】`FieldFunctionTuple` 中每个 `std::function` 的生成代码细节
- 【跳过】`PSerializer` 的递归序列化/反序列化实现
- 【跳过】`ReflectionPtr` 的完整赋值运算符和类型转换实现

---

## 第4章 资源与配置系统

### 学习目标

读完本章需要理解：
- `ConfigManager` 如何管理引擎配置
- `AssetManager` 如何利用反射系统加载/保存 JSON 资产
- 资源数据结构的组织方式（`res_type/` 目录）
- 资产路径解析流程

### 需要阅读的文件列表

- `engine/source/runtime/resource/config_manager/config_manager.h`
- `engine/source/runtime/resource/config_manager/config_manager.cpp`
- `engine/source/runtime/resource/asset_manager/asset_manager.h`
- `engine/source/runtime/resource/asset_manager/asset_manager.cpp`
- `engine/source/runtime/resource/res_type/common/world.h`
- `engine/source/runtime/resource/res_type/common/level.h`
- `engine/source/runtime/resource/res_type/common/object.h`
- `engine/source/runtime/resource/res_type/components/mesh.h`
- `engine/source/runtime/resource/res_type/components/camera.h`
- `engine/source/runtime/resource/res_type/components/rigid_body.h`
- `engine/source/runtime/resource/res_type/data/material.h`
- `engine/source/runtime/resource/res_type/data/mesh_data.h`
- `engine/source/runtime/resource/res_type/global/global_rendering.h`
- `engine/source/runtime/resource/res_type/global/global_particle.h`
- `engine/source/runtime/platform/file_service/file_service.h`
- `engine/source/runtime/platform/path/path.h`
- `engine/configs/development/PiccoloEditor.ini`

### 核心骨架知识点

#### 4.1 ConfigManager — 配置管理器

**职责**：解析引擎配置文件（INI 格式），提供各类资源路径。

**关键方法**：
- `initialize(config_file_path)` — 解析配置文件
- `getDefaultWorldUrl()` — 获取默认世界资源路径
- `getGlobalRenderingResUrl()` — 获取全局渲染配置路径
- `getGlobalParticleResUrl()` — 获取全局粒子配置路径

**在初始化时序中的位置**：第一个被创建的系统——所有后续系统都可能需要读取配置。

#### 4.2 AssetManager — 资产管理器

**职责**：基于反射+JSON 的资产加载/保存。

**关键方法**：
- `loadAsset(asset_url, out_res)` — 读取 JSON 文件 → 反序列化为 C++ 对象
- `saveAsset(res, asset_url)` — 序列化 C++ 对象 → 写入 JSON 文件
- `getFullPath(relative_path)` — 将相对路径转为绝对路径

**loadAsset 流程**：
```
AssetManager.loadAsset(url, out_res)
  ├─ getFullPath(url) → 绝对路径
  ├─ 读取文件内容 → JSON 字符串
  ├─ PJson::parse(json_str) → PJson 对象
  └─ PSerializer::readFromJson(json, out_res)
       └─ 利用反射系统递归构造对象（第3章内容）
```

#### 4.3 资源数据结构组织（res_type/）

```
res_type/
├── common/          # 顶层资源类型
│   ├── world.h      # WorldRes — 世界描述（含默认关卡URL）
│   ├── level.h      # LevelRes — 关卡数据（物体列表、重力、角色名）
│   └── object.h     # ObjectInstanceRes + ObjectDefinitionRes
│
├── components/      # 组件资源类型（组件的序列化数据结构）
│   ├── mesh.h       # MeshComponentRes
│   ├── camera.h     # CameraComponentRes
│   ├── rigid_body.h # RigidBodyComponentRes
│   ├── animation.h  # AnimationComponentRes
│   ├── motor.h      # MotorComponentRes
│   └── emitter.h    # ParticleComponentRes
│
├── data/            # 底层数据类型
│   ├── mesh_data.h       # 网格数据（顶点、索引）
│   ├── material.h        # 材质数据
│   ├── animation_clip.h  # 动画关键帧数据
│   ├── skeleton_data.h   # 骨骼定义
│   ├── blend_state.h     # 动画混合状态
│   └── skeleton_mask.h   # 骨骼遮罩
│
└── global/          # 全局配置
    ├── global_rendering.h  # IBL 贴图路径、相机配置、光照参数
    └── global_particle.h   # 全局粒子配置
```

#### 4.4 资产目录结构（engine/asset/）

```
asset/
├── world/       # 世界 JSON（引用关卡路径）
├── level/       # 关卡 JSON（引用物体定义）
├── objects/     # 物体定义 JSON（引用组件数据）
├── texture/     # 纹理文件（jpg, hdr 等）
└── global/      # 全局配置 JSON
```

**数据引用链**：
```
WorldRes → LevelRes → ObjectInstanceRes → ObjectDefinitionRes
                                              ├─ MeshComponentRes → .mesh / .glb
                                              ├─ RigidBodyComponentRes → 碰撞形状
                                              ├─ AnimationComponentRes → .skeleton / .anim
                                              └─ MaterialRes → 纹理路径
```

### 在引擎时序中的位置

```
初始化：ConfigManager(1st) → AssetManager(4th)

运行时加载：
  WorldManager.loadWorld() → AssetManager.loadAsset(WorldRes)
  Level.load()             → AssetManager.loadAsset(LevelRes)
  GObject.load()           → AssetManager.loadAsset(ObjectDefinitionRes)
  Component.postLoadResource() → 各组件自行加载子资源

保存：
  Level.save() → GObject.save() → AssetManager.saveAsset(LevelRes)
```

### 课后思考题

1. `ConfigManager` 为什么必须是第一个初始化的系统？如果把它移到 `LogSystem` 之后会怎样？
2. `AssetManager.loadAsset()` 是一个模板方法——它如何知道要反序列化成什么类型？（提示：反射系统）
3. `ObjectInstanceRes` 和 `ObjectDefinitionRes` 分开设计的目的是什么？（提示：复用与覆盖）
4. 资源数据文件（`res_type/` 下的 .h）和组件类（`component/` 下的 .h）是什么关系？它们是同一个类吗？
5. 如果要把资产格式从 JSON 换成二进制格式，需要修改哪些模块？

### 后续精读标记

- 【跳过】`AssetManager` 的具体文件读取实现
- 【跳过】各 `res_type` 数据结构的字段细节（用到时再查）
- 【跳过】`Path` 工具类的路径拼接逻辑
- 【跳过】`FileSystem` 的文件遍历实现

---

## 第5章 输入系统 InputSystem

### 学习目标

读完本章需要理解：
- GLFW 键盘/鼠标回调如何与引擎的输入系统对接
- 游戏命令的位标志（bitflag）设计
- 输入状态如何在一帧中被采集和使用

### 需要阅读的文件列表

- `engine/source/runtime/function/input/input_system.h`
- `engine/source/runtime/function/input/input_system.cpp`
- `engine/source/runtime/function/render/window_system.h`
- `engine/source/runtime/function/render/window_system.cpp`

### 核心骨架知识点

#### 5.1 InputSystem 职责

**职责**：采集键鼠输入，将其映射为游戏命令（`GameCommand`）。

**关键成员**：
- `m_game_command` — `unsigned int`，位标志组合，表示当前帧按下的命令
- `m_cursor_delta_x/y` — 鼠标移动增量
- `m_cursor_delta_yaw/pitch` — 鼠标移动转换后的角度增量

#### 5.2 GameCommand 位标志

```cpp
enum class GameCommand : unsigned int
{
    forward    = 1 << 0,  // W
    backward   = 1 << 1,  // S
    left       = 1 << 2,  // A
    right      = 1 << 3,  // D
    jump       = 1 << 4,  // SPACE
    squat      = 1 << 5,  // 未实现
    sprint     = 1 << 6,  // LEFT SHIFT
    fire       = 1 << 7,  // 未实现
    free_carema = 1 << 8, // F
    invalid    = 1 << 31  // 窗口失去焦点
};
```

**设计要点**：使用位标志，多个命令可以在一个 `unsigned int` 中同时存在。`MotorComponent` 通过位运算检查命令。

#### 5.3 输入数据流

```
GLFW 键盘回调
  └─ WindowSystem → InputSystem.onKey()
       └─ onKeyInGameMode() → 设置/清除 m_game_command 的对应位

GLFW 鼠标回调
  └─ WindowSystem → InputSystem.onCursorPos()
       └─ 计算 cursor_delta → calculateCursorDeltaAngles()

每帧消费：
  MotorComponent.tick() → InputSystem.getGameCommand() → 位运算检查命令
  Character.tick() → InputSystem.m_cursor_delta_yaw/pitch → 相机旋转
```

#### 5.4 WindowSystem 的角色

**职责**：GLFW 窗口封装，负责创建窗口、注册回调、轮询事件。

**关键方法**：
- `initialize(WindowCreateInfo)` — 创建 GLFW 窗口，注册键盘/鼠标/窗口resize回调
- `pollEvents()` — 调用 `glfwPollEvents()`，分发输入事件
- `shouldClose()` — 检查窗口是否请求关闭
- `setTitle()` — 设置窗口标题

**回调注册**：GLFW 回调函数作为静态函数注册，内部转发到 `InputSystem` 的方法。

### 在引擎时序中的位置

```
每帧：
  logicalTick(dt)
    ├─ WorldManager.tick(dt)    ← 组件消费输入（MotorComponent 读取 GameCommand）
    └─ InputSystem.tick()       ← 注意：在 WorldManager 之后！

  帧尾：
    WindowSystem.pollEvents()   ← GLFW 事件分发，触发 onKey/onCursorPos 回调
```

**关键理解**：`InputSystem.tick()` 在 `WorldManager.tick()` 之后调用，但 `pollEvents()` 在帧尾。这意味着当前帧消费的是上一帧 `pollEvents` 采集的输入。

### 课后思考题

1. `GameCommand` 为什么用位标志而不是枚举数组？这样设计有什么优势？
2. `InputSystem.tick()` 在 `WorldManager.tick()` 之后调用——这是否意味着当前帧的逻辑使用的是上一帧的输入？这样设计有什么影响？
3. 如果窗口失去焦点，`m_game_command` 被设置为 `invalid`——`MotorComponent` 应该如何处理这个状态？
4. 鼠标增量从像素到角度的转换（`calculateCursorDeltaAngles`）需要考虑什么因素？（提示：灵敏度、FOV）

### 后续精读标记

- 【跳过】`calculateCursorDeltaAngles()` 的具体角度计算
- 【跳过】`WindowSystem` 的 GLFW 回调注册细节
- 【跳过】窗口 resize 时的处理逻辑

---

## 第6章 物理子系统

### 学习目标

读完本章需要理解：
- Jolt Physics 在 Piccolo 中的封装层次
- `PhysicsManager` 和 `PhysicsScene` 的职责划分
- `RigidBodyComponent` 如何桥接游戏对象与物理世界
- 物理查询接口（raycast、sweep、overlap）

### 需要阅读的文件列表

- `engine/source/runtime/function/physics/physics_manager.h`
- `engine/source/runtime/function/physics/physics_manager.cpp`
- `engine/source/runtime/function/physics/physics_scene.h`
- `engine/source/runtime/function/physics/physics_scene.cpp`
- `engine/source/runtime/function/physics/physics_config.h`
- `engine/source/runtime/function/framework/component/rigidbody/rigidbody_component.h`
- `engine/source/runtime/function/framework/component/rigidbody/rigidbody_component.cpp`
- `engine/source/runtime/function/physics/jolt/utils.h`（如存在）

### 核心骨架知识点

#### 6.1 PhysicsManager — 物理管理器

**职责**：管理多个 `PhysicsScene` 的生命周期。

**关键成员**：
- `m_scenes` — `vector<shared_ptr<PhysicsScene>>`

**关键方法**：
- `initialize()` — 初始化 Jolt Physics 全局状态
- `createPhysicsScene(gravity)` — 创建一个新的物理场景
- `deletePhysicsScene(scene)` — 删除物理场景
- `renderPhysicsWorld(dt)` — [调试] 绘制物理世界可视化

**设计要点**：每个 Level 拥有独立的 PhysicsScene，支持不同的重力配置。

#### 6.2 PhysicsScene — 物理场景

**职责**：封装一个完整的 Jolt Physics 系统，提供刚体管理和物理查询。

**关键内部结构**：
```cpp
struct JoltPhysics {
    JPH::PhysicsSystem*            m_jolt_physics_system;  // Jolt 核心系统
    JPH::JobSystem*                m_jolt_job_system;       // Jolt 任务系统
    JPH::TempAllocator*            m_temp_allocator;        // 临时内存分配器
    JPH::BroadPhaseLayerInterface* m_jolt_broad_phase_layer_interface;
    int m_collision_steps;
    int m_integration_substeps;
};
```

**关键方法**：
- `createRigidBody(transform, res)` → 返回 `body_id`（uint32_t）
- `removeRigidBody(body_id)`
- `updateRigidBodyGlobalTransform(body_id, transform)` — 强制更新刚体位置（用于编辑器拖拽等）
- `tick(dt)` — 物理步进（碰撞检测 + 积分）
- `raycast(origin, direction, length, out_hits)` → 射线检测
- `sweep(shape, transform, direction, length, out_hits)` → 形状扫掠
- `isOverlap(shape, transform)` → 重叠检测

**`PhysicsHitInfo`**：
```cpp
struct PhysicsHitInfo {
    Vector3  hit_position;
    Vector3  hit_normal;
    float    hit_distance;
    uint32_t body_id;
};
```

#### 6.3 RigidBodyComponent — 刚体组件

**职责**：桥接 GObject 与 PhysicsScene 中的刚体。

**数据流**：
```
逻辑 → 物理：
  TransformComponent 更新位置 → RigidBodyComponent.tick()
    → PhysicsScene.updateRigidBodyGlobalTransform()

物理 → 逻辑：
  PhysicsScene.tick() 模拟 → RigidBodyComponent 读取新位置
    → 更新 TransformComponent
```

**关键理解**：`RigidBodyComponent` 持有 Jolt 刚体的 `body_id`，通过 `PhysicsScene` 间接操作。它不直接持有 Jolt 对象指针。

### 在引擎时序中的位置

```
Level.tick(dt)
  ├─ 遍历 GObject.tick(dt)
  │    └─ RigidBodyComponent.tick(dt)
  │         ├─ 读取 TransformComponent → 更新 Jolt 刚体位置（运动学刚体）
  │         └─ 从 Jolt 刚体读取新位置 → 写回 TransformComponent
  │
  └─ PhysicsScene.tick(dt)     ← 物理模拟步进
```

### 课后思考题

1. 为什么每个 Level 有独立的 PhysicsScene，而不是全局共享一个？
2. `RigidBodyComponent` 为什么不直接持有 `JPH::Body*` 指针，而是通过 `body_id` 间接引用？
3. 物理步进在 `GObject.tick()` 之后——这意味着本帧的组件逻辑看到的物理状态是上一帧的。这对游戏逻辑有什么影响？
4. `raycast` 和 `sweep` 的区别是什么？各自适用于什么场景？
5. 如果角色控制器（`CharacterController`）使用 `sweep` 做碰撞检测，它与 `RigidBodyComponent` 的物理模拟是什么关系？

### 后续精读标记

- 【跳过】`PhysicsScene` 中 Jolt 的初始化细节（BroadPhaseLayer、ObjectLayer 等）
- 【跳过】`PhysicsScene::tick()` 中 `PhysicsSystem::Update()` 的参数含义
- 【跳过】`RigidBodyComponent` 的碰撞形状创建流程
- 【跳过】Jolt 的层/掩码配置
- 【跳过】物理调试渲染器的实现

---

## 第7章 动画系统

### 学习目标

读完本章需要理解：
- 骨骼动画的基本数据流：SkeletonData → AnimationClip → Skeleton → 骨骼矩阵
- `AnimationManager` 的静态缓存设计
- `AnimationComponent` 如何驱动动画更新
- 动画混合（Blend）的基本概念

### 需要阅读的文件列表

- `engine/source/runtime/function/animation/animation_system.h`
- `engine/source/runtime/function/animation/animation_system.cpp`
- `engine/source/runtime/function/animation/skeleton.h`
- `engine/source/runtime/function/animation/skeleton.cpp`
- `engine/source/runtime/function/animation/node.h`
- `engine/source/runtime/function/animation/node.cpp`
- `engine/source/runtime/function/animation/animation_loader.h`
- `engine/source/runtime/function/animation/utilities.h`
- `engine/source/runtime/function/framework/component/animation/animation_component.h`
- `engine/source/runtime/function/framework/component/animation/animation_component.cpp`
- `engine/source/runtime/resource/res_type/data/animation_clip.h`
- `engine/source/runtime/resource/res_type/data/skeleton_data.h`
- `engine/source/runtime/resource/res_type/data/blend_state.h`
- `engine/source/runtime/resource/res_type/data/skeleton_mask.h`
- `engine/source/runtime/resource/res_type/data/animation_skeleton_node_map.h`

### 核心骨架知识点

#### 7.1 AnimationManager — 静态缓存

**职责**：加载并缓存骨骼定义、动画片段、骨骼映射、骨骼遮罩。

**关键特征**：所有成员和方法都是 `static`——这是一个纯静态缓存类，不需要实例化。

**四个缓存**：
| 缓存 | 类型 | 数据来源 |
|------|------|---------|
| `m_skeleton_definition_cache` | `SkeletonData` | 骨骼定义文件 |
| `m_animation_data_cache` | `AnimationClip` | 动画关键帧文件 |
| `m_animation_skeleton_map_cache` | `AnimSkelMap` | 动画→骨骼映射 |
| `m_skeleton_mask_cache` | `BoneBlendMask` | 骨骼混合遮罩 |

**关键方法**：
- `tryLoadSkeleton(path)` — 从缓存或文件加载
- `tryLoadAnimation(path)` — 同上
- `tryLoadAnimationSkeletonMap(path)` — 同上
- `tryLoadSkeletonMask(path)` — 同上
- `getBlendStateWithClipData(blend_state)` — 根据混合状态获取对应的动画剪辑数据

#### 7.2 Node 与 Bone

**`Node`** — 骨骼层级树节点
- 表示骨骼层级结构中的一个节点
- 持有局部变换（Local Transform）

**`Bone`** — 继承自 Node
- 表示可蒙皮的骨骼
- 持有绑定姿态（Bind Pose）变换

#### 7.3 Skeleton — 骨骼实例

**职责**：根据骨骼定义构建运行时骨骼实例，应用动画，输出最终的骨骼矩阵。

**关键成员**：
- `m_bones` — `Bone*` 数组（扁平化存储）
- `m_bone_count` — 骨骼数量
- `m_is_flat` — 是否已扁平化

**关键方法**：
- `buildSkeleton(SkeletonData)` — 从定义数据构建骨骼数组
- `applyAnimation(BlendStateWithClipData)` — 将动画关键帧应用到骨骼的局部变换
- `outputAnimationResult()` → `AnimationResult`（包含最终的骨骼矩阵数组）
- `resetSkeleton()` — 重置为绑定姿态

**数据流**：
```
SkeletonData (定义)
  → Skeleton.buildSkeleton()     → 构建 Bone 数组
  → Skeleton.applyAnimation()    → 应用动画关键帧到 Bone 局部变换
  → Skeleton.outputAnimationResult() → 输出矩阵数组 → 传给渲染系统做蒙皮
```

#### 7.4 AnimationComponent — 动画组件

**职责**：在每帧 tick 中驱动动画更新。

**tick 流程**：
```
AnimationComponent.tick(dt)
  ├─ 更新动画播放时间
  ├─ 构建 BlendState（混合状态）
  ├─ AnimationManager.getBlendStateWithClipData()
  ├─ Skeleton.applyAnimation(blend_data)
  └─ Skeleton.outputAnimationResult() → 存入组件，等待渲染系统读取
```

**输出数据**：`AnimationResult`（骨骼矩阵数组）通过 `RenderSwapContext` 传递给渲染系统，用于 GPU 蒙皮。

### 在引擎时序中的位置

```
GObject.tick(dt)
  └─ AnimationComponent.tick(dt)     ← 更新骨骼矩阵

RenderSystem.processSwapData()
  └─ 读取骨骼矩阵 → 写入 RenderEntity.m_joint_matrices → GPU 蒙皮
```

### 课后思考题

1. `AnimationManager` 为什么所有方法都是 static？这样设计有什么优缺点？
2. `Skeleton` 为什么要区分 `buildSkeleton` 和 `applyAnimation` 两步？能不能合并？
3. `BoneBlendMask`（骨骼遮罩）的作用是什么？举例说明什么场景需要它（提示：上半身/下半身独立动画）。
4. 动画结果（骨骼矩阵）如何从逻辑线程传递到渲染线程？（提示：回顾 RenderSwapContext）
5. `AnimSkelMap` 的作用是什么？为什么动画和骨骼之间需要一个映射？

### 后续精读标记

- 【跳过】`Skeleton::applyAnimation()` 的具体插值算法
- 【跳过】`Skeleton::outputAnimationResult()` 中局部→世界矩阵的计算
- 【跳过】`AnimationLoader` 的文件解析细节
- 【跳过】`utilities.h` 中的动画工具函数
- 【跳过】动画混合的权重计算细节

---

## 第8章 粒子系统

### 学习目标

读完本章需要理解：
- `ParticleManager` 的职责和接口
- 粒子数据如何通过 `RenderSwapContext` 传递给渲染侧
- `ParticleComponent` 的角色
- 粒子渲染在渲染管线中的位置（`ParticlePass`）

### 需要阅读的文件列表

- `engine/source/runtime/function/particle/particle_manager.h`
- `engine/source/runtime/function/particle/particle_manager.cpp`
- `engine/source/runtime/function/particle/particle_desc.h`
- `engine/source/runtime/function/particle/particle_common.h`
- `engine/source/runtime/function/particle/emitter_id_allocator.h`
- `engine/source/runtime/function/framework/component/particle/particle_component.h`
- `engine/source/runtime/function/framework/component/particle/particle_component.cpp`
- `engine/source/runtime/resource/res_type/components/emitter.h`
- `engine/source/runtime/resource/res_type/global/global_particle.h`
- `engine/source/runtime/function/render/passes/particle_pass.h`（仅接口）

### 核心骨架知识点

#### 8.1 ParticleManager — 粒子管理器

**职责**：管理粒子发射器的创建，持有全局粒子配置。

**关键成员**：
- `m_global_particle_res` — 全局粒子资源配置

**关键方法**：
- `initialize()` — 加载全局粒子配置
- `createParticleEmitter(particle_res, transform_desc)` — 创建粒子发射器
- `setParticlePass(ParticlePass*)` — 设置渲染侧的粒子 Pass

**设计要点**：`ParticleManager` 本身不模拟粒子——粒子模拟在 GPU 侧的 `ParticlePass` 中完成。`ParticleManager` 只是发射器的生命周期管理器。

#### 8.2 粒子数据流

```
逻辑侧：
  ParticleComponent.tick(dt)
    → 向 RenderSwapContext 提交：
       ├─ ParticleSubmitRequest（新建发射器）
       ├─ EmitterTickRequest（需要更新的发射器ID列表）
       └─ EmitterTransformRequest（发射器变换更新）

交换：
  RenderSystem.swapLogicRenderData()

渲染侧：
  RenderSystem.processSwapData()
    → 读取 SwapData 中的粒子请求
    → 传递给 ParticlePass：
       ├─ createEmitter() — 创建 GPU 发射器
       ├─ setTickIndices() — 标记需要更新的发射器
       └─ setTransformIndices() — 更新发射器变换

  RenderPipeline.deferredRender()
    → ParticlePass 执行 GPU 粒子模拟 + 渲染
```

#### 8.3 ParticleComponent — 粒子组件

**职责**：将粒子发射器附着到 GObject 上，每帧向渲染系统提交粒子数据。

**tick 时做的事**：
- 通过 `RenderSwapContext` 提交发射器的变换更新
- 提交需要 tick 的发射器 ID

#### 8.4 关键数据结构

| 结构 | 用途 |
|------|------|
| `ParticleEmitterDesc` | 发射器描述（粒子数量、生命周期、速度等） |
| `ParticleEmitterTransformDesc` | 发射器变换（位置/旋转/缩放） |
| `ParticleEmitterID` | 发射器唯一标识 |
| `GlobalParticleRes` | 全局粒子配置资源 |

### 在引擎时序中的位置

```
逻辑侧：
  GObject.tick(dt)
    └─ ParticleComponent.tick(dt) → 向 SwapContext 提交粒子数据

渲染侧：
  RenderSystem.tick(dt)
    ├─ processSwapData() → 处理粒子请求 → 传给 ParticlePass
    └─ deferredRender() → ParticlePass 执行粒子渲染
```

### 课后思考题

1. Piccolo 的粒子模拟是在 CPU 还是 GPU 上进行的？从代码结构如何判断？
2. 粒子数据为什么要通过 `RenderSwapContext` 传递，而不是直接调用 `ParticlePass` 的方法？
3. `ParticleManager` 持有 `ParticlePass*` 指针——这个指针是什么时候设置的？（提示：在 `RenderSystem::initialize` 中创建管线时）
4. 发射器的变换为什么要每帧更新？如果 GObject 移动了，发射器不更新变换会怎样？

### 后续精读标记

- 【跳过】`ParticlePass` 的 GPU 粒子模拟实现（Compute Shader）
- 【跳过】粒子发射器的具体参数含义（shape、spawn rate、lifetime 等）
- 【跳过】`EmitterIDAllocator` 的分配策略
- 【跳过】粒子渲染的 draw call 组织方式

---

## 第9章 渲染系统总览

### 学习目标

读完本章需要理解：
- `RenderSystem` 作为渲染总调度的角色
- `WindowSystem` 与 `RenderSystem` 的关系
- `RenderSwapContext` 双缓冲机制的设计目的和数据内容
- `RenderScene`、`RenderCamera`、`RenderResource` 的职责划分
- `RenderSystem.tick()` 的完整流程

### 需要阅读的文件列表

- `engine/source/runtime/function/render/render_system.h`
- `engine/source/runtime/function/render/render_system.cpp`
- `engine/source/runtime/function/render/window_system.h`
- `engine/source/runtime/function/render/window_system.cpp`
- `engine/source/runtime/function/render/render_swap_context.h`
- `engine/source/runtime/function/render/render_scene.h`
- `engine/source/runtime/function/render/render_scene.cpp`
- `engine/source/runtime/function/render/render_camera.h`
- `engine/source/runtime/function/render/render_camera.cpp`
- `engine/source/runtime/function/render/render_resource.h`
- `engine/source/runtime/function/render/render_resource_base.h`
- `engine/source/runtime/function/render/render_entity.h`
- `engine/source/runtime/function/render/render_object.h`
- `engine/source/runtime/function/render/render_common.h`
- `engine/source/runtime/function/render/render_type.h`
- `engine/source/runtime/function/render/light.h`
- `engine/source/runtime/function/render/render_guid_allocator.h`

### 核心骨架知识点

#### 9.1 RenderSystem — 渲染总调度

**职责**：持有渲染侧所有核心对象，驱动每帧渲染流程。

**关键成员**：
| 成员 | 类型 | 职责 |
|------|------|------|
| `m_rhi` | `shared_ptr<RHI>` | 渲染硬件抽象层（Vulkan 后端） |
| `m_render_camera` | `shared_ptr<RenderCamera>` | 渲染相机（view/projection 矩阵） |
| `m_render_scene` | `shared_ptr<RenderScene>` | 渲染场景数据（实体、光源、可见列表） |
| `m_render_resource` | `shared_ptr<RenderResourceBase>` | GPU 资源管理（网格、材质、IBL 贴图上传） |
| `m_render_pipeline` | `shared_ptr<RenderPipelineBase>` | 渲染管线（持有所有 RenderPass） |
| `m_swap_context` | `RenderSwapContext` | 逻辑/渲染双缓冲 |
| `m_render_pipeline_type` | `RENDER_PIPELINE_TYPE` | 当前管线类型（延迟/前向） |

**initialize 流程**：
```
RenderSystem.initialize(info)
  ├─ 创建 VulkanRHI → rhi.initialize()      ← Vulkan 设备、交换链等
  ├─ 加载全局渲染资源（IBL 贴图、颜色分级贴图）
  ├─ 创建 RenderResource → 上传全局 GPU 资源
  ├─ 创建 RenderCamera → 设置初始位姿
  ├─ 创建 RenderScene → 设置环境光、方向光
  └─ 创建 RenderPipeline → pipeline.initialize()  ← 创建所有 RenderPass
```

**tick 流程**：
```
RenderSystem.tick(dt)
  ├─ ① processSwapData()                ← 处理逻辑→渲染的交换数据
  ├─ ② m_rhi->prepareContext()           ← 准备渲染命令上下文
  ├─ ③ m_render_resource->updatePerFrameBuffer()  ← 更新每帧 Uniform Buffer
  ├─ ④ m_render_scene->updateVisibleObjects()      ← 可见性剔除
  ├─ ⑤ m_render_pipeline->preparePassData()        ← 准备各 Pass 数据
  ├─ ⑥ DebugDrawManager.tick(dt)
  └─ ⑦ m_render_pipeline->deferredRender() 或 forwardRender()  ← 执行渲染
```

#### 9.2 RenderSwapContext — 逻辑/渲染双缓冲

**设计目的**：逻辑线程（组件 tick）和渲染线程（RenderSystem tick）需要交换数据，但不能直接互相访问对方的数据结构。双缓冲解决了这个问题。

**结构**：
```cpp
class RenderSwapContext {
    RenderSwapData m_swap_data[2];  // 索引 0 = 逻辑侧，索引 1 = 渲染侧
    uint8_t m_logic_swap_data_index;
    uint8_t m_render_swap_data_index;

    void swapLogicRenderData();  // 交换两个索引
};
```

**RenderSwapData 包含的数据**：
| 字段 | 用途 |
|------|------|
| `m_level_resource_desc` | 关卡级资源变更（IBL、颜色分级） |
| `m_game_object_resource_desc` | 新建/更新的渲染实体 |
| `m_game_object_to_delete` | 待删除的渲染实体 |
| `m_camera_swap_data` | 相机参数更新（FOV、view 矩阵、类型） |
| `m_particle_submit_request` | 新粒子发射器 |
| `m_emitter_tick_request` | 需要更新的粒子发射器 ID |
| `m_emitter_transform_request` | 粒子发射器变换更新 |

**交换流程**：
```
逻辑侧写入 m_swap_data[LogicSwapDataType]
  ↓
swapLogicRenderData() → 交换索引
  ↓
渲染侧从 m_swap_data[RenderSwapDataType] 读取
```

#### 9.3 RenderScene — 渲染场景

**职责**：存储渲染侧的场景数据，管理可见性列表。

**关键成员**：
| 成员 | 用途 |
|------|------|
| `m_ambient_light` | 环境光 |
| `m_directional_light` | 方向光 |
| `m_point_light_list` | 点光源列表 |
| `m_render_entities` | 所有渲染实体 |
| `m_render_axis` | 编辑器 Gizmo 轴 |
| `m_directional_light_visible_mesh_nodes` | 方向光 Pass 可见网格 |
| `m_point_lights_visible_mesh_nodes` | 点光源 Pass 可见网格 |
| `m_main_camera_visible_mesh_nodes` | 主相机 Pass 可见网格 |

**关键方法**：
- `updateVisibleObjects(resource, camera)` — 每帧执行可见性剔除，填充各 Pass 的可见列表
- `setVisibleNodesReference()` — 设置可见节点指针（供 RenderPass 使用）

#### 9.4 RenderCamera — 渲染相机

**职责**：管理 view/projection 矩阵，支持多种相机模式。

**关键属性**：
- `m_znear / m_zfar` — 远近裁剪面
- 相机类型：透视/正交
- `lookAt(position, target, up)` — 设置相机朝向
- `setFOVx()` / `setAspect()` — 设置视场角和宽高比

#### 9.5 RenderResource / RenderResourceBase

**职责**：管理 GPU 资源的上传和管理。

**关键方法**：
- `uploadGlobalRenderResource(rhi, level_desc)` — 上传 IBL 贴图、颜色分级贴图
- `uploadGameObjectRenderResource(rhi, entity, mesh_data)` — 上传网格数据到 GPU
- `uploadGameObjectRenderResource(rhi, entity, material_data)` — 上传材质数据到 GPU
- `loadMeshData(source, out_bbox)` — 加载网格数据
- `loadMaterialData(source)` — 加载材质数据
- `updatePerFrameBuffer(scene, camera)` — 更新每帧 Uniform Buffer

### 在引擎时序中的位置

```
tickOneFrame(dt)
  ├─ logicalTick(dt)
  │    └─ 各组件 tick → 向 RenderSwapContext 写入数据
  │
  ├─ swapLogicRenderData()    ← ★ 数据交换点
  │
  └─ rendererTick(dt)
       └─ RenderSystem.tick(dt)    ← ★ 本章核心流程
            ├─ processSwapData()
            ├─ prepareContext()
            ├─ updatePerFrameBuffer()
            ├─ updateVisibleObjects()
            ├─ preparePassData()
            └─ deferredRender() / forwardRender()
```

### 课后思考题

1. 为什么需要 `RenderSwapContext` 双缓冲？如果逻辑线程和渲染线程直接共享 `RenderScene` 会有什么问题？
2. `RenderSystem.tick()` 中 `processSwapData()` 为什么放在第一步？如果放在渲染之后会怎样？
3. `updateVisibleObjects()` 的可见性剔除是如何实现的？（提示：阅读 `RenderScene::updateVisibleObjects` 的签名，猜测可能的实现方式）
4. `RenderResource` 和 `RenderScene` 的职责如何划分？为什么网格数据在 `RenderResource` 中管理，而实体列表在 `RenderScene` 中？
5. `RENDER_PIPELINE_TYPE` 支持延迟和前向两种管线——它们的主要区别是什么？各自适合什么场景？

### 后续精读标记

- 【跳过】`processSwapData()` 的完整实现（已在 render_system.cpp 中展示骨架）
- 【跳过】`updateVisibleObjects()` 的剔除算法（AABB 测试、frustum culling 等）
- 【跳过】`RenderResource` 的 GPU 资源上传细节（Vulkan buffer/image 创建）
- 【跳过】`GuidAllocator` 的 GUID 分配策略
- 【跳过】`RenderEntity` 和 `GameObjectDesc` 的完整字段

---

## 第10章 渲染管线与 RenderPass

### 学习目标

读完本章需要理解：
- `RenderPipeline` 和 `RenderPipelineBase` 的继承关系
- 10 个 RenderPass 各自的职责和执行顺序
- 延迟渲染管线（Deferred Pipeline）的完整流程
- Pass 之间的数据流转（Framebuffer、Descriptor Set）

### 需要阅读的文件列表

- `engine/source/runtime/function/render/render_pipeline_base.h`
- `engine/source/runtime/function/render/render_pipeline.h`
- `engine/source/runtime/function/render/render_pipeline.cpp`
- `engine/source/runtime/function/render/render_pass.h`
- `engine/source/runtime/function/render/render_pass_base.h`
- `engine/source/runtime/function/render/passes/main_camera_pass.h`
- `engine/source/runtime/function/render/passes/directional_light_pass.h`
- `engine/source/runtime/function/render/passes/point_light_pass.h`
- `engine/source/runtime/function/render/passes/particle_pass.h`
- `engine/source/runtime/function/render/passes/pick_pass.h`
- `engine/source/runtime/function/render/passes/color_grading_pass.h`
- `engine/source/runtime/function/render/passes/fxaa_pass.h`
- `engine/source/runtime/function/render/passes/tone_mapping_pass.h`
- `engine/source/runtime/function/render/passes/ui_pass.h`
- `engine/source/runtime/function/render/passes/combine_ui_pass.h`

### 核心骨架知识点

#### 10.1 RenderPipelineBase — 管线基类

**职责**：持有所有 RenderPass 的 `shared_ptr`，定义 forward/deferred render 接口。

**持有的 10 个 Pass**：
```cpp
std::shared_ptr<RenderPassBase> m_directional_light_pass;
std::shared_ptr<RenderPassBase> m_point_light_shadow_pass;
std::shared_ptr<RenderPassBase> m_main_camera_pass;
std::shared_ptr<RenderPassBase> m_color_grading_pass;
std::shared_ptr<RenderPassBase> m_fxaa_pass;
std::shared_ptr<RenderPassBase> m_tone_mapping_pass;
std::shared_ptr<RenderPassBase> m_ui_pass;
std::shared_ptr<RenderPassBase> m_combine_ui_pass;
std::shared_ptr<RenderPassBase> m_pick_pass;
std::shared_ptr<RenderPassBase> m_particle_pass;
```

#### 10.2 RenderPassBase — Pass 基类

**职责**：所有渲染 Pass 的抽象基类。

**关键方法**：
- `initialize(init_info)` — 创建 framebuffer、pipeline、descriptor set 等
- `setCommonInfo(common_info)` — 设置 RHI 和 RenderResource 引用
- `preparePassData(render_resource)` — 每帧准备 Pass 数据
- `initializeUIRenderBackend(window_ui)` — 初始化 UI 渲染后端

#### 10.3 延迟渲染管线执行顺序

```
RenderPipeline.deferredRender(rhi, render_resource)
  │
  │  ═══ 几何/阴影阶段 ═══
  │
  ├─ ① DirectionalLightPass        — 方向光阴影图渲染
  │     输入：场景深度
  │     输出：方向光阴影贴图
  │
  ├─ ② PointLightShadowPass        — 点光源阴影图渲染（6 面 cube map）
  │     输入：场景深度
  │     输出：点光源阴影 cube map
  │
  │  ═══ 主渲染阶段 ═══
  │
  ├─ ③ MainCameraPass              — 主相机 G-Buffer 渲染
  │     输入：可见网格、材质、光照、阴影贴图
  │     输出：G-Buffer（位置、法线、颜色、MR 等）
  │     这是最核心的 Pass
  │
  ├─ ④ ParticlePass                — 粒子渲染
  │     输入：粒子数据、深度缓冲
  │     输出：粒子画面
  │
  │  ═══ 后处理阶段 ═══
  │
  ├─ ⑤ PickPass                    — 物体拾取（编辑器用）
  │     输入：鼠标 UV 坐标
  │     输出：被拾取物体的 GUID
  │
  ├─ ⑥ ColorGradingPass            — 颜色分级
  │     输入：场景颜色 + 颜色分级 LUT
  │     输出：调色后的画面
  │
  ├─ ⑦ FxaaPass                    — 快速近似抗锯齿
  │     输入：上一步的颜色缓冲
  │     输出：抗锯齿后的画面
  │
  ├─ ⑧ ToneMappingPass             — 色调映射（HDR → LDR）
  │     输入：HDR 颜色缓冲
  │     输出：LDR 颜色缓冲
  │
  │  ═══ UI 阶段 ═══
  │
  ├─ ⑨ UiPass                      — ImGui UI 渲染
  │     输入：ImGui draw data
  │     输出：UI 图层
  │
  └─ ⑩ CombineUiPass               — 合并 UI 到最终画面
        输入：场景画面 + UI 图层
        输出：最终帧 → Present 到交换链
```

#### 10.4 前向渲染管线

```
RenderPipeline.forwardRender(rhi, render_resource)
  — 执行顺序与延迟管线不同
  — MainCameraPass 直接输出颜色（不写 G-Buffer）
  — 适用于透明物体较多的场景
```

#### 10.5 各 Pass 职责速查表

| Pass | 阶段 | 输入 | 输出 | 备注 |
|------|------|------|------|------|
| DirectionalLightPass | 阴影 | 深度 | 阴影图 | 方向光 |
| PointLightShadowPass | 阴影 | 深度 | 阴影 cube map | 点光源 |
| MainCameraPass | 几何/光照 | 网格+材质+阴影 | G-Buffer / 颜色 | 核心 Pass |
| ParticlePass | 特效 | 粒子+深度 | 粒子画面 | GPU 粒子 |
| PickPass | 交互 | 颜色+ID | 拾取结果 | 编辑器用 |
| ColorGradingPass | 后处理 | 颜色+LUT | 调色画面 | |
| FxaaPass | 后处理 | 颜色 | 抗锯齿画面 | |
| ToneMappingPass | 后处理 | HDR 颜色 | LDR 颜色 | |
| UiPass | UI | ImGui 数据 | UI 图层 | |
| CombineUiPass | 合成 | 场景+UI | 最终画面 | Present |

### 在引擎时序中的位置

```
RenderSystem.tick(dt)
  ├─ ... (前几步)
  └─ deferredRender() / forwardRender()    ← ★ 本章内容
       └─ 依次执行 10 个 RenderPass
```

### 课后思考题

1. 延迟渲染和直接渲染（前向渲染）的主要区别是什么？延迟渲染的 G-Buffer 包含哪些信息？
2. 为什么阴影 Pass（DirectionalLightPass、PointLightShadowPass）要在 MainCameraPass 之前执行？
3. PickPass 如何实现"根据鼠标位置拾取物体"？（提示：每个像素存储物体 ID）
4. 为什么 ToneMapping 在 Fxaa 之后？如果调换顺序会怎样？
5. CombineUiPass 的作用是什么？为什么不直接在 UiPass 中输出到交换链？

### 后续精读标记

- 【跳过】每个 Pass 的 `initialize()` 中 Vulkan 资源创建细节
- 【跳过】G-Buffer 的 MRT（Multiple Render Targets）配置
- 【跳过】各 Pass 的 Descriptor Set 布局和绑定
- 【跳过】阴影 map 的级联（Cascaded Shadow Map）实现
- 【跳过】FxaaPass 的着色器实现
- 【跳过】PickPass 的颜色编码/解码逻辑

---

## 第11章 RHI 抽象层与 Vulkan 后端

### 学习目标

读完本章需要理解：
- RHI（Rendering Hardware Interface）的设计目的
- RHI 抽象了哪些 Vulkan 概念
- `rhi_struct.h` 中的类型映射关系
- VulkanRHI 的大致初始化流程

### 需要阅读的文件列表

- `engine/source/runtime/function/render/interface/rhi.h`
- `engine/source/runtime/function/render/interface/rhi_struct.h`
- `engine/source/runtime/function/render/interface/vulkan/vulkan_rhi.h`
- `engine/source/runtime/function/render/interface/vulkan/vulkan_rhi.cpp`（仅看 initialize 和类结构）
- `engine/source/runtime/function/render/interface/vulkan/vulkan_rhi_resource.h`
- `engine/source/runtime/function/render/interface/vulkan/vulkan_util.h`

### 核心骨架知识点

#### 11.1 RHI 设计目的

**RHI（Rendering Hardware Interface）** 是一个纯虚接口类，抽象了底层图形 API 的所有操作。当前只有 Vulkan 后端（`VulkanRHI`），但接口设计允许未来扩展到其他 API（如 DirectX、Metal）。

#### 11.2 RHI 接口分类

| 类别 | 方法示例 | 说明 |
|------|---------|------|
| **初始化** | `initialize()`, `prepareContext()` | 设备创建、上下文准备 |
| **资源创建** | `createBuffer()`, `createImage()`, `createShaderModule()` | 创建 GPU 资源 |
| **管线创建** | `createGraphicsPipelines()`, `createComputePipelines()` | 创建渲染/计算管线 |
| **描述符** | `createDescriptorSetLayout()`, `allocateDescriptorSets()`, `updateDescriptorSets()` | 描述符管理 |
| **渲染Pass** | `createRenderPass()`, `createFramebuffer()` | 帧缓冲和渲染Pass |
| **命令录制** | `beginCommandBuffer()`, `cmdDraw()`, `cmdDrawIndexed()`, `cmdDispatch()` | 录制 GPU 命令 |
| **提交** | `queueSubmit()`, `submitRendering()` | 提交命令到 GPU 队列 |
| **同步** | `createFence()`, `createSemaphore()`, `waitForFences()` | GPU 同步原语 |
| **交换链** | `createSwapchain()`, `recreateSwapchain()` | 窗口交换链管理 |
| **内存** | `mapMemory()`, `unmapMemory()`, `freeMemory()` | CPU↔GPU 内存操作 |
| **销毁** | `destroyBuffer()`, `destroyImage()`, `clear()` | 资源释放 |
| **查询** | `getGraphicsQueue()`, `getComputeQueue()`, `getSwapchainInfo()` | 获取设备信息 |

#### 11.3 rhi_struct.h — 类型映射

`rhi_struct.h` 定义了 RHI 层的抽象类型，在 Vulkan 后端中映射到具体的 Vulkan 类型：

| RHI 类型 | Vulkan 对应 |
|----------|------------|
| `RHIBuffer` | `VkBuffer` |
| `RHIImage` | `VkImage` |
| `RHISampler` | `VkSampler` |
| `RHICommandBuffer` | `VkCommandBuffer` |
| `RHICommandPool` | `VkCommandPool` |
| `RHIDescriptorSet` | `VkDescriptorSet` |
| `RHIDescriptorSetLayout` | `VkDescriptorSetLayout` |
| `RHIFence` | `VkFence` |
| `RHISemaphore` | `VkSemaphore` |
| `RHIFramebuffer` | `VkFramebuffer` |
| `RHIPipeline` | `VkPipeline` |
| `RHIRenderPass` | `VkRenderPass` |
| `RHIQueue` | `VkQueue` |
| `RHISwapChainDesc` | 交换链描述信息 |

#### 11.4 VulkanRHI 初始化流程（骨架）

```
VulkanRHI.initialize(info)
  ├─ 创建 VkInstance
  ├─ 创建 VkSurfaceKHR（通过 GLFW）
  ├─ 选择物理设备（GPU）
  ├─ 创建 VkDevice（逻辑设备）
  ├─ 获取 Queue（Graphics + Compute）
  ├─ 创建 CommandPool
  ├─ 创建 DescriptorPool
  ├─ 创建 Swapchain + ImageViews
  ├─ 创建 Depth Image
  ├─ 创建 Sync Objects（Fences + Semaphores）
  └─ 初始化 VMA（Vulkan Memory Allocator）
```

#### 11.5 帧渲染流程（Vulkan 侧）

```
RHI.prepareContext()
  ├─ waitForFences() — 等待上一帧完成
  ├─ resetFences() — 重置 Fence
  └─ resetCommandPool() — 重置命令池

RHI.submitRendering()
  ├─ endCommandBuffer() — 结束命令录制
  ├─ queueSubmit() — 提交到 GPU
  └─ Present → 交换链展示
```

### 在引擎时序中的位置

```
RenderSystem.tick(dt)
  ├─ m_rhi->prepareContext()         ← 帧开始准备
  ├─ ... (更新资源、可见性)
  └─ deferredRender()
       └─ 各 RenderPass 通过 RHI 接口录制命令
            └─ RHI.submitRendering() ← 帧结束提交
```

### 课后思考题

1. 为什么要抽象 RHI 层而不是直接使用 Vulkan API？这样做的代价是什么？
2. `rhi_struct.h` 中的类型在实际使用时是如何从 `void*` 或 `uint64_t` 转换回 Vulkan 类型的？
3. VulkanRHI 中有 `getGraphicsQueue()` 和 `getComputeQueue()`——为什么需要两种队列？粒子系统的 Compute Shader 用哪个？
4. `prepareContext()` 中为什么要先 `waitForFences()` 再 `resetCommandPool()`？顺序能调换吗？
5. VMA（Vulkan Memory Allocator）的作用是什么？为什么不直接用 `vkAllocateMemory()`？

### 后续精读标记

- 【跳过】`VulkanRHI::initialize()` 的完整实现（约 500+ 行）
- 【跳过】物理设备选择逻辑（GPU 评分）
- 【跳过】Swapchain 格式选择和创建细节
- 【跳过】VMA 的内存类型选择策略
- 【跳过】`recreateSwapchain()` 的处理（窗口 resize 时）
- 【跳过】各 `cmd*` 方法的 Vulkan 调用细节

---

## 第12章 编辑器模块

### 学习目标

读完本章需要理解：
- `PiccoloEditor` 如何驱动编辑器主循环
- `EditorGlobalContext` 的角色
- `EditorUI` 的 ImGui 界面结构
- `EditorSceneManager` 的 Gizmo 操作原理
- 编辑器输入与游戏输入的区别

### 需要阅读的文件列表

- `engine/source/editor/source/main.cpp`
- `engine/source/editor/include/editor.h`
- `engine/source/editor/source/editor.cpp`
- `engine/source/editor/include/editor_global_context.h`
- `engine/source/editor/source/editor_global_context.cpp`
- `engine/source/editor/include/editor_ui.h`
- `engine/source/editor/source/editor_ui.cpp`
- `engine/source/editor/include/editor_scene_manager.h`
- `engine/source/editor/source/editor_scene_manager.cpp`
- `engine/source/editor/include/editor_input_manager.h`
- `engine/source/editor/source/editor_input_manager.cpp`
- `engine/source/editor/include/editor_file_service.h`
- `engine/source/editor/source/editor_file_service.cpp`
- `engine/source/editor/include/axis.h`

### 核心骨架知识点

#### 12.1 PiccoloEditor — 编辑器顶层

**职责**：初始化编辑器环境，驱动编辑器主循环。

**关键成员**：
- `m_editor_ui` — `shared_ptr<EditorUI>`
- `m_engine_runtime` — `PiccoloEngine*`

**编辑器主循环**（`editor.cpp:51`）：
```
PiccoloEditor::run()
  while (true)
    ├─ delta_time = engine->calculateDeltaTime()
    ├─ EditorSceneManager.tick(dt)     ← Gizmo 操作、相机控制
    ├─ EditorInputManager.tick(dt)     ← 编辑器输入处理
    └─ engine->tickOneFrame(dt)        ← 引擎一帧（逻辑+渲染）
         │                              如果返回 false → 退出循环
```

**与纯游戏模式的区别**：编辑器在主循环中额外执行 `EditorSceneManager` 和 `EditorInputManager` 的 tick，然后再调用引擎的 `tickOneFrame`。

#### 12.2 EditorGlobalContext — 编辑器全局上下文

**职责**：持有编辑器专属的系统指针。

**成员**：
| 成员 | 类型 | 职责 |
|------|------|------|
| `m_scene_manager` | `EditorSceneManager*` | 场景编辑操作 |
| `m_input_manager` | `EditorInputManager*` | 编辑器输入 |
| `m_render_system` | `RenderSystem*` | 渲染系统引用 |
| `m_window_system` | `WindowSystem*` | 窗口系统引用 |
| `m_engine` | `PiccoloEngine*` | 引擎引用 |

**初始化**：
```
EditorGlobalContext.initialize(init_info)
  ├─ 创建 EditorSceneManager
  ├─ 创建 EditorInputManager
  └─ 保存 RenderSystem、WindowSystem、Engine 引用
```

#### 12.3 EditorUI — ImGui 界面

**职责**：基于 ImGui 的编辑器用户界面。

**继承**：`EditorUI` extends `WindowUI`

**界面组成**（典型的编辑器布局）：
- 菜单栏
- 世界对象列表窗口（World Objects）
- 文件浏览器窗口（File Content）
- 游戏视口窗口（Game Window）
- 属性/详情面板（Detail/Inspector）

**UI 创建注册**：内部维护一个 UI-creator 注册表（`map`），每个子窗口对应一个创建函数。

#### 12.4 EditorSceneManager — 场景管理器

**职责**：管理编辑器中的场景操作——选择物体、平移/旋转/缩放 Gizmo、相机控制。

**关键功能**：
- **物体选择**：通过 `RenderSystem.getGuidOfPickedMesh(uv)` 实现点击拾取
- **Gizmo 操作**：三种模式
  - `EditorAxisMode::Translation` — 平移（T 键）
  - `EditorAxisMode::Rotation` — 旋转（R 键）
  - `EditorAxisMode::Scale` — 缩放（C 键）
- **相机控制**：编辑器自由相机（WASD + 鼠标）

**Gizmo 轴**：
- `EditorTranslationAxis` / `EditorRotationAxis` / `EditorScaleAxis`
- 继承自 `RenderEntity`，带有自定义的 `RenderMeshData`
- 通过 `RenderSystem.createAxis()` 上传到 GPU

#### 12.5 EditorInputManager — 编辑器输入

**职责**：处理编辑器专属的键鼠输入。

**EditorCommand 位标志**：
| 命令 | 按键 | 功能 |
|------|------|------|
| 相机移动 | WASD | 编辑器相机平移 |
| 模式切换 | T/R/C | 平移/旋转/缩放 |
| 删除 | Delete | 删除选中物体 |
| 鼠标 | 光标移动 | 相机旋转 |
| 滚轮 | 鼠标滚轮 | 相机缩放 |

#### 12.6 EditorFileService — 文件浏览器

**职责**：构建资产目录树，供编辑器 UI 展示。

**关键类**：
- `EditorFileNode` — 树节点（名称、类型、路径、深度、子节点列表）
- `EditorFileService` — 扫描 `engine/asset/` 目录，构建文件树

### 在引擎时序中的位置

```
PiccoloEditor::run() 主循环
  │
  ├─ EditorSceneManager.tick(dt)     ← Gizmo/相机更新
  ├─ EditorInputManager.tick(dt)     ← 编辑器输入
  │
  └─ engine->tickOneFrame(dt)        ← 引擎标准一帧
       ├─ logicalTick(dt)
       │    ├─ WorldManager.tick(dt)
       │    │    └─ Level.tick(dt)
       │    │         └─ GObject.tick(dt) → Component.tick(dt)
       │    │              （编辑器模式下只 tick Transform + Mesh）
       │    └─ InputSystem.tick()
       ├─ swapLogicRenderData()
       ├─ rendererTick(dt)
       ├─ pollEvents()
       └─ setTitle()
```

### 课后思考题

1. 编辑器主循环比纯游戏模式多了哪两步？为什么需要这两步？
2. `EditorSceneManager` 的物体拾取是如何工作的？从鼠标点击到物体被选中，数据经过了哪些模块？
3. 编辑器模式下只有 `TransformComponent` 和 `MeshComponent` 会 tick——如果不这样做，编辑器中会发生什么奇怪的现象？
4. `EditorUI` 继承自 `WindowUI`——这个继承关系的作用是什么？`RenderSystem` 如何利用 `WindowUI` 接口？
5. Gizmo 轴（`EditorTranslationAxis` 等）是如何被渲染到画面上的？它们和普通游戏物体的渲染路径有什么区别？

### 后续精读标记

- 【跳过】`EditorUI` 的各个子窗口绘制函数
- 【跳过】`EditorSceneManager` 的 Gizmo 拖拽交互逻辑（射线与轴碰撞检测）
- 【跳过】`EditorInputManager` 的相机运动计算
- 【跳过】`EditorFileService` 的目录扫描实现
- 【跳过】ImGui 的初始化和渲染集成

---

## 附录A 完整初始化顺序 + 一帧 Tick 流程图

### A.1 完整初始化顺序

```
main()
  │
  ├─ new PiccoloEngine()
  │
  ├─ engine->startEngine(config_path)
  │    ├─ TypeMetaRegister::Register()               ← 注册反射类型
  │    └─ g_runtime_global_context.startSystems()
  │         ├─ (1)  ConfigManager::initialize()       解析 INI 配置
  │         ├─ (2)  FileSystem                        文件服务
  │         ├─ (3)  LogSystem                         日志
  │         ├─ (4)  AssetManager                      资产 I/O
  │         ├─ (5)  PhysicsManager::initialize()      Jolt 物理初始化
  │         ├─ (6)  WorldManager::initialize()        世界管理
  │         ├─ (7)  WindowSystem::initialize()        GLFW 窗口
  │         ├─ (8)  InputSystem::initialize()         输入
  │         ├─ (9)  ParticleManager::initialize()     粒子
  │         ├─ (10) RenderSystem::initialize()        Vulkan 渲染
  │         │        ├─ VulkanRHI.initialize()
  │         │        ├─ 加载全局渲染资源
  │         │        ├─ 创建 RenderResource
  │         │        ├─ 创建 RenderCamera
  │         │        ├─ 创建 RenderScene
  │         │        └─ 创建 RenderPipeline → 10 个 Pass
  │         ├─ (11) DebugDrawManager::initialize()    调试绘制
  │         └─ (12) RenderDebugConfig                 调试配置
  │
  ├─ engine->initialize()                             当前为空
  │
  ├─ new PiccoloEditor()
  │    └─ 注册编辑器 tick 组件（Transform + Mesh）
  │
  └─ editor->initialize(engine)
       ├─ g_is_editor_mode = true
       ├─ EditorGlobalContext.initialize()
       │    ├─ 创建 EditorSceneManager
       │    └─ 创建 EditorInputManager
       ├─ SceneManager.setEditorCamera()
       ├─ SceneManager.uploadAxisResource()
       └─ 创建 EditorUI → initialize()
```

### A.2 一帧 Tick 完整流程

```
┌─────────────────────── 编辑器主循环 ───────────────────────┐
│                                                             │
│  ① calculateDeltaTime()                                     │
│  │   steady_clock → delta_time                              │
│  │                                                          │
│  ② EditorSceneManager.tick(dt)                              │
│  │   ├─ 更新 Gizmo 状态                                     │
│  │   └─ 更新编辑器相机                                      │
│  │                                                          │
│  ③ EditorInputManager.tick(dt)                              │
│  │   └─ 处理编辑器键鼠输入                                  │
│  │                                                          │
│  ④ engine->tickOneFrame(dt)                                 │
│  │   │                                                      │
│  │   ├─ ④-a logicalTick(dt)                                 │
│  │   │   ├─ WorldManager.tick(dt)                           │
│  │   │   │   ├─ [首帧] loadWorld() → loadLevel()            │
│  │   │   │   └─ Level.tick(dt)                              │
│  │   │   │       ├─ 遍历 GObject.tick(dt)                   │
│  │   │   │       │   └─ 遍历 Component.tick(dt)             │
│  │   │   │       │       ├─ TransformComponent              │
│  │   │   │       │       ├─ MeshComponent → 提交渲染数据    │
│  │   │   │       │       ├─ RigidBodyComponent              │
│  │   │   │       │       ├─ MotorComponent                  │
│  │   │   │       │       ├─ AnimationComponent              │
│  │   │   │       │       └─ ParticleComponent               │
│  │   │   │       ├─ [非编辑器] Character.tick(dt)           │
│  │   │   │       └─ PhysicsScene.tick(dt)                   │
│  │   │   └─ InputSystem.tick()                              │
│  │   │                                                      │
│  │   ├─ ④-b calculateFPS(dt)                                │
│  │   │                                                      │
│  │   ├─ ④-c swapLogicRenderData()                           │
│  │   │   逻辑数据 ←→ 渲染数据 交换                          │
│  │   │                                                      │
│  │   ├─ ④-d rendererTick(dt)                                │
│  │   │   └─ RenderSystem.tick(dt)                           │
│  │   │       ├─ processSwapData()                           │
│  │   │       ├─ rhi->prepareContext()                       │
│  │   │       ├─ updatePerFrameBuffer()                      │
│  │   │       ├─ updateVisibleObjects()                      │
│  │   │       ├─ preparePassData()                           │
│  │   │       ├─ DebugDrawManager.tick()                     │
│  │   │       └─ deferredRender()                            │
│  │   │           ├─ DirectionalLightPass                    │
│  │   │           ├─ PointLightShadowPass                    │
│  │   │           ├─ MainCameraPass                          │
│  │   │           ├─ ParticlePass                            │
│  │   │           ├─ PickPass                                │
│  │   │           ├─ ColorGradingPass                        │
│  │   │           ├─ FxaaPass                                │
│  │   │           ├─ ToneMappingPass                         │
│  │   │           ├─ UiPass                                  │
│  │   │           └─ CombineUiPass → Present                 │
│  │   │                                                      │
│  │   ├─ ④-e [可选] PhysicsManager.renderPhysicsWorld()      │
│  │   │                                                      │
│  │   ├─ ④-f WindowSystem.pollEvents()                       │
│  │   │                                                      │
│  │   └─ ④-g WindowSystem.setTitle("Piccolo - XX FPS")      │
│  │                                                          │
│  ⑤ 检查 shouldClose → 如果 true 则退出循环                 │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## 附录B 重要 TODO 精读点

以下是后续需要深入阅读的函数/类，按模块分类：

### B.1 游戏框架
| 位置 | 内容 | 关注点 |
|------|------|--------|
| `TransformComponent::tick()` | 双缓冲变换矩阵 | 如何避免读写冲突 |
| `RigidBodyComponent::tick()` | 物理↔逻辑变换同步 | 运动学 vs 动力学刚体 |
| `MeshComponent::tick()` | 向 SwapContext 提交数据 | 渲染数据格式 |
| `MotorComponent::tick()` | 角色运动控制 | 跳跃/冲刺物理 |
| `CharacterController` | 胶囊体扫掠 | 碰撞检测算法 |

### B.2 反射与序列化
| 位置 | 内容 | 关注点 |
|------|------|--------|
| `meta_parser/parser/` | libclang AST 遍历 | Cursor 遍历策略 |
| `ReflectionGenerator` | 反射代码生成 | 生成的代码格式 |
| `SerializerGenerator` | 序列化代码生成 | 递归序列化逻辑 |
| `PSerializer` | 运行时序列化 | JSON ↔ 对象递归转换 |

### B.3 渲染
| 位置 | 内容 | 关注点 |
|------|------|--------|
| `RenderSystem::processSwapData()` | 交换数据处理 | 实体创建/删除流程 |
| `RenderScene::updateVisibleObjects()` | 可见性剔除 | Frustum Culling 算法 |
| `MainCameraPass::initialize()` | G-Buffer 配置 | MRT + Descriptor Set |
| `RenderPipeline::deferredRender()` | 延迟渲染完整流程 | Pass 间数据流转 |
| `VulkanRHI::initialize()` | Vulkan 初始化 | 设备/交换链/同步 |
| `RenderResource::uploadGameObjectRenderResource()` | GPU 资源上传 | Buffer/Image 创建 |

### B.4 物理
| 位置 | 内容 | 关注点 |
|------|------|--------|
| `PhysicsScene::tick()` | Jolt 步进 | 碰撞步长/子步 |
| `PhysicsScene::raycast/sweep()` | 物理查询 | 查询算法 |
| `RigidBodyComponent::postLoadResource()` | 刚体创建 | 碰撞形状配置 |

### B.5 动画
| 位置 | 内容 | 关注点 |
|------|------|--------|
| `Skeleton::applyAnimation()` | 动画应用 | 关键帧插值 |
| `Skeleton::outputAnimationResult()` | 矩阵计算 | 局部→世界变换 |
| `AnimationComponent::tick()` | 动画状态机 | 混合权重计算 |

### B.6 编辑器
| 位置 | 内容 | 关注点 |
|------|------|--------|
| `EditorSceneManager` | Gizmo 交互 | 射线-轴碰撞检测 |
| `EditorUI` | ImGui 界面 | 各面板布局/交互 |
| `EditorInputManager` | 编辑器相机 | 相机运动学 |

---

## 附录C 高频面试题清单

基于 Piccolo 引擎架构的高频面试题，按主题分类：

### C.1 引擎架构

1. **描述 Piccolo 引擎的子系统初始化顺序，为什么必须按这个顺序？**
   > 考察对依赖关系的理解：ConfigManager 最先（提供路径），RenderSystem 在 WindowSystem 之后（需要窗口句柄）等。

2. **RuntimeGlobalContext 持有所有子系统的 shared_ptr——这种设计的优缺点是什么？与 Service Locator 模式有什么区别？**
   > 优点：简单直接、生命周期清晰。缺点：全局状态、难以测试。对比 Service Locator 的延迟查找。

3. **为什么编辑器模式和游戏模式使用不同的主循环驱动？`g_is_editor_mode` 全局变量影响了哪些行为？**
   > 编辑器需要额外的 SceneManager/InputManager tick；组件 tick 过滤；Character tick 跳过。

4. **`RenderSwapContext` 双缓冲解决了什么问题？如果不用双缓冲，逻辑线程和渲染线程共享数据会有哪些并发问题？**
   > 数据竞争、读写不一致。双缓冲通过交换索引避免锁。

### C.2 反射与序列化

5. **C++ 没有原生反射，Piccolo 是如何实现反射的？与 Unreal Engine 的 UHT（Unreal Header Tool）有什么异同？**
   > 编译期 libclang 解析 + Mustache 代码生成。类似 UHT，但 Piccolo 用 libclang 而非自定义解析器。

6. **`ReflectionPtr<T>` 为什么要同时保存类型名和指针？在序列化/反序列化中如何发挥作用？**
   > 多态序列化：JSON 中需要知道具体类型名才能正确重建对象。

7. **反射系统的 `FieldFunctionTuple` 存储了哪些函数？为什么用 `std::function` 而不是虚函数？**
   > Set/Get/GetName 等函数。用 `std::function` 可以存储自由函数/lambda，避免在每个类中添加虚函数。

### C.3 渲染

8. **描述 Piccolo 延迟渲染管线的完整流程。G-Buffer 包含哪些 MRT 附件？**
   > 阴影Pass → MainCameraPass(G-Buffer) → 光照计算 → 后处理 → UI → Present。

9. **RHI 抽象层的设计模式是什么？如果要将 Vulkan 后端替换为 DirectX 12，需要修改哪些地方？**
   > 策略模式/桥接模式。需要实现新的 DX12RHI 类，实现 RHI 所有虚函数。

10. **`RenderScene::updateVisibleObjects()` 可能使用了什么剔除算法？如何优化大规模场景的渲染？**
    > Frustum Culling、Occlusion Culling、空间划分（BVH/Octree）。

### C.4 物理

11. **为什么每个 Level 有独立的 PhysicsScene？全局共享一个 PhysicsSystem 有什么问题？**
    > 不同关卡可能有不同重力；关卡卸载时需要清理物理世界；隔离性。

12. **`RigidBodyComponent` 如何与 `PhysicsScene` 交互？为什么用 body_id 而不是直接持有 Jolt 的 Body 指针？**
    > 通过 ID 间接引用，PhysicsScene 管理 Body 生命周期，避免悬空指针。

### C.5 游戏框架

13. **GObject + Component 架构与纯 ECS 有什么区别？Piccolo 的选择有什么优缺点？**
    > Piccolo 是"组件组合"模式（GObject 持有 Component 数组），不是纯 ECS（没有 System 遍历所有匹配 Entity）。优点：简单直观。缺点：不利于数据导向优化。

14. **编辑器模式下只让 TransformComponent 和 MeshComponent tick——如果不这样做会出什么问题？**
    > MotorComponent 会响应输入移动角色；PhysicsComponent 会驱动物理模拟——在编辑器中不希望这些自动发生。

15. **Level.tick() 中物理步进在 GObject tick 之后——这对 RigidBodyComponent 的同步策略有什么影响？**
    > 本帧组件先更新位置→写入物理世界→物理模拟→下帧组件读取结果。存在一帧延迟。

### C.6 综合

16. **从鼠标点击一个物体到物体被渲染高亮，数据经过了哪些模块？**
    > GLFW 回调 → InputSystem → EditorInputManager → EditorSceneManager → RenderSystem.getGuidOfPickedMesh() → PickPass → 返回 GUID → SceneManager 标记选中 → 下一帧渲染。

17. **从 JSON 文件加载一个游戏对象到屏幕上显示它，数据经过了哪些模块？**
    > AssetManager.loadAsset() → PSerializer → Reflection → GObject.load() → Component.postLoadResource() → MeshComponent 提交到 SwapContext → RenderSystem.processSwapData() → RenderResource.upload() → RenderPipeline 渲染。

18. **如果要在 Piccolo 中添加一个"音频系统"，你需要修改哪些文件？描述你的集成方案。**
    > 在 `RuntimeGlobalContext` 中添加 `m_audio_system`；在 `startSystems()` 中按依赖顺序创建；在 `tickOneFrame()` 中添加 `audioTick()`；创建 `AudioManager` + `AudioComponent`；通过反射支持序列化。

---

> **学习建议**：完成本手册的学习后，建议按以下顺序精读代码：
> 1. 先精读第 2 章（游戏对象框架）——这是引擎的核心数据结构
> 2. 再精读第 9-10 章（渲染系统）——这是引擎最复杂的子系统
> 3. 然后精读第 3 章（反射系统）——这是引擎最独特的设计
> 4. 最后根据兴趣选择物理/动画/粒子深入
