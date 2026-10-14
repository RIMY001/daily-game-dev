# 游戏客户端开发专用设计模式学习手册

> **目标读者**：准备游戏客户端岗位、有 C++ 基础的学习者
> **核心原则**：所有设计模式，优先讲解在 Unity/UE/Piccolo 等游戏引擎、游戏客户端工程中的真实应用场景；弱化 Web 等其他领域的例子；重点突出源码里在哪能见到、面试怎么考、工程选型取舍。

**标记说明**：
- 🔧 = **引擎底层常见**（渲染、资源、场景管理）
- 🎮 = **业务层常见**（UI、逻辑、输入）
- ⚡ = **性能敏感**（影响帧率或内存）

---

# 第一部分：模式原理、结构与底层构成

---

# 创建型模式

---

## 1. 单例模式 Singleton 🎮⚡

### 1.1 结构可视化

```
  Singleton 核心意图：全局唯一实例 + 全局访问点

  UML 简图：
  ┌─────────────────────────────────┐
  │         GameManager             │
  ├─────────────────────────────────┤
  │ - instance: GameManager*        │  ◄── 静态指针，持有唯一实例
  │ - instance: GameManager (C++11) │  ◄── 或用静态局部变量
  ├─────────────────────────────────┤
  │ - GameManager()                 │  ◄── 私有构造函数
  │ + getInstance(): GameManager&   │  ◄── 静态公有方法
  │ + update(): void                │
  │ + pause(): void                 │
  └─────────────────────────────────┘

  调用关系：
  Player ──────► GameManager::getInstance().update()
  UIManager ───► GameManager::getInstance().pause()
  AudioManager ► GameManager::getInstance().getState()

  所有系统都通过同一个访问点获取实例
```

### 1.2 底层构成拆解

| C++ 特性 | 用途 |
|---------|------|
| 静态成员变量 | 存储唯一实例指针 |
| 私有构造函数 | 禁止外部 new |
| 静态方法 | 提供全局访问点 |
| 删除拷贝构造/赋值 | 禁止复制 |
| `static` 局部变量（Meyers 单例） | C++11 线程安全的懒加载 |
| `std::once_flag` / `std::call_once` | 多线程安全初始化 |

### 1.3 伪代码

```cpp
// ===== 方案一：Meyers 单例（推荐，C++11 线程安全）=====
class GameManager {
public:
    static GameManager& getInstance() {
        static GameManager instance;  // 局部静态变量，C++11 保证线程安全
        return instance;
    }

    void update(float deltaTime) { /* ... */ }
    void pause() { /* ... */ }

    // 禁止拷贝
    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;

private:
    GameManager() {}   // 私有构造
    ~GameManager() {}  // 私有析构
};

// 使用：
// GameManager::getInstance().update(dt);

// ===== 方案二：双重检查锁（懒加载 + 指针）=====
class ResourceManager {
    static ResourceManager* instance;
    static std::mutex mtx;

public:
    static ResourceManager* getInstance() {
        if (instance == nullptr) {              // 第一次检查（无锁）
            std::lock_guard<std::mutex> lock(mtx);
            if (instance == nullptr) {          // 第二次检查（有锁）
                instance = new ResourceManager();
            }
        }
        return instance;
    }

    // 禁止拷贝
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

private:
    ResourceManager() {}
};

// ===== 方案三：引擎风格（手动生命周期控制）=====
class AudioSystem {
    static AudioSystem* s_instance;

public:
    static void create() {
        assert(s_instance == nullptr);
        s_instance = new AudioSystem();
    }

    static void destroy() {
        assert(s_instance != nullptr);
        delete s_instance;
        s_instance = nullptr;
    }

    static AudioSystem& getInstance() {
        assert(s_instance != nullptr);
        return *s_instance;
    }

    void playSound(int soundId) { /* ... */ }

private:
    AudioSystem() {}
    ~AudioSystem() {}
};

// 引擎启动时：AudioSystem::create();
// 引擎关闭时：AudioSystem::destroy();
```

### 1.4 C++ 标准库对应

| 项 | 内容 |
|---|------|
| **头文件** | `<mutex>`（线程安全单例） |
| **std 组件** | `std::call_once` / `std::once_flag`（保证只执行一次） |
| **游戏注意事项** | |

- **Meyers 单例**（静态局部变量）是 C++11 后最推荐的写法，编译器保证线程安全。
- **引擎级单例**通常不用懒加载，而是在引擎启动时显式 `create()`，关闭时 `destroy()`，避免析构顺序问题。
- **Unity 中的单例**：`MonoBehaviour` 单例通常继承自基类 `Singleton<T>`，利用 `DontDestroyOnLoad` 保持跨场景。

---

## 2. 对象池 Object Pool ⚡🔧

### 2.1 结构可视化

```
  Object Pool 核心意图：预分配 + 复用，避免频繁 new/delete

  UML 简图：
  ┌─────────────────────────────────┐
  │        ObjectPool<T>            │
  ├─────────────────────────────────┤
  │ - pool: std::queue<T*>          │  ◄── 空闲对象队列
  │ - active: std::vector<T*>       │  ◄── 活跃对象列表
  │ - maxSize: int                  │
  ├─────────────────────────────────┤
  │ + acquire(): T*                 │  ◄── 从池中取出
  │ + release(T* obj): void         │  ◄── 归还到池中
  │ + prewarm(int count): void      │  ◄── 预热
  └─────────────────────────────────┘

  内存布局：
  ┌──────────────────────────────────────────────────┐
  │ 预分配的连续内存块（一次性分配）                      │
  │ ┌─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┐│
  │ │ obj │ obj │ obj │ obj │ obj │ obj │ obj │ obj ││
  │ │ [0] │ [1] │ [2] │ [3] │ [4] │ [5] │ [6] │ [7] ││
  │ └─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┘│
  └──────────────────────────────────────────────────┘

  使用状态：
  Active:  [0] [1] [2] [3]              （正在使用）
  Free:              [4] [5] [6] [7]    （在池中待用）

  acquire() → 从 Free 取一个 → Active
  release() → 归还到 Free

  ──────────────────────────────────────────────────────

  游戏场景：子弹对象池

  初始预热：创建 100 个 Bullet 对象
  ┌────┬────┬────┬────┬────┬────┬────┬────┬────┬────┐
  │ B0 │ B1 │ B2 │ B3 │ B4 │ B5 │ B6 │ B7 │ B8 │ B9 │
  └────┴────┴────┴────┴────┴────┴────┴────┴────┴────┘
   空闲  空闲 空闲 空闲 空闲 空闲 空闲 空闲 空闲 空闲

  玩家射击：acquire() 3 次
  ┌────┬────┬────┬────┬────┬────┬────┬────┬────┬────┐
  │ B0 │ B1 │ B2 │ B3 │ B4 │ B5 │ B6 │ B7 │ B8 │ B9 │
  └────┴────┴────┴────┴────┴────┴────┴────┴────┴────┘
  活跃  活跃 活跃 空闲 空闲 空闲 空闲 空闲 空闲 空闲

  子弹命中：release(B0), release(B1)
  ┌────┬────┬────┬────┬────┬────┬────┬────┬────┬────┐
  │ B0 │ B1 │ B2 │ B3 │ B4 │ B5 │ B6 │ B7 │ B8 │ B9 │
  └────┴────┴────┴────┴────┴────┴────┴────┴────┴────┘
  空闲  空闲 活跃 空闲 空闲 空闲 空闲 空闲 空闲 空闲
```

### 2.2 底层构成拆解

| C++ 特性 | 用途 |
|---------|------|
| 模板 `template<T>` | 泛型对象池，支持任意类型 |
| 容器（queue/vector） | 存储空闲/活跃对象 |
| 连续内存分配 | 预分配一块连续内存，避免碎片 |
| 重置接口 `reset()` | 归还时清理状态 |

### 2.3 伪代码

```cpp
template <typename T>
class ObjectPool {
    std::vector<T*> pool;       // 空闲对象
    std::vector<T*> active;     // 活跃对象
    size_t maxSize;

public:
    ObjectPool(size_t poolSize) : maxSize(poolSize) {
        prewarm(poolSize);
    }

    ~ObjectPool() {
        for (T* obj : pool)   delete obj;
        for (T* obj : active) delete obj;
    }

    // 预热：预先创建所有对象
    void prewarm(size_t count) {
        for (size_t i = 0; i < count; ++i) {
            pool.push_back(new T());
        }
    }

    // 从池中取出
    T* acquire() {
        T* obj;
        if (pool.empty()) {
            // 池空：可以选择扩容或返回 nullptr
            obj = new T();  // 扩容
        } else {
            obj = pool.back();
            pool.pop_back();
        }
        obj->reset();       // 重置状态
        obj->onAcquire();   // 激活回调
        active.push_back(obj);
        return obj;
    }

    // 归还到池中
    void release(T* obj) {
        if (!obj) return;
        obj->onRelease();   // 释放回调

        // 从 active 移除（swap-and-pop，O(1)）
        auto it = std::find(active.begin(), active.end(), obj);
        if (it != active.end()) {
            *it = active.back();
            active.pop_back();
        }

        pool.push_back(obj);
    }

    size_t activeCount() const { return active.size(); }
    size_t freeCount() const { return pool.size(); }
};

// ===== 游戏对象基类 =====
class GameObject {
public:
    virtual void reset() { /* 重置位置、状态等 */ }
    virtual void onAcquire() { /* 激活时调用 */ }
    virtual void onRelease() { /* 归还时调用 */ }
    virtual void update(float dt) = 0;
};

// ===== 子弹类 =====
class Bullet : public GameObject {
    Vec3 position;
    Vec3 velocity;
    bool isActive;

public:
    void reset() override {
        position = Vec3(0, 0, 0);
        velocity = Vec3(0, 0, 0);
        isActive = false;
    }

    void onAcquire() override {
        isActive = true;
    }

    void onRelease() override {
        isActive = false;
    }

    void update(float dt) override {
        if (!isActive) return;
        position += velocity * dt;
    }
};

// ===== 使用 =====
ObjectPool<Bullet> bulletPool(100);  // 预热 100 发子弹

// 射击时：
Bullet* b = bulletPool.acquire();
b->position = player.pos;
b->velocity = player.forward * 50.0f;

// 命中时：
bulletPool.release(bullet);
```

### 2.4 C++ 标准库对应

| 项 | 内容 |
|---|------|
| **头文件** | `<vector>`, `<queue>`, `<memory>` |
| **游戏注意事项** | |

- **Unity 的对象池**：`UnityEngine.Pool.ObjectPool<T>`，内置 `Get()` / `Release()`。
- **UE 的对象池**：`TObjectPool<T>`，或用 `TArray` 手动实现。
- **Piccolo（Games104）**：实体组件系统中，Entity 的分配通常用对象池或 free-list。
- **性能关键**：对象池的 `acquire/release` 必须 O(1)，避免 `std::find` 的 O(n) 查找（可以用索引映射优化）。

---

## 3. 工厂方法 Factory Method 🎮🔧

### 3.1 结构可视化

```
  Factory Method 核心意图：定义创建对象的接口，让子类决定实例化哪个类

  UML 简图：
  ┌─────────────────┐
  │   <<interface>> │
  │   IWeapon       │
  ├─────────────────┤
  │ + attack(): void│
  └────────┬────────┘
           │
     ┌─────┴──────┐
     │            │
  ┌──┴──┐    ┌────┴───┐
  │Sword│    │  Bow   │
  └─────┘    └────────┘

  ┌─────────────────────────┐
  │    WeaponFactory        │  ◄── 抽象工厂
  ├─────────────────────────┤
  │ + createWeapon(): IWeapon* │
  └────────────┬────────────┘
               │
        ┌──────┴──────┐
        │             │
  ┌─────┴──────┐ ┌────┴───────┐
  │SwordFactory│ │ BowFactory │  ◄── 具体工厂
  └────────────┘ └────────────┘

  调用：
  WeaponFactory* factory = new SwordFactory();
  IWeapon* weapon = factory->createWeapon();  // 返回 Sword*
  weapon->attack();
```

### 3.2 底层构成拆解

| C++ 特性 | 用途 |
|---------|------|
| 继承 + 多态 | 具体工厂继承抽象工厂 |
| 虚函数 | `createWeapon()` 是虚函数 |
| 智能指针 | 返回 `std::unique_ptr<IWeapon>` 管理生命周期 |

### 3.3 伪代码

```cpp
// ===== 产品接口 =====
class IWeapon {
public:
    virtual ~IWeapon() = default;
    virtual void attack() = 0;
    virtual float getDamage() = 0;
};

// ===== 具体产品 =====
class Sword : public IWeapon {
public:
    void attack() override { /* 近战攻击逻辑 */ }
    float getDamage() override { return 25.0f; }
};

class Bow : public IWeapon {
public:
    void attack() override { /* 远程攻击逻辑 */ }
    float getDamage() override { return 15.0f; }
};

class Staff : public IWeapon {
public:
    void attack() override { /* 魔法攻击逻辑 */ }
    float getDamage() override { return 30.0f; }
};

// ===== 抽象工厂 =====
class WeaponFactory {
public:
    virtual ~WeaponFactory() = default;
    virtual std::unique_ptr<IWeapon> createWeapon() = 0;
};

// ===== 具体工厂 =====
class SwordFactory : public WeaponFactory {
public:
    std::unique_ptr<IWeapon> createWeapon() override {
        return std::make_unique<Sword>();
    }
};

class BowFactory : public WeaponFactory {
public:
    std::unique_ptr<IWeapon> createWeapon() override {
        return std::make_unique<Bow>();
    }
};

// ===== 使用 =====
void spawnWeapon(WeaponFactory* factory) {
    auto weapon = factory->createWeapon();
    weapon->attack();
}

// 或者用简单工厂（不需要抽象工厂类）：
enum class WeaponType { Sword, Bow, Staff };

class SimpleWeaponFactory {
public:
    static std::unique_ptr<IWeapon> create(WeaponType type) {
        switch (type) {
            case WeaponType::Sword: return std::make_unique<Sword>();
            case WeaponType::Bow:   return std::make_unique<Bow>();
            case WeaponType::Staff: return std::make_unique<Staff>();
        }
        return nullptr;
    }
};
```

### 3.4 C++ 标准库对应

| 项 | 内容 |
|---|------|
| **头文件** | `<memory>`（`std::unique_ptr`, `std::make_unique`） |
| **游戏注意事项** | |

- **Unity 中的工厂**：`ScriptableObject` 常用于配置驱动的对象创建。
- **UE 中的工厂**：`UFactory` 类用于编辑器中的资源创建，`ASpawnActor` 用于运行时生成 Actor。
- **简单工厂 vs 工厂方法**：简单工厂用 switch-case，工厂方法用继承。游戏开发中简单工厂更常用（代码量少）。

---

## 4. 抽象工厂 Abstract Factory 🎮

### 4.1 结构可视化

```
  Abstract Factory 核心意图：创建一系列相关对象（产品族）

  UML 简图：
  ┌─────────────────────────────────────────┐
  │         UIFactory (抽象工厂)              │
  ├─────────────────────────────────────────┤
  │ + createButton(): IButton               │
  │ + createPanel(): IPanel                 │
  │ + createScrollBar(): IScrollBar         │
  └──────────────────┬──────────────────────┘
                     │
          ┌──────────┴──────────┐
          │                     │
  ┌───────┴────────┐   ┌───────┴────────┐
  │ WindowsFactory │   │  MacFactory    │
  ├────────────────┤   ├────────────────┤
  │ createButton() │   │ createButton() │
  │  → WinButton   │   │  → MacButton   │
  │ createPanel()  │   │ createPanel()  │
  │  → WinPanel    │   │  → MacPanel    │
  └────────────────┘   └────────────────┘

  产品族：
  Windows 风格：WinButton + WinPanel + WinScrollBar
  Mac 风格：    MacButton + MacPanel + MacScrollBar
```

### 4.2 底层构成拆解

| C++ 特性 | 用途 |
|---------|------|
| 多组继承体系 | 多个产品接口（Button、Panel 等） |
| 抽象工厂类 | 定义创建一族产品的方法 |
| 具体工厂类 | 实现特定风格的产品创建 |

### 4.3 伪代码

```cpp
// ===== 产品接口族 =====
class IButton {
public:
    virtual ~IButton() = default;
    virtual void render() = 0;
    virtual void onClick() = 0;
};

class IPanel {
public:
    virtual ~IPanel() = default;
    virtual void render() = 0;
};

// ===== 具体产品 =====
class WindowsButton : public IButton {
public:
    void render() override { /* Windows 风格按钮 */ }
    void onClick() override { /* ... */ }
};

class MacButton : public IButton {
public:
    void render() override { /* Mac 风格按钮 */ }
    void onClick() override { /* ... */ }
};

class WindowsPanel : public IPanel {
public:
    void render() override { /* Windows 风格面板 */ }
};

class MacPanel : public IPanel {
public:
    void render() override { /* Mac 风格面板 */ }
};

// ===== 抽象工厂 =====
class UIFactory {
public:
    virtual ~UIFactory() = default;
    virtual std::unique_ptr<IButton> createButton() = 0;
    virtual std::unique_ptr<IPanel> createPanel() = 0;
};

// ===== 具体工厂 =====
class WindowsUIFactory : public UIFactory {
public:
    std::unique_ptr<IButton> createButton() override {
        return std::make_unique<WindowsButton>();
    }
    std::unique_ptr<IPanel> createPanel() override {
        return std::make_unique<WindowsPanel>();
    }
};

class MacUIFactory : public UIFactory {
public:
    std::unique_ptr<IButton> createButton() override {
        return std::make_unique<MacButton>();
    }
    std::unique_ptr<IPanel> createPanel() override {
        return std::make_unique<MacPanel>();
    }
};

// ===== 使用 =====
class UIManager {
    std::unique_ptr<UIFactory> factory;

public:
    UIManager(Platform platform) {
        if (platform == Platform::Windows)
            factory = std::make_unique<WindowsUIFactory>();
        else
            factory = std::make_unique<MacUIFactory>();
    }

    void createLoginDialog() {
        auto button = factory->createButton();
        auto panel  = factory->createPanel();
        // 保证风格一致
    }
};
```

### 4.4 游戏客户端考点小结

**游戏适用场景**：
- 跨平台 UI 系统（Windows/Mac/Mobile 风格）
- 多套美术资源风格切换（写实/卡通/低多边形）
- 不同渲染后处理管线（HDR/SDR/移动端优化）

**不适用**：
- 只有一套产品族时（过度设计）
- 产品之间没有关联时

**工程陷阱**：
- 类爆炸：每增加一种风格，需要新增 N 个具体产品类 + 1 个工厂类
- 游戏开发中更常用**配置驱动**（JSON/数据表）而非抽象工厂

---

## 5. 建造者 Builder 🎮

### 5.1 结构可视化

```
  Builder 核心意图：分步骤构建复杂对象，分离构造与表示

  UML 简图：
  ┌─────────────────────────────┐
  │        MeshBuilder          │  ◄── 建造者
  ├─────────────────────────────┤
  │ - vertices: vector<Vec3>    │
  │ - normals: vector<Vec3>     │
  │ - uvs: vector<Vec2>         │
  │ - indices: vector<uint16>   │
  ├─────────────────────────────┤
  │ + addVertex(Vec3): Builder& │
  │ + addNormal(Vec3): Builder& │
  │ + addUV(Vec2): Builder&     │
  │ + addTriangle(i0,i1,i2)     │
  │ + build(): Mesh             │
  └─────────────────────────────┘

  链式调用：
  Mesh mesh = MeshBuilder()
      .addVertex({0, 0, 0})
      .addVertex({1, 0, 0})
      .addVertex({0, 1, 0})
      .addNormal({0, 0, 1})
      .addNormal({0, 0, 1})
      .addNormal({0, 0, 1})
      .addTriangle(0, 1, 2)
      .build();
```

### 5.2 底层构成拆解

| C++ 特性 | 用途 |
|---------|------|
| 链式调用（返回 `*this`） | 流式 API |
| 临时容器 | 收集顶点、法线等数据 |
| 最终构建方法 | 一次性生成目标对象 |

### 5.3 伪代码

```cpp
class Mesh {
public:
    std::vector<Vec3> vertices;
    std::vector<Vec3> normals;
    std::vector<Vec2> uvs;
    std::vector<uint16_t> indices;
};

class MeshBuilder {
    std::vector<Vec3> vertices;
    std::vector<Vec3> normals;
    std::vector<Vec2> uvs;
    std::vector<uint16_t> indices;

public:
    MeshBuilder& addVertex(const Vec3& v) {
        vertices.push_back(v);
        return *this;
    }

    MeshBuilder& addNormal(const Vec3& n) {
        normals.push_back(n);
        return *this;
    }

    MeshBuilder& addUV(const Vec2& uv) {
        uvs.push_back(uv);
        return *this;
    }

    MeshBuilder& addTriangle(uint16_t i0, uint16_t i1, uint16_t i2) {
        indices.push_back(i0);
        indices.push_back(i1);
        indices.push_back(i2);
        return *this;
    }

    Mesh build() {
        Mesh mesh;
        mesh.vertices = std::move(vertices);
        mesh.normals  = std::move(normals);
        mesh.uvs      = std::move(uvs);
        mesh.indices  = std::move(indices);
        return mesh;
    }
};

// ===== 使用 =====
auto mesh = MeshBuilder()
    .addVertex({0, 0, 0}).addVertex({1, 0, 0}).addVertex({0, 1, 0})
    .addNormal({0, 0, 1}).addNormal({0, 0, 1}).addNormal({0, 0, 1})
    .addTriangle(0, 1, 2)
    .build();
```

### 5.4 游戏客户端考点小结

**游戏适用场景**：
- 程序化网格生成（地形、植被）
- 复杂配置对象构建（角色属性、技能配置）
- 渲染管线配置（RenderPipelineBuilder）

**不适用**：
- 简单对象（直接用构造函数）
- 不可变对象（Builder 适合可变构建过程）

**工程陷阱**：
- Builder 类过多导致代码膨胀
- 游戏开发中更常用**配置数据 + 解析器**而非 Builder

---

# 结构型模式

---

## 6. 组合模式 Composite 🔧🎮

### 6.1 结构可视化

```
  Composite 核心意图：将对象组合成树形结构，统一处理单个对象和组合

  UML 简图：
  ┌─────────────────────────────┐
  │      <<interface>>          │
  │      IUIElement             │
  ├─────────────────────────────┤
  │ + render(): void            │
  │ + update(float dt): void    │
  │ + addChild(IUIElement*)     │
  │ + removeChild(IUIElement*)  │
  └──────────────┬──────────────┘
                 │
          ┌──────┴──────┐
          │             │
  ┌───────┴──────┐ ┌────┴─────────┐
  │   Leaf       │ │  Composite   │
  │ (Button)     │ │  (Panel)     │
  ├──────────────┤ ├──────────────┤
  │ render()     │ │ children:    │
  │ update()     │ │  vector<     │
  └──────────────┘ │   IUIElement*>│
                   │ render() {   │
                   │   for child: │
                   │     child->  │
                   │      render()│
                   │ }            │
                   └──────────────┘

  UI 树结构示例：
  ┌─────────────────────────────────┐
  │          MainPanel              │
  │  ┌─────────────────────────┐    │
  │  │     HeaderPanel         │    │
  │  │  [Title] [CloseButton]  │    │
  │  └─────────────────────────┘    │
  │  ┌─────────────────────────┐    │
  │  │     ContentPanel        │    │
  │  │  [Avatar] [NameLabel]   │    │
  │  │  [HPBar] [MPBar]        │    │
  │  └─────────────────────────┘    │
  │  ┌─────────────────────────┐    │
  │  │     FooterPanel         │    │
  │  │  [OKButton] [CancelButton]   │
  │  └─────────────────────────┘    │
  └─────────────────────────────────┘

  统一调用：
  mainPanel->render();  // 递归渲染所有子节点
```

### 6.2 底层构成拆解

| C++ 特性 | 用途 |
|---------|------|
| 继承 + 多态 | Leaf 和 Composite 实现同一接口 |
| 容器（vector） | Composite 存储子节点 |
| 递归 | 遍历树形结构 |
| 智能指针 | 管理子节点生命周期 |

### 6.3 伪代码

```cpp
// ===== 组件接口 =====
class IUIElement {
public:
    virtual ~IUIElement() = default;
    virtual void render() = 0;
    virtual void update(float dt) = 0;
    virtual void addChild(IUIElement* child) {}  // 默认空实现
    virtual void removeChild(IUIElement* child) {}
};

// ===== 叶子节点 =====
class Button : public IUIElement {
    std::string text;
    Vec2 position;

public:
    Button(const std::string& t, Vec2 pos) : text(t), position(pos) {}

    void render() override {
        // 绘制按钮
        Renderer::drawButton(text, position);
    }

    void update(float dt) override {
        // 处理按钮逻辑
    }
};

// ===== 组合节点 =====
class Panel : public IUIElement {
    std::vector<std::unique_ptr<IUIElement>> children;
    Vec2 position;

public:
    Panel(Vec2 pos) : position(pos) {}

    void addChild(IUIElement* child) override {
        children.push_back(std::unique_ptr<IUIElement>(child));
    }

    void render() override {
        // 先渲染面板背景
        Renderer::drawPanelBackground(position);
        // 递归渲染所有子节点
        for (auto& child : children) {
            child->render();
        }
    }

    void update(float dt) override {
        for (auto& child : children) {
            child->update(dt);
        }
    }
};

// ===== 使用 =====
auto mainPanel = new Panel({0, 0});

auto header = new Panel({0, 100});
header->addChild(new Button("Title", {10, 110}));
header->addChild(new Button("X", {190, 110}));

auto content = new Panel({0, 50});
content->addChild(new Button("Avatar", {10, 60}));
content->addChild(new Button("HP Bar", {10, 80}));

mainPanel->addChild(header);
mainPanel->addChild(content);

// 统一渲染
mainPanel->render();  // 递归渲染整棵树
```

### 6.4 游戏客户端考点小结

**游戏适用场景**：
- **UI 系统**：所有引擎的 UI 都是树形结构（Unity UGUI、UE UMG）
- **场景图**：节点-子节点层级（Transform 层级）
- **骨骼动画**：骨骼层级树
- **行为树**：AI 决策树

**不适用**：
- 扁平结构（不需要层级关系）
- 叶子和组合差异很大时（接口污染）

**引擎实例**：
- **Unity**：`Transform` 层级，`GameObject` 组合 `Component`
- **UE**：`AActor` 包含 `UActorComponent`，`USceneComponent` 层级
- **Piccolo**：`Entity` 包含 `Component`，场景图是树形结构

**工程陷阱**：
- 过深的层级导致递归开销（遍历性能下降）
- 子节点过多时考虑空间划分（四叉树）而非纯树形

---

## 7. 装饰器 Decorator 🔧🎮

### 7.1 结构可视化

```
  Decorator 核心意图：动态给对象添加功能，替代继承爆炸

  UML 简图：
  ┌─────────────────────┐
  │    ICharacter       │
  ├─────────────────────┤
  │ + getAttack(): int  │
  │ + getDefense(): int │
  └──────────┬──────────┘
             │
      ┌──────┴──────┐
      │             │
  ┌───┴────┐   ┌────┴──────────┐
  │ Warrior│   │  Decorator    │
  │(基础类) │   │  (装饰器基类)  │
  └────────┘   ├───────────────┤
               │ - component:  │
               │   ICharacter* │
               │ + getAttack() │
               │  { return     │
               │  component->  │
               │  getAttack();}│
               └───────┬───────┘
                       │
                ┌──────┴──────┐
                │             │
         ┌──────┴───┐   ┌────┴──────┐
         │SwordBuff │   │ShieldBuff │
         │Decorator │   │Decorator  │
         ├──────────┤   ├───────────┤
         │getAttack()│  │getDefense()│
         │{ return   │  │{ return    │
         │ base + 20 │  │ base + 15  │
         │}          │  │}           │
         └──────────┘   └───────────┘

  装饰过程：
  Warrior(100 atk, 50 def)
    ↓ 加 SwordBuff
  SwordBuff(Warrior) → 120 atk, 50 def
    ↓ 加 ShieldBuff
  ShieldBuff(SwordBuff(Warrior)) → 120 atk, 65 def
```

### 7.2 底层构成拆解

| C++ 特性 | 用途 |
|---------|------|
| 继承 + 多态 | 装饰器继承组件接口 |
| 组合（持有指针） | 装饰器包装被装饰对象 |
| 虚函数 | 动态分发 |

### 7.3 伪代码

```cpp
// ===== 组件接口 =====
class ICharacter {
public:
    virtual ~ICharacter() = default;
    virtual int getAttack() = 0;
    virtual int getDefense() = 0;
    virtual std::string getDescription() = 0;
};

// ===== 基础组件 =====
class Warrior : public ICharacter {
public:
    int getAttack() override { return 100; }
    int getDefense() override { return 50; }
    std::string getDescription() override { return "Warrior"; }
};

// ===== 装饰器基类 =====
class CharacterDecorator : public ICharacter {
protected:
    ICharacter* component;

public:
    CharacterDecorator(ICharacter* comp) : component(comp) {}

    int getAttack() override { return component->getAttack(); }
    int getDefense() override { return component->getDefense(); }
    std::string getDescription() override { return component->getDescription(); }
};

// ===== 具体装饰器 =====
class SwordBuff : public CharacterDecorator {
public:
    SwordBuff(ICharacter* comp) : CharacterDecorator(comp) {}

    int getAttack() override {
        return component->getAttack() + 20;  // 攻击 +20
    }

    std::string getDescription() override {
        return component->getDescription() + " + SwordBuff";
    }
};

class ShieldBuff : public CharacterDecorator {
public:
    ShieldBuff(ICharacter* comp) : CharacterDecorator(comp) {}

    int getDefense() override {
        return component->getDefense() + 15;  // 防御 +15
    }

    std::string getDescription() override {
        return component->getDescription() + " + ShieldBuff";
    }
};

class RageBuff : public CharacterDecorator {
public:
    RageBuff(ICharacter* comp) : CharacterDecorator(comp) {}

    int getAttack() override {
        return component->getAttack() + 30;  // 攻击 +30
    }

    int getDefense() override {
        return component->getDefense() - 10; // 防御 -10（副作用）
    }
};

// ===== 使用 =====
ICharacter* warrior = new Warrior();
warrior = new SwordBuff(warrior);    // 攻击 +20
warrior = new ShieldBuff(warrior);   // 防御 +15
warrior = new RageBuff(warrior);     // 攻击 +30, 防御 -10

// 最终：100+20+30=150 atk, 50+15-10=55 def
```

### 7.4 游戏客户端考点小结

**游戏适用场景**：
- **Buff/Debuff 系统**：动态叠加属性修改
- **装备系统**：装备附加属性
- **技能效果叠加**：多层效果包装

**不适用**：
- 静态属性（直接用继承或配置）
- 装饰器层级过深（性能开销 + 调试困难）

**引擎实例**：
- **Unity**：`Component` 模式类似装饰器（但更灵活）
- **UE**：`GameplayAbility` 的 `GameplayEffect` 可以叠加修改属性

**工程陷阱**：
- 装饰器过多导致对象膨胀
- 调试困难（层层包装，难以追踪）
- 游戏开发中更常用**属性修改器列表**（`vector<Modifier>`）而非装饰器

---

## 8. 适配器 Adapter 🔧🎮

### 8.1 结构可视化

```
  Adapter 核心意图：将一个类的接口转换为客户期望的另一个接口

  UML 简图：
  ┌─────────────────┐     ┌─────────────────┐
  │   IAudioPlayer  │     │  ThirdPartySDK  │
  ├─────────────────┤     ├─────────────────┤
  │ + play(soundId) │     │ + init()        │
  │ + stop()        │     │ + playFile(path)│
  │ + setVolume(v)  │     │ + stopFile()    │
  └─────────────────┘     │ + setVol(float) │
                          └────────┬────────┘
                                   │
                          ┌────────┴────────┐
                          │  AudioAdapter   │  ◄── 适配器
                          ├─────────────────┤
                          │ - sdk: ThirdPartySDK* │
                          │ + play(soundId) │
                          │  {              │
                          │    path =       │
                          │    soundMgr->   │
                          │    getPath(id); │
                          │    sdk->playFile│
                          │    (path);      │
                          │  }              │
                          └─────────────────┘

  调用：
  IAudioPlayer* player = new AudioAdapter();
  player->play(SOUND_EXPLOSION);  // 适配器内部转换
```

### 8.2 底层构成拆解

| C++ 特性 | 用途 |
|---------|------|
| 组合（持有被适配者） | 适配器包装第三方库 |
| 接口实现 | 适配器实现目标接口 |
| 接口转换 | 在适配器内部做参数/逻辑转换 |

### 8.3 伪代码

```cpp
// ===== 目标接口（引擎定义的）=====
class IAudioPlayer {
public:
    virtual ~IAudioPlayer() = default;
    virtual void play(int soundId) = 0;
    virtual void stop() = 0;
    virtual void setVolume(float vol) = 0;
};

// ===== 第三方 SDK（接口不兼容）=====
class ThirdPartyAudioSDK {
public:
    void init() { /* ... */ }
    void playFile(const std::string& filePath) { /* ... */ }
    void stopFile() { /* ... */ }
    void setVolume(float volume) { /* 0.0 ~ 1.0 */ }
};

// ===== 适配器 =====
class AudioAdapter : public IAudioPlayer {
    ThirdPartyAudioSDK* sdk;
    SoundManager* soundMgr;  // 管理 soundId → filePath 映射

public:
    AudioAdapter(ThirdPartyAudioSDK* s, SoundManager* mgr)
        : sdk(s), soundMgr(mgr) {
        sdk->init();
    }

    void play(int soundId) override {
        std::string path = soundMgr->getFilePath(soundId);
        sdk->playFile(path);  // 接口转换：soundId → filePath
    }

    void stop() override {
        sdk->stopFile();
    }

    void setVolume(float vol) override {
        // 引擎用 0~100，SDK 用 0.0~1.0
        sdk->setVolume(vol / 100.0f);
    }
};
```

### 8.4 游戏客户端考点小结

**游戏适用场景**：
- **第三方 SDK 集成**：广告、分析、音频中间件（Wwise/FMOD）
- **跨平台抽象层**：统一不同平台的 API
- **旧系统兼容**：新接口包装旧实现

**不适用**：
- 接口差异太大（适配器会变得很复杂）
- 需要频繁修改被适配者

**引擎实例**：
- **Unity**：`AudioSource` 适配不同平台的音频后端
- **UE**：`IAudioInterface` 适配 FMOD/Wwise
- **Piccolo**：渲染后端适配（Vulkan/DirectX/Metal）

**工程陷阱**：
- 适配器成为"万能转换层"，职责不清
- 性能开销（多一层间接调用）

---

## 9. 外观模式 Facade 🔧🎮

### 9.1 结构可视化

```
  Facade 核心意图：为复杂子系统提供统一的高层接口

  UML 简图：
  ┌─────────────────────────────────────────────────┐
  │                 GameEngine                       │  ◄── 外观
  ├─────────────────────────────────────────────────┤
  │ + init(): void                                  │
  │ + update(dt): void                              │
  │ + shutdown(): void                              │
  └──────────┬──────────────────────────────────────┘
             │ 调用
    ┌────────┼────────┬─────────┬──────────┐
    │        │        │         │          │
    ▼        ▼        ▼         ▼          ▼
┌────────┐┌────────┐┌────────┐┌────────┐┌────────┐
│Renderer││AudioSys││InputSys││Physics ││NetSys  │
└────────┘└────────┘└────────┘└────────┘└────────┘

  没有 Facade 时：
  // 初始化引擎需要调用 10+ 个子系统
  renderer->init();
  audio->init();
  input->init();
  physics->init();
  network->init();
  resource->init();
  // ...

  有 Facade 后：
  GameEngine::init();  // 一行搞定
```

### 9.2 底层构成拆解

| C++ 特性 | 用途 |
|---------|------|
| 组合（持有子系统指针） | 外观类聚合所有子系统 |
| 接口简化 | 隐藏子系统复杂性 |
| 顺序控制 | 管理子系统初始化和更新顺序 |

### 9.3 伪代码

```cpp
class GameEngine {
    Renderer*    renderer;
    AudioSystem* audio;
    InputSystem* input;
    PhysicsWorld* physics;
    NetworkSystem* network;
    ResourceManager* resource;

public:
    void init() {
        // 按依赖顺序初始化
        resource = new ResourceManager();
        resource->init();

        renderer = new Renderer();
        renderer->init(resource);

        audio = new AudioSystem();
        audio->init();

        input = new InputSystem();
        input->init();

        physics = new PhysicsWorld();
        physics->init();

        network = new NetworkSystem();
        network->init();
    }

    void update(float deltaTime) {
        input->pollEvents();
        physics->step(deltaTime);
        // gameLogic->update(deltaTime);
        renderer->beginFrame();
        // renderer->renderScene(scene);
        renderer->endFrame();
        audio->update();
        network->update();
    }

    void shutdown() {
        // 按逆序关闭
        network->shutdown();
        audio->shutdown();
        physics->shutdown();
        input->shutdown();
        renderer->shutdown();
        resource->shutdown();

        delete network;
        delete audio;
        delete physics;
        delete input;
        delete renderer;
        delete resource;
    }
};

// ===== 使用 =====
GameEngine engine;
engine.init();

while (!engine.shouldQuit()) {
    engine.update(0.016f);  // 60 FPS
}

engine.shutdown();
```

### 9.4 游戏客户端考点小结

**游戏适用场景**：
- **引擎主循环**：统一管理所有子系统
- **SDK 封装**：对外提供简单 API
- **模块解耦**：高层模块不直接依赖底层细节

**不适用**：
- 子系统需要精细控制时
- 外观成为"上帝对象"（职责过多）

**引擎实例**：
- **Unity**：`UnityEngine.Application`、`UnityEngine.Debug` 是外观
- **UE**：`UEngine`、`UGameInstance` 是外观
- **Piccolo**：`Engine` 类是外观

**工程陷阱**：
- 外观类过于庞大，成为"上帝对象"
- 隐藏了必要的细节，导致调试困难

---

## 10. 代理 Proxy 🔧🎮

### 10.1 结构可视化

```
  Proxy 核心意图：为其他对象提供一种代理以控制访问

  UML 简图：
  ┌─────────────────┐
  │    IResource    │
  ├─────────────────┤
  │ + load(): void  │
  │ + render(): void│
  │ + unload(): void│
  └────────┬────────┘
           │
    ┌──────┴──────┐
    │             │
┌───┴─────┐  ┌────┴────────┐
│Resource │  │ResourceProxy│  ◄── 代理
│(真实对象)│  ├─────────────┤
└─────────┘  │ - real:     │
             │   Resource* │
             │ - isLoaded: │
             │   bool      │
             ├─────────────┤
             │ + load()    │
             │ + render()  │
             │  { if (!    │
             │   isLoaded) │
             │    load();  │
             │   real->    │
             │    render();│
             │  }          │
             └─────────────┘

  延迟加载（Lazy Loading）：
  创建代理时不加载资源，第一次 render() 时才真正加载
```

### 10.2 底层构成拆解

| C++ 特性 | 用途 |
|---------|------|
| 组合（持有真实对象指针） | 代理包装真实对象 |
| 接口实现 | 代理实现相同接口 |
| 延迟初始化 | 控制真实对象的创建时机 |

### 10.3 伪代码

```cpp
// ===== 接口 =====
class ITexture {
public:
    virtual ~ITexture() = default;
    virtual void load() = 0;
    virtual void render() = 0;
    virtual void unload() = 0;
};

// ===== 真实对象 =====
class Texture : public ITexture {
    std::string filePath;
    GLuint textureID;
    bool isLoaded;

public:
    Texture(const std::string& path) : filePath(path), isLoaded(false) {}

    void load() override {
        if (isLoaded) return;
        // 从磁盘加载纹理（耗时操作）
        textureID = OpenGL::loadTexture(filePath);
        isLoaded = true;
    }

    void render() override {
        if (!isLoaded) load();
        OpenGL::bindTexture(textureID);
    }

    void unload() override {
        if (!isLoaded) return;
        OpenGL::deleteTexture(textureID);
        isLoaded = false;
    }
};

// ===== 代理 =====
class TextureProxy : public ITexture {
    std::string filePath;
    Texture* realTexture;
    bool isLoaded;

public:
    TextureProxy(const std::string& path)
        : filePath(path), realTexture(nullptr), isLoaded(false) {}

    ~TextureProxy() {
        unload();
    }

    void load() override {
        if (isLoaded) return;
        realTexture = new Texture(filePath);
        realTexture->load();
        isLoaded = true;
    }

    void render() override {
        if (!isLoaded) {
            load();  // 延迟加载
        }
        realTexture->render();
    }

    void unload() override {
        if (!isLoaded) return;
        realTexture->unload();
        delete realTexture;
        realTexture = nullptr;
        isLoaded = false;
    }
};

// ===== 使用 =====
// 创建代理时不加载纹理
std::vector<ITexture*> textures;
textures.push_back(new TextureProxy("grass.png"));
textures.push_back(new TextureProxy("rock.png"));
textures.push_back(new TextureProxy("tree.png"));

// 渲染时才加载
for (auto tex : textures) {
    tex->render();  // 第一次调用时才真正加载
}
```

### 10.4 游戏客户端考点小结

**游戏适用场景**：
- **资源延迟加载**：第一次使用时才加载
- **远程资源代理**：网络资源的本地代理
- **权限控制**：检查是否有权限访问资源

**不适用**：
- 需要立即加载的场景
- 代理逻辑过于复杂

**引擎实例**：
- **Unity**：`AssetBundle` 的延迟加载
- **UE**：`TSoftObjectPtr` 是资源代理
- **Piccolo**：资源管理系统的延迟加载

**工程陷阱**：
- 代理层数过多导致性能下降
- 延迟加载可能导致运行时卡顿（首次加载）

---

# 行为型模式

---

## 11. 观察者 Observer 🎮⚡

### 11.1 结构可视化

```
  Observer 核心意图：一对多依赖，状态变化时自动通知所有观察者

  UML 简图：
  ┌─────────────────┐       ┌─────────────────┐
  │    Subject      │       │    Observer     │
  │  (被观察者)      │       │   (观察者接口)    │
  ├─────────────────┤       ├─────────────────┤
  │ - observers:    │       │ + onNotify(data)│
  │   vector<       │       └────────┬────────┘
  │    Observer*>   │                │
  ├─────────────────┤         ┌──────┴──────┐
  │ + attach(obs)   │         │             │
  │ + detach(obs)   │    ┌────┴───┐   ┌─────┴────┐
  │ + notify()      │    │UIPanel │   │AudioMgr  │
  └────────┬────────┘    │Observer│   │Observer  │
           │             └────────┘   └──────────┘
           │ 依赖
           ▼
  ┌─────────────────────────────────┐
  │         GameEvent               │
  │  (具体被观察者，如角色血量变化)    │
  ├─────────────────────────────────┤
  │ - hp: int                       │
  │ - observers: vector<Observer*>  │
  ├─────────────────────────────────┤
  │ + setHP(newHP)                  │
  │  { hp = newHP; notify(); }      │
  │ + notify()                      │
  │  { for obs: obs->onNotify(hp);} │
  └─────────────────────────────────┘

  事件流：
  Player.takeDamage(50)
    → player.hp -= 50
    → player.notify()
      → UIPanel.onNotify(hp) → 更新血条 UI
      → AudioMgr.onNotify(hp) → 播放受伤音效
      → AchievementMgr.onNotify(hp) → 检查成就条件
```

### 11.2 底层构成拆解

| C++ 特性 | 用途 |
|---------|------|
| 接口（Observer） | 定义通知回调 |
| 容器（vector） | 存储观察者列表 |
| 回调函数 / `std::function` | 灵活的事件处理 |
| 弱指针 `std::weak_ptr` | 避免循环引用 |

### 11.3 伪代码

```cpp
// ===== 观察者接口 =====
class IObserver {
public:
    virtual ~IObserver() = default;
    virtual void onNotify(const std::string& event, void* data = nullptr) = 0;
};

// ===== 被观察者（Subject）=====
class Subject {
    std::vector<IObserver*> observers;

public:
    void attach(IObserver* obs) {
        observers.push_back(obs);
    }

    void detach(IObserver* obs) {
        observers.erase(
            std::remove(observers.begin(), observers.end(), obs),
            observers.end()
        );
    }

    void notify(const std::string& event, void* data = nullptr) {
        for (auto obs : observers) {
            obs->onNotify(event, data);
        }
    }
};

// ===== 具体被观察者 =====
class Player : public Subject {
    int hp;
    int maxHP;

public:
    Player(int maxHp) : hp(maxHp), maxHP(maxHp) {}

    void takeDamage(int damage) {
        hp -= damage;
        notify("HP_CHANGED", &hp);
        if (hp <= 0) {
            notify("PLAYER_DEAD");
        }
    }

    int getHP() const { return hp; }
};

// ===== 具体观察者 =====
class UIPanel : public IObserver {
public:
    void onNotify(const std::string& event, void* data) override {
        if (event == "HP_CHANGED") {
            int* hp = static_cast<int*>(data);
            updateHPBar(*hp);
        } else if (event == "PLAYER_DEAD") {
            showGameOverScreen();
        }
    }

    void updateHPBar(int hp) { /* 更新 UI */ }
    void showGameOverScreen() { /* 显示游戏结束 */ }
};

class AudioManager : public IObserver {
public:
    void onNotify(const std::string& event, void* data) override {
        if (event == "HP_CHANGED") {
            playSound(SOUND_HIT);
        } else if (event == "PLAYER_DEAD") {
            playSound(SOUND_DEATH);
        }
    }

    void playSound(int soundId) { /* 播放音效 */ }
};

// ===== 使用 =====
Player player(100);
UIPanel ui;
AudioManager audio;

player.attach(&ui);
player.attach(&audio);

player.takeDamage(30);  // 触发通知
// → ui.onNotify("HP_CHANGED")
// → audio.onNotify("HP_CHANGED")
```

### 11.4 C++ 标准库对应

| 项 | 内容 |
|---|------|
| **头文件** | `<functional>`（`std::function`） |
| **游戏注意事项** | |

- **Unity**：`UnityEvent`、`Action<T>` 是观察者模式实现。
- **UE**：`Delegate`（`FMulticastDelegate`）是观察者模式。
- **Piccolo**：事件系统用观察者模式。
- **性能注意**：观察者过多时 `notify()` 开销大，考虑批量通知或分帧处理。

---

## 12. 状态模式 State 🎮

### 12.1 结构可视化

```
  State 核心意图：允许对象在内部状态改变时改变行为

  UML 简图：
  ┌─────────────────┐
  │   IState        │
  ├─────────────────┤
  │ + enter()       │
  │ + update(dt)    │
  │ + exit()        │
  │ + handleInput() │
  └────────┬────────┘
           │
    ┌──────┴──────┬──────────┬──────────┐
    │             │          │          │
┌───┴────┐  ┌─────┴───┐ ┌────┴───┐ ┌────┴────┐
│IdleState│  │RunState │ │JumpState│ │AttackState│
└────────┘  └─────────┘ └────────┘ └─────────┘

  ┌─────────────────────────────────┐
  │        Character (Context)      │
  ├─────────────────────────────────┤
  │ - currentState: IState*         │
  │ - states: map<StateType,IState*>│
  ├─────────────────────────────────┤
  │ + changeState(newState)         │
  │ + update(dt)                    │
  │  { currentState->update(dt); }  │
  │ + handleInput(input)            │
  │  { currentState->handleInput();}│
  └─────────────────────────────────┘

  状态转换：
  Idle ──(按下W)──► Run
  Run ──(按下Space)──► Jump
  Jump ──(落地)──► Idle 或 Run
  Run ──(按下J)──► Attack
  Attack ──(动画结束)──► Idle
```

### 12.2 底层构成拆解

| C++ 特性 | 用途 |
|---------|------|
| 继承 + 多态 | 具体状态实现接口 |
| 状态机（Context） | 持有当前状态，委托行为 |
| 状态转换逻辑 | 在状态内部或外部定义 |

### 12.3 伪代码

```cpp
// ===== 状态接口 =====
class IState {
public:
    virtual ~IState() = default;
    virtual void enter() = 0;
    virtual void update(float dt) = 0;
    virtual void exit() = 0;
    virtual void handleInput(Input input) = 0;
};

// ===== 具体状态 =====
class IdleState : public IState {
    Character* character;

public:
    IdleState(Character* c) : character(c) {}

    void enter() override {
        character->playAnimation("idle");
    }

    void update(float dt) override {
        // 空闲时不做任何事
    }

    void exit() override {}

    void handleInput(Input input) override {
        if (input == INPUT_MOVE) {
            character->changeState(StateType::Run);
        } else if (input == INPUT_JUMP) {
            character->changeState(StateType::Jump);
        } else if (input == INPUT_ATTACK) {
            character->changeState(StateType::Attack);
        }
    }
};

class RunState : public IState {
    Character* character;

public:
    RunState(Character* c) : character(c) {}

    void enter() override {
        character->playAnimation("run");
    }

    void update(float dt) override {
        character->move(character->direction * character->speed * dt);
    }

    void exit() override {}

    void handleInput(Input input) override {
        if (input == INPUT_STOP) {
            character->changeState(StateType::Idle);
        } else if (input == INPUT_JUMP) {
            character->changeState(StateType::Jump);
        }
    }
};

class JumpState : public IState {
    Character* character;
    float jumpVelocity;
    bool isGrounded;

public:
    JumpState(Character* c) : character(c), jumpVelocity(10.0f), isGrounded(false) {}

    void enter() override {
        character->playAnimation("jump");
        character->velocity.y = jumpVelocity;
    }

    void update(float dt) override {
        character->velocity.y -= 9.8f * dt;  // 重力
        character->move(character->velocity * dt);

        if (character->isOnGround()) {
            isGrounded = true;
            character->changeState(StateType::Idle);
        }
    }

    void exit() override {}

    void handleInput(Input input) override {
        // 跳跃中不接受输入
    }
};

// ===== Context（角色）=====
class Character {
    IState* currentState;
    std::map<StateType, IState*> states;

public:
    void changeState(StateType newState) {
        if (currentState) currentState->exit();
        currentState = states[newState];
        currentState->enter();
    }

    void update(float dt) {
        if (currentState) currentState->update(dt);
    }

    void handleInput(Input input) {
        if (currentState) currentState->handleInput(input);
    }
};
```

### 12.4 游戏客户端考点小结

**游戏适用场景**：
- **角色状态机**：Idle/Run/Jump/Attack
- **UI 状态**：菜单/游戏/暂停
- **游戏流程**：加载/游戏/结算

**不适用**：
- 状态过多（类爆炸）
- 状态之间有复杂转换（用层次状态机或行为树）

**引擎实例**：
- **Unity**：`Animator` 状态机
- **UE**：`UStateMachine`（或自研）
- **Piccolo**：角色状态机

**工程陷阱**：
- 状态类爆炸（10 个状态 = 10 个类）
- 状态转换逻辑分散（难以维护）
- 游戏开发中常用**表驱动状态机**（状态转换表）而非纯状态模式

---

## 13. 命令模式 Command 🎮

### 13.1 结构可视化

```
  Command 核心意图：将请求封装为对象，支持撤销、队列化、日志

  UML 简图：
  ┌─────────────────┐
  │    ICommand     │
  ├─────────────────┤
  │ + execute()     │
  │ + undo()        │
  └────────┬────────┘
           │
    ┌──────┴──────┬──────────┬──────────┐
    │             │          │          │
┌───┴─────┐ ┌─────┴───┐ ┌────┴───┐ ┌────┴────┐
│MoveCmd  │ │AttackCmd│ │JumpCmd │ │UseItemCmd│
└─────────┘ └─────────┘ └────────┘ └─────────┘

  ┌─────────────────────────────────┐
  │       CommandQueue              │
  ├─────────────────────────────────┤
  │ - queue: std::queue<ICommand*>  │
  ├─────────────────────────────────┤
  │ + enqueue(cmd)                  │
  │ + processAll()                  │
  │  { while !empty:               │
  │      cmd = queue.front();      │
  │      cmd->execute();           │
  │      queue.pop();              │
  │  }                             │
  └─────────────────────────────────┘

  输入系统：
  按键按下 → 创建 MoveCommand → 入队
  主循环 → 处理命令队列 → 执行命令

  撤销系统：
  执行命令 → 压入 undoStack
  按 Ctrl+Z → 从 undoStack 弹出 → 调用 undo()
```

### 13.2 底层构成拆解

| C++ 特性 | 用途 |
|---------|------|
| 接口（ICommand） | 定义 execute/undo |
| 多态 | 不同命令实现不同行为 |
| 容器（queue/stack） | 命令队列和撤销栈 |
| 智能指针 | 管理命令生命周期 |

### 13.3 伪代码

```cpp
// ===== 命令接口 =====
class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
};

// ===== 具体命令 =====
class MoveCommand : public ICommand {
    Character* character;
    Vec3 oldPosition;
    Vec3 newPosition;

public:
    MoveCommand(Character* c, Vec3 target)
        : character(c), newPosition(target) {}

    void execute() override {
        oldPosition = character->getPosition();
        character->setPosition(newPosition);
    }

    void undo() override {
        character->setPosition(oldPosition);
    }
};

class AttackCommand : public ICommand {
    Character* attacker;
    Character* target;
    int damageDealt;

public:
    AttackCommand(Character* a, Character* t)
        : attacker(a), target(t), damageDealt(0) {}

    void execute() override {
        damageDealt = attacker->getAttack() - target->getDefense();
        if (damageDealt < 0) damageDealt = 0;
        target->takeDamage(damageDealt);
    }

    void undo() override {
        target->heal(damageDealt);
    }
};

// ===== 命令队列 =====
class CommandQueue {
    std::queue<std::unique_ptr<ICommand>> queue;

public:
    void enqueue(std::unique_ptr<ICommand> cmd) {
        queue.push(std::move(cmd));
    }

    void processAll() {
        while (!queue.empty()) {
            auto cmd = std::move(queue.front());
            queue.pop();
            cmd->execute();
        }
    }
};

// ===== 撤销管理器 =====
class UndoManager {
    std::stack<std::unique_ptr<ICommand>> undoStack;
    std::stack<std::unique_ptr<ICommand>> redoStack;

public:
    void executeCommand(std::unique_ptr<ICommand> cmd) {
        cmd->execute();
        undoStack.push(std::move(cmd));
        redoStack = {};  // 清空 redo
    }

    void undo() {
        if (undoStack.empty()) return;
        auto cmd = std::move(undoStack.top());
        undoStack.pop();
        cmd->undo();
        redoStack.push(std::move(cmd));
    }

    void redo() {
        if (redoStack.empty()) return;
        auto cmd = std::move(redoStack.top());
        redoStack.pop();
        cmd->execute();
        undoStack.push(std::move(cmd));
    }
};
```

### 13.4 游戏客户端考点小结

**游戏适用场景**：
- **输入系统**：按键→命令→执行
- **撤销/重做**：编辑器操作、回合制游戏
- **命令队列**：网络命令、异步操作
- **回放系统**：记录命令序列，重放游戏

**不适用**：
- 简单操作（命令对象开销大于收益）
- 实时性要求极高（命令队列有延迟）

**引擎实例**：
- **Unity**：`InputSystem` 可以封装为命令
- **UE**：`UInputAction` 绑定命令
- **Piccolo**：输入系统用命令模式

**工程陷阱**：
- 命令对象过多导致内存开销
- 撤销逻辑复杂（需要保存完整状态）

---

## 14-17. 其他行为型模式（简述）

### 14. 迭代器 Iterator

**核心意图**：顺序访问聚合对象的元素，不需要暴露内部表示。

**游戏应用**：遍历场景中的实体、UI 子节点、背包物品。

**C++ 对应**：`std::iterator`、范围 for 循环。

**引擎实例**：Unity 的 `Transform.GetEnumerator()`、UE 的 `TObjectIterator`。

---

### 15. 访问者 Visitor

**核心意图**：在不修改类的前提下给类添加新操作。

**游戏应用**：对不同类型的碰撞体执行不同操作（AABB、OBB、Mesh）。

**工程陷阱**：增加新类型困难（需要修改所有 Visitor）。

---

### 16. 策略模式 Strategy

**核心意图**：定义一系列算法，封装起来，可以互相替换。

**游戏应用**：
- 寻路算法切换（A*/Dijkstra/FlowField）
- AI 行为选择（巡逻/追击/逃跑）
- 渲染策略（前向渲染/延迟渲染）

**与状态模式对比**：策略强调算法替换，状态强调状态转换。

---

### 17. 中介者 Mediator

**核心意图**：用中介对象封装一系列对象的交互方式。

**游戏应用**：
- 聊天系统（玩家→中介者→其他玩家）
- UI 事件分发（按钮→中介者→多个面板）
- 多人游戏同步

**与观察者对比**：中介者集中控制，观察者分散通知。

---

# 第二部分：模式对比与工程取舍

---

## 模式对比小节

### 1. 单例 vs 对象池

| 维度 | 单例 | 对象池 |
|------|------|--------|
| **目的** | 全局唯一实例 | 复用对象，避免频繁分配 |
| **实例数** | 1 个 | N 个（池中多个） |
| **生命周期** | 全局 | 按需 acquire/release |
| **游戏场景** | GameManager、AudioSystem | Bullet、Particle、Entity |
| **工程陷阱** | 全局依赖、测试困难 | 池大小不当导致浪费或扩容 |

**选型建议**：
- 管理器类用单例
- 频繁创建销毁的对象用对象池

---

### 2. 策略 vs 状态

| 维度 | 策略模式 | 状态模式 |
|------|----------|----------|
| **目的** | 算法替换 | 状态转换 |
| **切换方式** | 外部设置 | 内部转换或外部触发 |
| **关注点** | 怎么做（How） | 当前是什么（What） |
| **游戏场景** | 寻路算法、AI 行为 | 角色状态、UI 状态 |
| **类爆炸** | 策略多时 | 状态多时 |

**选型建议**：
- 需要动态切换算法 → 策略
- 需要管理状态转换 → 状态

---

### 3. 观察者 vs 中介者

| 维度 | 观察者模式 | 中介者模式 |
|------|------------|------------|
| **通信方式** | 广播（一对多） | 集中（多对多通过中介） |
| **耦合度** | 低（观察者不知道 Subject） | 中（同事知道 Mediator） |
| **控制流** | 分散 | 集中 |
| **游戏场景** | 事件系统、UI 更新 | 聊天系统、多人同步 |
| **调试难度** | 高（事件流难追踪） | 低（中介者集中） |

**选型建议**：
- 松耦合事件通知 → 观察者
- 复杂交互需要集中控制 → 中介者

---

### 4. 工厂方法 vs 抽象工厂

| 维度 | 工厂方法 | 抽象工厂 |
|------|----------|----------|
| **创建粒度** | 单个产品 | 产品族 |
| **复杂度** | 低 | 高 |
| **游戏场景** | 创建单个武器 | 创建一套 UI 风格 |
| **类爆炸** | 中等 | 严重 |

**选型建议**：
- 创建单个对象 → 工厂方法（或简单工厂）
- 创建一族相关对象 → 抽象工厂

---

### 5. 装饰器 vs 策略

| 维度 | 装饰器 | 策略 |
|------|--------|------|
| **目的** | 动态添加功能 | 算法替换 |
| **包装方式** | 层层包装 | 直接替换 |
| **游戏场景** | Buff 叠加 | 寻路算法切换 |
| **性能** | 多层调用开销 | 单层调用 |

**选型建议**：
- 需要叠加功能 → 装饰器
- 需要替换算法 → 策略

---

# 附录：游戏客户端面试高频考点速记

## 创建型

1. **单例的线程安全问题？** → Meyers 单例（C++11 静态局部变量）
2. **对象池的 acquire/release 如何 O(1)？** → 用索引映射，避免 find
3. **工厂方法 vs 抽象工厂？** → 单个产品 vs 产品族

## 结构型

4. **组合模式在引擎中的应用？** → UI 树、场景图、骨骼层级
5. **装饰器 vs 属性修改器列表？** → 游戏开发中更常用列表
6. **适配器的性能开销？** → 多一层间接调用

## 行为型

7. **观察者模式的性能问题？** → 观察者过多时 notify 开销大
8. **状态模式的类爆炸？** → 用表驱动状态机
9. **命令模式的撤销实现？** → undoStack + redoStack

## 工程取舍

10. **什么时候不用设计模式？** → 简单问题不要过度设计
11. **数据驱动 vs 模式驱动？** → 游戏开发中配置数据更重要
12. **性能 vs 可维护性？** → 引擎底层优先性能，业务层优先可维护

---

> **手册结语**：设计模式不是银弹。游戏客户端开发的特殊之处在于：**性能是第一优先级**。一个简单直接的实现，往往比套用设计模式更合适。始终从实际需求出发，考虑帧率预算、内存限制、团队规模。设计模式是工具，不是教条。
