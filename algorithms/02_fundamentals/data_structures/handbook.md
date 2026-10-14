# 游戏客户端开发专用数据结构学习手册

> **目标读者**：准备游戏客户端岗位、有 C++ 基础的学习者
> **核心原则**：不堆砌学术冷门内容，所有内容优先突出在游戏引擎/游戏业务里的实际使用场景，服务于引擎底层、场景管理、渲染、对象池、UI、寻路、资源管理等客户端开发场景。

---

# 第一部分：数据结构原理与底层实现

---

## 1. 动态数组 Vector

### 1.1 逻辑构造原理图

```
  Vector 逻辑结构：
  ┌─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┐
  │  A  │  B  │  C  │  D  │     │     │     │     │
  └─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┘
   [0]   [1]   [2]   [3]   [4]   [5]   [6]   [7]
   ◄──── size=4 ────►◄───── capacity=8 ─────────►|

  内存布局（连续）：
  data_ ──► [elem0][elem1][elem2][elem3][ 未构造 ][ 未构造 ]...

  三指针模型（典型实现）：
  ┌──────────┐   ┌──────────┐   ┌──────────┐
  │ data_    │   │ size_    │   │ end_     │
  │ (首地址)  │   │ (已用末尾)│   │ (容量末尾)│
  └──────────┘   └──────────┘   └──────────┘
       │               │              │
       ▼               ▼              ▼
      [A][B][C][D][   未构造空间     ]
```

**原理**：Vector 在**连续内存**上存储元素，支持 O(1) 随机访问。当容量不足时，分配一块更大的新内存（通常 1.5x 或 2x），将旧元素逐个 move/copy 到新内存，再释放旧内存。

### 1.2 底层构成拆解

| 基础组件 | 用途 |
|---------|------|
| 原生数组（连续内存块） | 存储元素，通过 `new T[capacity]` 或 `allocator::allocate` 分配 |
| size_t 计数器 | 记录已用大小 `size` 和总容量 `capacity` |
| 内存分配器 | 负责 raw memory 的分配与释放（`std::allocator` 或自定义 allocator） |

本质上 = **一块连续堆内存 + 两个指针/计数器**。

### 1.3 伪代码

```cpp
template <typename T>
class Vector {
    T*      data_;      // 指向首元素
    size_t  size_;      // 当前元素个数
    size_t  capacity_;  // 当前分配的槽位数

public:
    Vector() : data_(nullptr), size_(0), capacity_(0) {}

    ~Vector() {
        for (size_t i = 0; i < size_; ++i)
            data_[i].~T();                    // 逐个析构
        delete[] data_;                        // 释放 raw memory
    }

    void push_back(const T& value) {
        if (size_ >= capacity_) {
            grow();                            // 扩容
        }
        data_[size_] = value;                  // copy-assign 或 placement new
        ++size_;
    }

    void grow() {
        size_t new_cap = (capacity_ == 0) ? 1 : capacity_ * 2;  // 2 倍增长
        T* new_data = new T[new_cap];          // 分配新内存
        for (size_t i = 0; i < size_; ++i) {
            new_data[i] = std::move(data_[i]); // move 到 new buffer
        }
        delete[] data_;                        // 释放旧内存
        data_     = new_data;
        capacity_ = new_cap;
    }

    T& operator[](size_t index) {
        return data_[index];                   // O(1) 随机访问
    }

    void pop_back() {
        --size_;
        data_[size_].~T();                     // 析构但不释放内存
    }

    void reserve(size_t new_cap) {
        if (new_cap <= capacity_) return;
        T* new_data = new T[new_cap];
        for (size_t i = 0; i < size_; ++i)
            new_data[i] = std::move(data_[i]);
        delete[] data_;
        data_     = new_data;
        capacity_ = new_cap;
    }
};
```

### 1.4 C++ 标准库对应容器

| 项 | 内容 |
|---|------|
| **容器** | `std::vector` |
| **头文件** | `<vector>` |
| **游戏开发注意事项** | |

- **扩容开销**：每次 `push_back` 触发扩容时，所有元素需要 move/copy，单次代价 O(n)。游戏中应在 `reserve()` 预分配，避免运行时频繁扩容卡顿。
- **缓存友好**：连续内存布局，CPU cache line 命中率高，是游戏开发中**最常用的容器**。
- **中间插入/删除**：O(n)，不适合频繁在中间增删的场景（此时考虑链表或 free-list）。
- **指针/引用失效**：扩容后所有指针、引用、迭代器全部失效。游戏逻辑中如果持有 `vector<Entity*>` 内部元素的裸指针，扩容后会导致崩溃。
- **内存碎片**：大块连续分配，本身不产生碎片，但频繁扩容释放旧大块会产生大块碎片。

---

## 2. 单向链表 & 双向链表

### 2.1 逻辑构造原理图

```
单向链表 (Singly Linked List)：
  head ──► [val|next] ──► [val|next] ──► [val|next] ──► nullptr
            node_0        node_1        node_2

双向链表 (Doubly Linked List)：
  head ──► ┌────────┐    ┌────────┐    ┌────────┐ ◄── tail
           │prev|val|next│──►│prev|val|next│──►│prev|val|next│
  nullptr ◄─┤  node  │◄───┤  node  │◄───┤  node  │──► nullptr
           └────────┘    └────────┘    └────────┘
              node_0        node_1        node_2

  内存分布（非连续，每个节点独立分配）：
  0x1000: [node_0]     ← 散落在堆上各处
  0x5F20: [node_1]
  0x3A80: [node_2]
```

**原理**：
- **单向链表**：每个节点存储数据 + 指向下一个节点的指针，只能从前往后遍历。
- **双向链表**：每个节点额外存储指向前一个节点的指针，可以双向遍历。
- 两者都不要求连续内存，插入/删除只需修改指针。

### 2.2 底层构成拆解

| 基础组件 | 用途 |
|---------|------|
| 节点结构体（Node） | 存储数据 + 前驱/后继指针 |
| 堆内存逐节点分配 | 每个 `new Node` 独立分配，节点间物理地址不连续 |
| head/tail 指针 | 维护链表入口 |

本质上 = **零散的堆节点 + 指针串联**。

### 2.3 伪代码

```cpp
// ===== 双向链表 =====
template <typename T>
struct ListNode {
    T         data;
    ListNode* prev;
    ListNode* next;
    ListNode(const T& val) : data(val), prev(nullptr), next(nullptr) {}
};

template <typename T>
class DoublyLinkedList {
    ListNode<T>* head;
    ListNode<T>* tail;
    size_t       length;

public:
    DoublyLinkedList() : head(nullptr), tail(nullptr), length(0) {}

    // 尾部插入 O(1)
    void push_back(const T& value) {
        ListNode<T>* node = new ListNode<T>(value);
        if (tail == nullptr) {
            head = tail = node;
        } else {
            node->prev = tail;
            tail->next = node;
            tail = node;
        }
        ++length;
    }

    // 指定位置前插入 O(n) 查找 + O(1) 插入
    void insert(size_t pos, const T& value) {
        ListNode<T>* node = new ListNode<T>(value);
        if (pos == 0) {
            node->next = head;
            if (head) head->prev = node;
            head = node;
            if (!tail) tail = node;
        } else {
            ListNode<T>* cur = head;
            for (size_t i = 0; i < pos - 1 && cur; ++i)
                cur = cur->next;
            if (!cur) return;
            node->next = cur->next;
            node->prev = cur;
            if (cur->next) cur->next->prev = node;
            else tail = node;
            cur->next = node;
        }
        ++length;
    }

    // 删除指定节点 O(1)（已持有节点指针时）
    void erase(ListNode<T>* node) {
        if (!node) return;
        if (node->prev) node->prev->next = node->next;
        else head = node->next;
        if (node->next) node->next->prev = node->prev;
        else tail = node->prev;
        delete node;
        --length;
    }

    // 遍历
    void for_each(void (*func)(T&)) {
        ListNode<T>* cur = head;
        while (cur) {
            func(cur->data);
            cur = cur->next;
        }
    }
};
```

### 2.4 C++ 标准库对应容器

| 项 | 内容 |
|---|------|
| **容器** | `std::list`（双向链表）、`std::forward_list`（单向链表） |
| **头文件** | `<list>`、`<forward_list>` |
| **游戏开发注意事项** | |

- **内存碎片严重**：每个节点独立 `new`，大量小对象散落在堆上，长期运行后堆碎片化严重。游戏开发中**极少直接使用** `std::list`。
- **缓存不友好**：节点物理地址不连续，CPU cache line 命中率极低。遍历链表比遍历 vector 慢一个数量级。
- **替代方案**：游戏引擎中通常使用**基于数组的 free-list**（对象池模式）来替代链表，兼顾 O(1) 插入删除与缓存友好。
- **适用场景**：需要频繁在中间插入/删除且**不频繁遍历**的场景，如活跃/非活跃实体分离管理。

---

## 3. 栈 Stack

### 3.1 逻辑构造原理图

```
  栈 (LIFO - Last In First Out)：

       │       │
       │  C    │  ◄── top（最后入栈，最先出栈）
       │  B    │
       │  A    │  ◄── bottom
       └───────┘

  入栈 push(D)：        出栈 pop()：
       │       │             │       │
       │  D    │ ◄── top     │  C    │ ◄── top (D 已弹出)
       │  C    │             │  B    │
       │  B    │             │  A    │
       │  A    │             │  A    │
       └───────┘             └───────┘

  底层实现：通常基于 vector / 动态数组
  ┌───┬───┬───┬───┬───┬───┐
  │ A │ B │ C │   │   │   │  capacity
  └───┴───┴───┴───┴───┴───┘
              ▲
            top = 2
```

**原理**：只允许在一端（栈顶）进行插入和删除。后进先出（LIFO）。

### 3.2 底层构成拆解

| 基础组件 | 用途 |
|---------|------|
| 动态数组（Vector）或固定数组 | 存储元素，提供连续内存 |
| top 指针/索引 | 标记栈顶位置 |

本质上 = **一个数组 + 一个 top 索引**，限制只能在一端操作。

### 3.3 伪代码

```cpp
template <typename T>
class Stack {
    T*     data_;
    size_t top_;      // 指向栈顶元素的下一个位置
    size_t capacity_;

public:
    Stack(size_t cap = 8) : top_(0), capacity_(cap) {
        data_ = new T[capacity_];
    }

    ~Stack() { delete[] data_; }

    void push(const T& value) {
        if (top_ >= capacity_) {
            // 扩容（同 vector 逻辑）
            size_t new_cap = capacity_ * 2;
            T* new_data = new T[new_cap];
            for (size_t i = 0; i < top_; ++i)
                new_data[i] = std::move(data_[i]);
            delete[] data_;
            data_     = new_data;
            capacity_ = new_cap;
        }
        data_[top_++] = value;
    }

    void pop() {
        if (top_ > 0) --top_;
    }

    T& top() {
        return data_[top_ - 1];
    }

    bool empty() const { return top_ == 0; }
    size_t size() const { return top_; }
};
```

### 3.4 C++ 标准库对应容器

| 项 | 内容 |
|---|------|
| **容器** | `std::stack`（容器适配器，默认底层为 `std::deque`） |
| **头文件** | `<stack>` |
| **游戏开发注意事项** | |

- **默认底层是 deque**：`std::stack` 默认用 `std::deque` 做底层，而 deque 是分段连续结构，缓存友好性不如 vector。游戏开发中建议 `std::stack<T, std::vector<T>>` 显式指定 vector 为底层容器。
- **调用栈模拟**：脚本虚拟机（如 Lua VM）、UI 导航栈（场景 push/pop）、动画状态机状态保存都依赖栈结构。
- **固定大小栈**：性能敏感场景可用固定大小数组栈，避免堆分配。

---

## 4. 队列 Queue & 循环队列

### 4.1 逻辑构造原理图

```
普通队列 (FIFO - First In First Out)：
  出队 ◄── [ A | B | C | D ] ◄── 入队
  dequeue    head           tail   enqueue

循环队列 (Circular Queue)：
  基于固定大小数组，head 和 tail 绕回：

  初始状态：
  ┌───┬───┬───┬───┬───┬───┬───┬───┐
  │   │   │   │   │   │   │   │   │
  └───┴───┴───┴───┴───┴───┴───┴───┘
   ▲                               ▲
  head                            tail
  (head == tail → 空)

  入队 3 个元素后：
  ┌───┬───┬───┬───┬───┬───┬───┬───┐
  │   │ A │ B │ C │   │   │   │   │
  └───┴───┴───┴───┴───┴───┴───┴───┘
        ▲           ▲
       head        tail

  出队 2 个，再入队 4 个后（tail 绕回）：
  ┌───┬───┬───┬───┬───┬───┬───┬───┐
  │ E │ F │ G │ H │   │   │ C │ D │
  └───┴───┴───┴───┴───┴───┴───┴───┘
                ▲               ▲
               head           tail

  关键公式：
    next_index = (current_index + 1) % capacity
    count      = (tail - head + capacity) % capacity
```

**原理**：
- **普通队列**：FIFO，一端入队，另一端出队。
- **循环队列**：用固定大小数组实现队列，head/tail 到达数组末尾后绕回到开头，避免普通数组队列的"假溢出"问题。

### 4.2 底层构成拆解

| 基础组件 | 用途 |
|---------|------|
| 固定大小数组 或 动态数组 | 存储元素 |
| head 索引 | 指向队首元素 |
| tail 索引 | 指向队尾下一个空位 |
| 取模运算 | 实现循环绕回 |

本质上 = **数组 + 两个索引 + 取模绕回逻辑**。

### 4.3 伪代码

```cpp
template <typename T>
class CircularQueue {
    T*     data_;
    size_t head_;
    size_t tail_;
    size_t capacity_;

public:
    CircularQueue(size_t cap) : head_(0), tail_(0), capacity_(cap + 1) {
        // 多分配 1 个槽位，用于区分空和满
        data_ = new T[capacity_];
    }

    ~CircularQueue() { delete[] data_; }

    bool empty() const { return head_ == tail_; }

    bool full() const { return (tail_ + 1) % capacity_ == head_; }

    void enqueue(const T& value) {
        if (full()) return;  // 或扩容
        data_[tail_] = value;
        tail_ = (tail_ + 1) % capacity_;
    }

    T dequeue() {
        if (empty()) return T();  // 错误处理
        T val = data_[head_];
        head_ = (head_ + 1) % capacity_;
        return val;
    }

    T& front() { return data_[head_]; }

    size_t size() const {
        return (tail_ - head_ + capacity_) % capacity_;
    }
};
```

### 4.4 C++ 标准库对应容器

| 项 | 内容 |
|---|------|
| **容器** | `std::queue`（默认底层 `std::deque`） |
| **头文件** | `<queue>` |
| **游戏开发注意事项** | |

- **循环队列在游戏中极常用**：消息队列、输入事件缓冲、网络包缓冲、渲染命令队列等，通常用**固定大小环形缓冲区**实现，避免运行时内存分配。
- **std::queue 底层是 deque**：分段连续，不如自定义环形缓冲高效。游戏中推荐手写 `CircularQueue`。
- **双端队列 `std::deque`**：支持两端 push/pop，适合工作窃取（work-stealing）任务调度。

---

## 5. 哈希表（散列表）

### 5.1 逻辑构造原理图

```
  哈希表结构（链地址法 / Separate Chaining）：

  hash(key) % bucket_count
        │
        ▼
  ┌──────────────────────────────────────────┐
  │ buckets[] (桶数组)                        │
  │  [0] ──► [K:"hp",V:100] ──► [K:"mp",V:50]│
  │  [1] ──► [K:"pos",V:(3,4)]               │
  │  [2] ──► null                              │
  │  [3] ──► [K:"name",V:"hero"] ──► [...]    │
  │  ...                                       │
  │  [15] ─► null                              │
  └──────────────────────────────────────────┘

  开放寻址法（Open Addressing，线性探测）：
  ┌──────────────────────────────────────────┐
  │ slots[] (扁平数组，每个槽存 key+value)      │
  │  [0]  [EMPTY]                              │
  │  [1]  [K:"hp",  V:100]  ◄── hash("hp")=1  │
  │  [2]  [K:"mp",  V:50]   ◄── hash("mp")=4, 但 4 被占, 探测到 2 │
  │  [3]  [K:"pos", V:(3,4)] ◄── hash("pos")=3 │
  │  [4]  [K:"atk", V:25]   ◄── hash("atk")=4  │
  │  [5]  [EMPTY]                              │
  └──────────────────────────────────────────┘

  哈希函数映射过程：
  key ──► hash_function() ──► hash_code ──► hash_code % bucket_count ──► bucket_index
  "hp"       多项式哈希         31            31 % 16 = 15                  15
```

**原理**：通过哈希函数将 key 映射到数组索引，实现 O(1) 平均查找。冲突处理有两种主流方案：链地址法和开放寻址法。

### 5.2 底层构成拆解

| 基础组件 | 链地址法 | 开放寻址法 |
|---------|---------|-----------|
| 桶数组 | `Bucket*` 数组，每个桶是链表头 | `Slot{key, value, state}` 扁平数组 |
| 链表节点 | 冲突元素挂在链表上 | 不需要，冲突时线性/二次探测下一个空槽 |
| 负载因子 | `size / bucket_count`，超过阈值扩容 | 同左，但阈值通常更低（0.5~0.7） |

游戏引擎中**开放寻址法更常见**（如 UE 的 `TMap`），因为缓存友好、无额外链表分配。

### 5.3 伪代码（开放寻址法 - 线性探测）

```cpp
template <typename K, typename V>
class HashMap {
    struct Slot {
        K      key;
        V      value;
        bool   occupied;    // 是否被占用
        bool   deleted;     // 墓碑标记（懒惰删除）
    };

    Slot*  slots_;
    size_t capacity_;
    size_t size_;

    size_t hash(const K& key) const {
        // 简化：假设 key 有 hash 方法
        return std::hash<K>{}(key);
    }

    size_t probe(size_t idx) const {
        return (idx + 1) % capacity_;  // 线性探测
    }

public:
    HashMap(size_t cap = 16) : capacity_(cap), size_(0) {
        slots_ = new Slot[capacity_];
        for (size_t i = 0; i < capacity_; ++i) {
            slots_[i].occupied = false;
            slots_[i].deleted  = false;
        }
    }

    void insert(const K& key, const V& value) {
        if (size_ >= capacity_ * 0.7) rehash();  // 负载因子 0.7

        size_t idx = hash(key) % capacity_;
        while (slots_[idx].occupied && !slots_[idx].deleted
               && slots_[idx].key != key) {
            idx = probe(idx);
        }
        if (!slots_[idx].occupied || slots_[idx].deleted) ++size_;
        slots_[idx].key      = key;
        slots_[idx].value    = value;
        slots_[idx].occupied = true;
        slots_[idx].deleted  = false;
    }

    V* find(const K& key) {
        size_t idx = hash(key) % capacity_;
        while (slots_[idx].occupied || slots_[idx].deleted) {
            if (slots_[idx].occupied && slots_[idx].key == key)
                return &slots_[idx].value;
            idx = probe(idx);
        }
        return nullptr;  // 未找到
    }

    void erase(const K& key) {
        size_t idx = hash(key) % capacity_;
        while (slots_[idx].occupied || slots_[idx].deleted) {
            if (slots_[idx].occupied && slots_[idx].key == key) {
                slots_[idx].occupied = false;
                slots_[idx].deleted  = true;  // 墓碑
                --size_;
                return;
            }
            idx = probe(idx);
        }
    }

    void rehash() {
        size_t old_cap = capacity_;
        Slot*  old_slots = slots_;
        capacity_ *= 2;
        slots_ = new Slot[capacity_];
        size_ = 0;
        for (size_t i = 0; i < capacity_; ++i) {
            slots_[i].occupied = false;
            slots_[i].deleted  = false;
        }
        for (size_t i = 0; i < old_cap; ++i) {
            if (old_slots[i].occupied && !old_slots[i].deleted)
                insert(old_slots[i].key, old_slots[i].value);
        }
        delete[] old_slots;
    }
};
```

### 5.4 C++ 标准库对应容器

| 项 | 内容 |
|---|------|
| **容器** | `std::unordered_map` / `std::unordered_set` |
| **头文件** | `<unordered_map>` / `<unordered_set>` |
| **游戏开发注意事项** | |

- **链地址法实现**：`std::unordered_map` 使用链表处理冲突，每个节点独立分配，内存碎片严重。游戏开发中通常**不用** `std::unordered_map`。
- **引擎替代方案**：UE 使用 `TMap`（开放寻址 + 扁平数组），自研引擎通常手写 flat hash map。
- **rehash 开销**：扩容时需要重新哈希所有元素，可能引起卡顿。游戏运行期应 `reserve()` 预分配。
- **自定义 key 类型**：需要提供 `hash` 函数和 `operator==`。游戏开发中常用 `FName`、`EntityID` 等作为 key。
- **缓存友好性**：开放寻址法 >> 链地址法。遍历哈希表时差异巨大。

---

## 6. 二叉搜索树 BST

### 6.1 逻辑构造原理图

```
  二叉搜索树 (Binary Search Tree)：
  性质：左子树所有节点 < 根 < 右子树所有节点

            [8]
           /   \
         [3]   [10]
        /  \      \
      [1]  [6]    [14]
          /  \    /
        [4]  [7][13]

  查找 key=6 的路径：
  [8] → 6 < 8, 走左 → [3] → 6 > 3, 走右 → [6] 找到！

  中序遍历（得到有序序列）：
  1 → 3 → 4 → 6 → 7 → 8 → 10 → 13 → 14

  最坏情况（退化为链表）：
  依次插入 1,2,3,4,5：
  [1]
    \
    [2]
      \
      [3]
        \
        [4]
          \
          [5]
  此时查找退化为 O(n)
```

**原理**：每个节点最多两个子节点，满足左 < 根 < 右的性质。查找、插入、删除平均 O(log n)，最坏 O(n)。

### 6.2 底层构成拆解

| 基础组件 | 用途 |
|---------|------|
| 节点结构体 | 存储 key、left 指针、right 指针 |
| 堆上逐节点分配 | 每个节点独立 `new` |
| root 指针 | 指向根节点 |

本质上 = **每个节点带两个指针的链式结构**，通过 key 的大小关系组织成树形。

### 6.3 伪代码

```cpp
template <typename K>
struct BSTNode {
    K        key;
    BSTNode* left;
    BSTNode* right;
    BSTNode(const K& k) : key(k), left(nullptr), right(nullptr) {}
};

template <typename K>
class BST {
    BSTNode<K>* root;

    BSTNode<K>* insertImpl(BSTNode<K>* node, const K& key) {
        if (!node) return new BSTNode<K>(key);
        if (key < node->key)
            node->left  = insertImpl(node->left, key);
        else if (key > node->key)
            node->right = insertImpl(node->right, key);
        return node;
    }

    BSTNode<K>* findMin(BSTNode<K>* node) {
        while (node->left) node = node->left;
        return node;
    }

    BSTNode<K>* removeImpl(BSTNode<K>* node, const K& key) {
        if (!node) return nullptr;
        if (key < node->key)       node->left  = removeImpl(node->left, key);
        else if (key > node->key)  node->right = removeImpl(node->right, key);
        else {
            // 找到目标节点
            if (!node->left)  { BSTNode* tmp = node->right; delete node; return tmp; }
            if (!node->right) { BSTNode* tmp = node->left;  delete node; return tmp; }
            // 有两个子节点：用右子树最小值替换
            BSTNode* successor = findMin(node->right);
            node->key = successor->key;
            node->right = removeImpl(node->right, successor->key);
        }
        return node;
    }

    void inorderImpl(BSTNode<K>* node, void (*visit)(const K&)) {
        if (!node) return;
        inorderImpl(node->left, visit);
        visit(node->key);
        inorderImpl(node->right, visit);
    }

public:
    BST() : root(nullptr) {}

    void insert(const K& key) { root = insertImpl(root, key); }
    void remove(const K& key) { root = removeImpl(root, key); }

    bool contains(const K& key) {
        BSTNode<K>* cur = root;
        while (cur) {
            if (key < cur->key)       cur = cur->left;
            else if (key > cur->key)  cur = cur->right;
            else return true;
        }
        return false;
    }

    void inorder(void (*visit)(const K&)) { inorderImpl(root, visit); }
};
```

### 6.4 C++ 标准库对应容器

| 项 | 内容 |
|---|------|
| **容器** | `std::set` / `std::map`（底层为红黑树，非普通 BST） |
| **头文件** | `<set>` / `<map>` |
| **游戏开发注意事项** | |

- **BST 本身在游戏中不直接使用**：因为可能退化为 O(n)。实际使用的是其平衡版本（红黑树、AVL）。
- **理解 BST 是基础**：红黑树、线段树、区间树等高级结构都建立在 BST 概念之上。
- **游戏中的变体应用**：场景对象的有序管理（按距离排序的渲染队列）、BVH（包围盒层次结构）的构建思想与 BST 相关。

---

## 7. 平衡二叉树（AVL、红黑树）

### 7.1 逻辑构造原理图

```
  AVL 树：任意节点的左右子树高度差 ≤ 1

        [8] h=3
       /   \
    [4]     [12] h=2
    /  \    /   \
  [2]  [6][10] [14]
  高度差检查：每个节点 |left_h - right_h| ≤ 1 ✓

  失衡时需要旋转修复（4 种情况）：
  LL 型（右旋）：          RR 型（左旋）：
      [C]                     [A]
     /                       \
   [B]                       [B]
   /                           \
 [A]                           [C]

  右旋(C)：               左旋(A)：
      [B]                     [B]
     / \                     / \
   [A] [C]                 [A] [C]


  红黑树：
  规则：
  1. 每个节点要么红要么黑
  2. 根节点为黑
  3. 红节点的子节点必须为黑（不能有两个连续红节点）
  4. 从任一节点到其所有后代叶子的路径上，黑节点数目相同

            [8B]
           /    \
        [4R]    [12R]
        /  \    /   \
     [2B] [6B][10B][14B]

  黑高 = 2（从根到叶每条路径 2 个黑节点）
```

**原理**：
- **AVL 树**：通过高度差约束 + 旋转操作，保证树始终平衡，查找严格 O(log n)。
- **红黑树**：通过颜色约束 + 旋转/变色，保证最长路径不超过最短路径的 2 倍。插入/删除时旋转次数比 AVL 少。

### 7.2 底层构成拆解

| 基础组件 | AVL | 红黑树 |
|---------|-----|--------|
| 节点结构 | key + left + right + **height** | key + left + right + **color** |
| 平衡维护 | 旋转（LL/RR/LR/RL 四种） | 旋转 + 变色 |
| 底层存储 | 逐节点堆分配 | 逐节点堆分配 |

两者本质上都是 **BST + 自平衡机制**。

### 7.3 伪代码（AVL 旋转核心逻辑）

```cpp
template <typename K>
struct AVLNode {
    K        key;
    AVLNode* left;
    AVLNode* right;
    int      height;
    AVLNode(const K& k) : key(k), left(nullptr), right(nullptr), height(1) {}
};

int height(AVLNode* n) { return n ? n->height : 0; }
int balanceFactor(AVLNode* n) { return n ? height(n->left) - height(n->right) : 0; }
void updateHeight(AVLNode* n) {
    if (n) n->height = 1 + max(height(n->left), height(n->right));
}

// 右旋（LL 型失衡）
AVLNode* rotateRight(AVLNode* y) {
    AVLNode* x  = y->left;
    AVLNode* T2 = x->right;
    x->right = y;
    y->left  = T2;
    updateHeight(y);
    updateHeight(x);
    return x;  // 新的子树根
}

// 左旋（RR 型失衡）
AVLNode* rotateLeft(AVLNode* x) {
    AVLNode* y  = x->right;
    AVLNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    updateHeight(x);
    updateHeight(y);
    return y;
}

// 插入后自平衡
AVLNode* insert(AVLNode* node, const K& key) {
    if (!node) return new AVLNode(key);
    if (key < node->key)      node->left  = insert(node->left, key);
    else if (key > node->key) node->right = insert(node->right, key);
    else return node;

    updateHeight(node);
    int bf = balanceFactor(node);

    // LL
    if (bf > 1 && key < node->left->key)
        return rotateRight(node);
    // RR
    if (bf < -1 && key > node->right->key)
        return rotateLeft(node);
    // LR
    if (bf > 1 && key > node->left->key) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    // RL
    if (bf < -1 && key < node->right->key) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }
    return node;
}
```

### 7.4 C++ 标准库对应容器

| 项 | 内容 |
|---|------|
| **容器** | `std::map` / `std::set` / `std::multimap`（底层为红黑树） |
| **头文件** | `<map>` / `<set>` |
| **游戏开发注意事项** | |

- **红黑树是 C++ 标准库的选择**：`std::map` 底层就是红黑树，保证 O(log n) 操作。
- **内存碎片**：逐节点分配，与 `std::list` 类似，碎片问题严重。游戏中不推荐直接使用。
- **AVL vs 红黑树**：AVL 查找更快（更严格平衡），但插入/删除旋转更多。游戏中如果需要频繁插入删除的有序容器，红黑树更合适。
- **游戏中的应用**：`std::map` 在**非运行时关键路径**可以使用（如编辑器工具、资源加载配置表）。运行时高频场景应使用哈希表或排序数组替代。
- **UE 中的替代**：UE 的 `TMap` 使用开放寻址哈希表而非红黑树，`TSortedMap` 使用排序数组。

---

## 8. 堆（大顶堆、小顶堆）

### 8.1 逻辑构造原理图

```
  堆的本质：用数组模拟完全二叉树

  数组表示（小顶堆示例）：
  index:  0   1   2   3   4   5   6
  value: [1] [3] [2] [6] [4] [5] [7]

  对应的完全二叉树：
              [1]           ◄── 堆顶（最小值）
            /     \
          [3]     [2]
          / \     / \
        [6] [4] [5] [7]

  父子关系公式（0-indexed）：
    parent(i)      = (i - 1) / 2
    left_child(i)  = 2 * i + 1
    right_child(i) = 2 * i + 2

  大顶堆 vs 小顶堆：
  大顶堆：父 ≥ 子，堆顶是最大值
  小顶堆：父 ≤ 子，堆顶是最小值

  sift_up（插入后上浮）：
  插入 0 到小顶堆：
              [1]                 [1]
            /     \             /     \
          [3]     [2]   →    [3]     [2]
          / \     /          / \     /
        [6] [4] [5]        [6] [4] [5]
                                       \
                                       ... 先放末尾
        然后 0 不断与父节点交换上浮：
              [1]
            /     \
          [3]     [0]  ◄── 上浮中
          / \     /
        [6] [4] [5]
              ...
              [0]           ◄── 最终堆顶
            /     \
          [3]     [1]
          / \     / \
        [6] [4] [5] [2]

  sift_down（删除堆顶后下沉）：
  删除堆顶 0 后，末尾元素 7 放到堆顶，然后下沉：
              [7]                [2]          ◄── 与较小孩子交换
            /     \            /     \
          [3]     [2]   →    [3]     [7]
          / \     /          / \     /
        [6] [4] [5]        [6] [4] [5]
```

**原理**：堆是一棵完全二叉树，用数组存储。通过 `sift_up` 和 `sift_down` 操作维护堆序性。插入 O(log n)，取堆顶 O(1)，删除堆顶 O(log n)。

### 8.2 底层构成拆解

| 基础组件 | 用途 |
|---------|------|
| 动态数组（Vector） | 存储元素，连续内存 |
| 索引计算 | 通过 `2i+1`、`2i+2`、`(i-1)/2` 模拟树结构 |

本质上 = **一个数组 + 父子索引关系 + sift 操作**。不需要指针，纯算术关系。

### 8.3 伪代码

```cpp
template <typename T>
class MinHeap {
    T*     data_;
    size_t size_;
    size_t capacity_;

    size_t parent(size_t i) { return (i - 1) / 2; }
    size_t left(size_t i)   { return 2 * i + 1; }
    size_t right(size_t i)  { return 2 * i + 2; }

    void swap(size_t i, size_t j) {
        T tmp = data_[i]; data_[i] = data_[j]; data_[j] = tmp;
    }

    void siftUp(size_t i) {
        while (i > 0 && data_[parent(i)] > data_[i]) {
            swap(parent(i), i);
            i = parent(i);
        }
    }

    void siftDown(size_t i) {
        while (left(i) < size_) {
            size_t smallest = i;
            if (left(i) < size_ && data_[left(i)] < data_[smallest])
                smallest = left(i);
            if (right(i) < size_ && data_[right(i)] < data_[smallest])
                smallest = right(i);
            if (smallest == i) break;
            swap(i, smallest);
            i = smallest;
        }
    }

public:
    MinHeap(size_t cap = 16) : size_(0), capacity_(cap) {
        data_ = new T[capacity_];
    }

    void insert(const T& val) {
        if (size_ >= capacity_) { /* 扩容 */ }
        data_[size_++] = val;
        siftUp(size_ - 1);
    }

    T extractMin() {
        T min_val = data_[0];
        data_[0] = data_[--size_];
        siftDown(0);
        return min_val;
    }

    T& peekMin() { return data_[0]; }
    bool empty() const { return size_ == 0; }
    size_t count() const { return size_; }
};
```

### 8.4 C++ 标准库对应容器

| 项 | 内容 |
|---|------|
| **工具** | `std::priority_queue`（默认大顶堆） |
| **头文件** | `<queue>` |
| **底层算法** | `std::make_heap` / `std::push_heap` / `std::pop_heap`（`<algorithm>`） |
| **游戏开发注意事项** | |

- **优先队列在游戏中极常用**：A* 寻路的 open list、任务调度优先级、LOD 距离排序、事件优先级。
- **缓存友好**：数组实现，连续内存，比树结构缓存友好得多。
- **std::priority_queue 默认大顶堆**：需要小顶堆时写 `std::priority_queue<T, vector<T>, greater<T>>`。
- **A* 寻路注意**：open list 用小顶堆（取 f 值最小节点），但需要支持 `decrease_key` 操作。`std::priority_queue` 不支持，通常需要允许重复入队 + 懒惰删除。
- **Top-K 问题**：用堆解决"距离玩家最近的 N 个敌人"等问题，时间复杂度 O(n log k)。

---

## 9. 图（邻接矩阵、邻接表）

### 9.1 逻辑构造原理图

```
  游戏场景中的导航图示例：

  逻辑图结构（有向带权图）：
        ──10──►
    [A] ──────── [B]
     │  ◄────────  │
   5 │    ──3──►   │ 7
     ▼             ▼
    [C] ──────── [D]
        ◄──12──
     ──8──►

  邻接矩阵（适合稠密图）：
      A   B   C   D
  A [ 0  10   5   ∞ ]
  B [ ∞   0   ∞   7 ]
  C [ ∞   3   0  12 ]
  D [ ∞   ∞   8   0 ]

  空间：O(V²)，查找边 O(1)

  邻接表（适合稀疏图）：
  ┌─────────────────────────────┐
  │ adj[] (数组，每个元素是链表)  │
  │  [A] ──► (B,10) ──► (C,5)  │
  │  [B] ──► (D,7)              │
  │  [C] ──► (B,3) ──► (D,12)  │
  │  [D] ──► (C,8)              │
  └─────────────────────────────┘

  空间：O(V + E)，查找边 O(出度)

  游戏寻路中的实际图结构（NavMesh 简化）：
  ┌────────────────────────────────────┐
  │  游戏场景                            │
  │                                     │
  │   [WP0]───[WP1]───[WP2]            │
  │     │       │       │              │
  │   [WP3]───[WP4]───[WP5]───[WP6]    │
  │     │               │              │
  │   [WP7]───[WP8]───[WP9]            │
  │                                     │
  │  WP = Waypoint (航点/路径点)         │
  │  边权 = 两点距离                      │
  └────────────────────────────────────┘
```

**原理**：
- **邻接矩阵**：V×V 的二维数组，`matrix[i][j]` 表示顶点 i 到 j 的边权。适合稠密图。
- **邻接表**：每个顶点维护一个邻居列表。适合稀疏图（游戏场景通常是稀疏图）。

### 9.2 底层构成拆解

| 实现方式 | 底层结构 | 适用场景 |
|---------|---------|---------|
| 邻接矩阵 | 二维数组 `float[V][V]` | 稠密图，顶点数少（<1000），需要 O(1) 查边 |
| 邻接表 | `Vector< Vector<Edge> >` 或 `Vector< List<Edge> >` | 稀疏图，游戏寻路常用 |
| 前向星 | 排序后的边数组 + 偏移索引 | 静态图，内存紧凑 |

### 9.3 伪代码（邻接表 + Dijkstra 寻路）

```cpp
struct Edge {
    int   to;
    float weight;
};

class Graph {
    int num_vertices;
    // 邻接表：vector of vectors（游戏推荐写法）
    Vector<Vector<Edge>> adj;

public:
    Graph(int n) : num_vertices(n), adj(n) {}

    void addEdge(int from, int to, float weight) {
        adj[from].push_back({to, weight});
    }

    // Dijkstra 最短路径（A* 的基础）
    // 返回从 start 到所有点的最短距离
    Vector<float> dijkstra(int start) {
        Vector<float> dist(num_vertices, INFINITY);
        dist[start] = 0.0f;

        // 小顶堆：(distance, vertex)
        // pair.first = distance, pair.second = vertex_id
        MinHeap<Pair<float, int>> pq;
        pq.insert({0.0f, start});

        while (!pq.empty()) {
            auto [d, u] = pq.extractMin();
            if (d > dist[u]) continue;  // 懒惰删除：跳过过时条目

            for (const Edge& e : adj[u]) {
                float new_dist = dist[u] + e.weight;
                if (new_dist < dist[e.to]) {
                    dist[e.to] = new_dist;
                    pq.insert({new_dist, e.to});
                }
            }
        }
        return dist;
    }
};
```

### 9.4 C++ 标准库对应容器

| 项 | 内容 |
|---|------|
| **容器** | 无直接对应，通常用 `std::vector<std::vector<Edge>>` 手动构建 |
| **游戏开发注意事项** | |

- **寻路是游戏客户端核心**：A*、Dijkstra、JPS 等寻路算法都建立在图结构之上。
- **邻接表用 vector of vector**：不要用 `list`，缓存友好性差距巨大。
- **NavMesh 与 waypoint graph**：3A 游戏用 NavMesh（连续空间），2D/手游常用 waypoint graph（离散图）。
- **动态图**：场景物件变化（开门/关门、破坏地形）需要动态更新图结构，考虑增量更新而非重建。
- **内存布局**：大型开放世界的图可能很大，考虑分块加载（只加载当前区域及相邻区域的图数据）。

---

## 10. 并查集

### 10.1 逻辑构造原理图

```
  并查集 (Union-Find / Disjoint Set)：
  管理一组不相交的集合，支持快速合并和查询。

  初始状态（每个元素自成一个集合）：
  parent: [0, 1, 2, 3, 4]
  rank:   [0, 0, 0, 0, 0]

  集合 0    集合 1    集合 2    集合 3    集合 4
   {0}      {1}      {2}      {3}      {4}

  union(0, 1) 后：
       [1]         ← root
      /
    [0]           parent[0] = 1
  集合 {0, 1}     集合 2     集合 3     集合 4

  union(2, 3), union(0, 3) 后：
       [1]              [3]
      / \               /
    [0] [3]    →     [2]
          \
          ... 合并后
       [1]
      / | \
    [0][3] [4]? 不，union(0,3) 合并 {0,1} 和 {2,3}
    
    实际结果（按秩合并）：
         [1]
        /   \
      [0]   [3]
            /
          [2]

  find(2) 的路径压缩：
  查找 2 的根：2 → 3 → 1
  路径压缩后：parent[2] = 1, parent[3] = 1
         [1]
        / | \
      [0][3][2]    ← 所有节点直接指向根

  游戏场景：判断两个游戏对象是否在同一阵营/编组
  ┌─────────────────────────────────────┐
  │  编组 A: {战士, 法师, 弓箭手}         │
  │  编组 B: {骑士, 刺客}                │
  │  合并（骑士加入编组A）→ union(骑士, 战士) │
  │  查询（法师和弓箭手同组？）→ find == find │
  └─────────────────────────────────────┘
```

**原理**：用数组表示森林（每棵树是一个集合）。`find` 找根节点（带路径压缩），`union` 合并两棵树（带按秩合并）。两个优化叠加后，单次操作均摊接近 O(1)。

### 10.2 底层构成拆解

| 基础组件 | 用途 |
|---------|------|
| 整型数组 `parent[]` | `parent[i]` 存储元素 i 的父节点索引 |
| 整型数组 `rank[]` | 按秩合并优化，记录树的近似高度 |

本质上 = **两个平行数组**，极其轻量。

### 10.3 伪代码

```cpp
class UnionFind {
    int*  parent;
    int*  rank;
    int   count;   // 当前集合数量

public:
    UnionFind(int n) : count(n) {
        parent = new int[n];
        rank   = new int[n];
        for (int i = 0; i < n; ++i) {
            parent[i] = i;   // 初始：自己是自己的根
            rank[i]   = 0;
        }
    }

    ~UnionFind() { delete[] parent; delete[] rank; }

    // 查找根节点（带路径压缩）
    int find(int x) {
        if (parent[x] != x)
            parent[x] = find(parent[x]);  // 路径压缩
        return parent[x];
    }

    // 合并两个集合（按秩合并）
    void unite(int x, int y) {
        int rootX = find(x);
        int rootY = find(y);
        if (rootX == rootY) return;  // 已在同一集合

        if (rank[rootX] < rank[rootY]) {
            parent[rootX] = rootY;
        } else if (rank[rootX] > rank[rootY]) {
            parent[rootY] = rootX;
        } else {
            parent[rootY] = rootX;
            rank[rootX]++;
        }
        --count;
    }

    // 判断是否在同一集合
    bool connected(int x, int y) {
        return find(x) == find(y);
    }

    int getCount() const { return count; }
};
```

### 10.4 C++ 标准库对应容器

| 项 | 内容 |
|---|------|
| **容器** | 无直接对应，需手写实现（代码量极小） |
| **游戏开发注意事项** | |

- **游戏中的典型应用**：阵营判断、编组系统、连通区域检测（地图区域连通性）、地图战争迷雾区域合并。
- **极其轻量**：只需要两个 `int` 数组，内存开销可忽略。
- **性能极高**：路径压缩 + 按秩合并后，单次操作均摊 O(α(n)) ≈ O(1)。
- **不适用场景**：需要支持分割（split）的场景。并查集只能合并不能拆分。

---

## 11. 前缀树 Trie

### 11.1 逻辑构造原理图

```
  Trie（前缀树 / 字典树）：
  存储一组字符串，共享前缀路径。

  存储词汇：{"hero", "help", "heap", "hp", "hud"}

            [root]
           / |    \
          h  |     |  (其他字母分支省略)
          |       |
         [h]      |
        / | \     |
       e  |  \    |
      /|  |   |   |
     / |  |   |   |
   [he] |  |   |  |
   /  \ |  |   |  |
  r    l |  |   |  |
  |    | |  |   |  |
 [her] | |  |   |  |
  |    | |  |   |  |
 [hero]│ |  |   |  |
  *(end)| |  |   |  |
       [hel] |  |  |
        *(end)|  |
             ... (a 分支)
            [hela] → 不对，是 help
       [hel]
       /    \
      p      ...
      |
    [help]*

  简化版（只展示 hero, help, heap, hp, hud）：
            root
             |
            [h]
           / | \
          e  p   u
         /|  |   |
        r l  |   d
        | |  a   |
        | |  |  [hud]*
       [hero]* |
             [hea]
               |
             [heap]*
        [hel]
          |
        [help]*

  查找 "help"：
  root → h ✓ → e ✓ → l ✓ → p ✓ → isEnd=true → 找到！

  查找 "heal"：
  root → h ✓ → e ✓ → a ✗ (不存在) → 未找到

  前缀匹配 "he"：
  root → h ✓ → e ✓ → 以此为前缀的所有词：hero, help, heap
```

**原理**：每个节点代表一个字符，从根到标记节点的路径构成一个完整字符串。共享前缀的字符串共享路径，节省存储并加速前缀查询。

### 11.2 底层构成拆解

| 基础组件 | 用途 |
|---------|------|
| 节点结构 | 存储字符 + 子节点数组/哈希表 + isEnd 标记 |
| 子节点容器 | `map<char, Node*>` 或固定大小数组 `Node* children[26]`（小写字母） |
| 根节点 | 空字符，所有字符串的起点 |

游戏中常用**固定大小数组**实现子节点（字符集有限时），避免哈希表开销。

### 11.3 伪代码

```cpp
class Trie {
    struct TrieNode {
        // 假设只处理 a-z，用固定大小数组
        TrieNode* children[26];
        bool      isEnd;
        TrieNode() : isEnd(false) {
            for (int i = 0; i < 26; ++i) children[i] = nullptr;
        }
    };

    TrieNode* root;

    // 释放整棵树的辅助函数
    void destroy(TrieNode* node) {
        if (!node) return;
        for (int i = 0; i < 26; ++i) destroy(node->children[i]);
        delete node;
    }

public:
    Trie() : root(new TrieNode()) {}
    ~Trie() { destroy(root); }

    void insert(const char* word) {
        TrieNode* cur = root;
        for (int i = 0; word[i] != '\0'; ++i) {
            int idx = word[i] - 'a';
            if (!cur->children[idx])
                cur->children[idx] = new TrieNode();
            cur = cur->children[idx];
        }
        cur->isEnd = true;
    }

    bool search(const char* word) {
        TrieNode* cur = root;
        for (int i = 0; word[i] != '\0'; ++i) {
            int idx = word[i] - 'a';
            if (!cur->children[idx]) return false;
            cur = cur->children[idx];
        }
        return cur->isEnd;
    }

    bool startsWith(const char* prefix) {
        TrieNode* cur = root;
        for (int i = 0; prefix[i] != '\0'; ++i) {
            int idx = prefix[i] - 'a';
            if (!cur->children[idx]) return false;
            cur = cur->children[idx];
        }
        return true;  // 前缀存在即可
    }
};
```

### 11.4 C++ 标准库对应容器

| 项 | 内容 |
|---|------|
| **容器** | 无直接对应 |
| **游戏开发注意事项** | |

- **游戏中的典型应用**：
  - **聊天敏感词过滤**：将敏感词库构建为 Trie，对玩家输入做前缀匹配。
  - **命令/技能名自动补全**：GM 控制台输入前缀自动匹配命令。
  - **本地化文本查找**：按前缀快速检索本地化 key。
- **内存优化**：大量小节点会导致内存碎片。游戏中可用**内存池**分配 TrieNode，或改用双数组 Trie（Double-Array Trie）压缩存储。
- **不适用场景**：精确匹配单个字符串（哈希表更快）。Trie 的优势在于前缀匹配。

---

## 12. 空间划分结构：AABB、四叉树、八叉树、KD 树

### 12.1 逻辑构造原理图

```
  AABB 包围盒 (Axis-Aligned Bounding Box)：
  ┌──────────────────────────┐
  │     ┌────────┐           │
  │     │ Object │           │
  │     │        │           │
  │     └────────┘           │
  │  min=(x1,y1)  max=(x2,y2)│
  └──────────────────────────┘

  AABB 表示：{ min: (x1, y1, z1), max: (x2, y2, z2) }
  碰撞检测：两个 AABB 在各轴上都有重叠 → 碰撞
  overlap = (A.min.x <= B.max.x && A.max.x >= B.min.x) &&
            (A.min.y <= B.max.y && A.max.y >= B.min.y) &&
            (A.min.z <= B.max.z && A.max.z >= B.min.z)

  ──────────────────────────────────────────────────────

  四叉树 (Quadtree) - 2D 空间划分：
  ┌──────────────┬──────────────┐
  │              │              │
  │    NW 区     │    NE 区     │
  │              │              │
  │              │              │
  ├──────────────┼──────────────┤
  │              │              │
  │    SW 区     │    SE 区     │
  │              │              │
  │              │              │
  └──────────────┴──────────────┘

  每个节点最多容纳 N 个对象，超过则分裂为 4 个子节点：

  游戏场景示例（检测视野内敌人）：
  ┌──────────────────────────────────────┐
  │          游戏世界 (2D)                │
  │   ┌──────┬──────┐                    │
  │   │ E    │   E  │                    │
  │   ├──────┼──────┤                    │
  │   │  E   │ P(*) │  P 的视野范围       │
  │   │      │  ◉   │  → 只需查询 SE 区   │
  │   └──────┴──────┘    + 相邻区域       │
  └──────────────────────────────────────┘

  ──────────────────────────────────────────────────────

  八叉树 (Octree) - 3D 空间划分：
  四叉树的 3D 版本，每个节点分裂为 8 个子节点。

       ┌──────┬──────┐
      /│      /│     │
     ┌──────┬──────┐ │
     │ │     │ │    │ │
     │ └─────┼─┼────┘
     │/      │/     │/
     └──────┬──────┘

  8 个子节点：
  Top-Bottom × Left-Right × Front-Back = 2×2×2 = 8

  ──────────────────────────────────────────────────────

  KD 树 (K-Dimensional Tree)：
  交替沿不同轴切割空间：

  第 1 层：沿 X 轴切割
  ┌──────┬──────┐
  │      │      │
  │  左   │  右   │
  │      │      │
  └──────┴──────┘
  第 2 层：沿 Y 轴切割
  ┌──────┬──────┐
  │  左上 │ 右上  │
  ├──────┼──────┤
  │  左下 │ 右下  │
  └──────┴──────┘
  第 3 层：沿 Z 轴切割（3D）...

  KD 树节点：
  struct KDNode {
      Vec3  split_point;   // 分割点
      int   split_axis;    // 分割轴 (0=X, 1=Y, 2=Z)
      KDNode* left;
      KDNode* right;
      // 叶子节点存储对象列表
  };
```

**原理**：
- **AABB**：最基础的包围盒，用平行于坐标轴的最小/最大值框住物体。碰撞检测的第一步（宽相检测）。
- **四叉树**：2D 空间递归四等分，适合 2D 游戏或 3D 游戏的俯视图场景管理。
- **八叉树**：3D 版四叉树，适合 3D 场景的空间划分。
- **KD 树**：交替沿不同轴切割，适合静态场景的最近邻查询和碰撞检测。

### 12.2 底层构成拆解

| 结构 | 底层实现 | 特点 |
|------|---------|------|
| AABB | 两个 Vec3（min, max） | 最轻量，碰撞宽相必备 |
| 四叉树 | 树节点 + 对象列表 | 每个节点 4 个子节点，2D 空间 |
| 八叉树 | 树节点 + 对象列表 | 每个节点 8 个子节点，3D 空间 |
| KD 树 | 二叉树 + 分割轴信息 | 交替轴切割，适合静态场景 |

### 12.3 伪代码（四叉树）

```cpp
struct Vec2 { float x, y; };

struct AABB2D {
    Vec2 min, max;
    bool contains(Vec2 p) {
        return p.x >= min.x && p.x <= max.x &&
               p.y >= min.y && p.y <= max.y;
    }
    bool intersects(AABB2D other) {
        return !(other.min.x > max.x || other.max.x < min.x ||
                 other.min.y > max.y || other.max.y < min.y);
    }
};

struct GameObject {
    Vec2 position;
    int  id;
};

class Quadtree {
    static const int MAX_OBJECTS = 8;
    static const int MAX_DEPTH   = 5;

    AABB2D          bounds;
    int             level;
    GameObject*     objects[32];  // 固定数组，避免动态分配
    int             object_count;
    Quadtree*       children[4];  // NW, NE, SW, SE

    void split() {
        float sub_w = bounds.max.x - bounds.min.x;
        float sub_h = bounds.max.y - bounds.min.y;
        float half_w = sub_w / 2.0f;
        float half_h = sub_h / 2.0f;
        Vec2 min = bounds.min;

        children[0] = new Quadtree({min.x,       min.y},       {half_w, half_h}, level+1); // NW
        children[1] = new Quadtree({min.x+half_w, min.y},       {half_w, half_h}, level+1); // NE
        children[2] = new Quadtree({min.x,       min.y+half_h}, {half_w, half_h}, level+1); // SW
        children[3] = new Quadtree({min.x+half_w, min.y+half_h}, {half_w, half_h}, level+1); // SE
    }

    int getChildIndex(Vec2 pos) {
        float mid_x = (bounds.min.x + bounds.max.x) / 2.0f;
        float mid_y = (bounds.min.y + bounds.max.y) / 2.0f;
        if (pos.x <= mid_x) {
            return (pos.y <= mid_y) ? 0 : 2;  // NW or SW
        } else {
            return (pos.y <= mid_y) ? 1 : 3;  // NE or SE
        }
    }

public:
    Quadtree(Vec2 min, Vec2 max, int lev = 0)
        : bounds({min, max}), level(lev), object_count(0) {
        for (int i = 0; i < 4; ++i) children[i] = nullptr;
    }

    void insert(GameObject* obj) {
        if (children[0] != nullptr) {
            int idx = getChildIndex(obj->position);
            children[idx]->insert(obj);
            return;
        }
        objects[object_count++] = obj;
        if (object_count >= MAX_OBJECTS && level < MAX_DEPTH) {
            split();
            for (int i = 0; i < object_count; ++i) {
                int idx = getChildIndex(objects[i]->position);
                children[idx]->insert(objects[i]);
            }
            object_count = 0;
        }
    }

    // 查询范围内的所有对象（用于视野裁剪、碰撞检测）
    void query(AABB2D range, Vector<GameObject*>& result) {
        if (!bounds.intersects(range)) return;

        for (int i = 0; i < object_count; ++i) {
            if (range.contains(objects[i]->position))
                result.push_back(objects[i]);
        }

        if (children[0] != nullptr) {
            for (int i = 0; i < 4; ++i)
                children[i]->query(range, result);
        }
    }
};
```

### 12.4 C++ 标准库对应容器

| 项 | 内容 |
|---|------|
| **容器** | 无直接对应，需手写 |
| **游戏开发注意事项** | |

- **四叉树/八叉树是场景管理的核心**：用于视锥裁剪（frustum culling）、碰撞宽相检测、空间查询（"找出玩家周围 10m 内的所有敌人"）。
- **AABB 是碰撞检测第一步**：所有引擎的碰撞管线都从 AABB 重叠测试开始（宽相），再进入精确检测（窄相）。
- **KD 树适合静态场景**：光照探针分布、静态几何体碰撞查询。不适合频繁移动的对象（需要频繁重建）。
- **动态对象管理**：游戏对象移动后需要更新四叉树/八叉树中的位置。考虑每帧重建（简单但开销大）或增量更新（复杂但高效）。
- **内存分配注意**：节点频繁创建销毁会导致碎片。建议用**对象池**管理四叉树节点。

---

## 13. 跳表

### 13.1 逻辑构造原理图

```
  跳表 (Skip List)：
  多层链表结构，通过"跳跃"实现 O(log n) 查找。

  存储有序序列：{1, 3, 5, 7, 9, 11, 13, 15}

  Level 3:  HEAD ──────────────────────────────────► NIL
            (只有极少数节点出现在最高层)

  Level 2:  HEAD ──────► 3 ──────────► 9 ────► NIL

  Level 1:  HEAD ──► 1 ──► 5 ──► 7 ──► 11 ──► 15 ──► NIL

  Level 0:  HEAD ► 1 ► 3 ► 5 ► 7 ► 9 ► 11 ► 13 ► 15 ► NIL
            (底层是完整有序链表)

  查找 key=11 的路径：
  Level 2: HEAD → 3 (3<11, 继续) → 9 (9<11, 继续) → 下一个是 NIL, 下降
  Level 1: 9 → 下一个是 NIL, 下降
  Level 0: 9 → 11 找到！

  只访问了 4 个节点（3, 9, 9, 11），而不是遍历全部 8 个。

  概率平衡：
  每个节点插入时，通过随机抛硬币决定层数：
  - 50% 概率在第 0 层
  - 25% 概率在第 1 层
  - 12.5% 概率在第 2 层
  - ...

  与红黑树对比：
  ┌────────────┬──────────────┬──────────────┐
  │            │ 红黑树        │ 跳表          │
  ├────────────┼──────────────┼──────────────┤
  │ 查找       │ O(log n)     │ O(log n) 期望 │
  │ 插入       │ O(log n)     │ O(log n) 期望 │
  │ 实现复杂度  │ 高（旋转变色） │ 低（改指针）   │
  │ 并发友好    │ 差           │ 好            │
  │ 缓存友好    │ 差           │ 差（都是链表）  │
  └────────────┴──────────────┴──────────────┘
```

**原理**：在有序链表的基础上，随机抽取部分节点建立多层索引。查找时从最高层开始，能跳就跳，不能跳就下降。期望时间复杂度 O(log n)。

### 13.2 底层构成拆解

| 基础组件 | 用途 |
|---------|------|
| 多层链表 | 每层是一个有序链表，高层是低层的"快速通道" |
| 节点结构 | 存储 key + value + 多层 forward 指针 |
| 随机函数 | 决定新节点的层数（概率平衡） |

本质上 = **有序链表 + 多层索引 + 随机化平衡**。

### 13.3 伪代码

```cpp
class SkipList {
    static const int MAX_LEVEL = 16;
    static const float P = 0.5f;

    struct Node {
        int    key;
        int    value;
        Node*  forward[MAX_LEVEL];  // 每层的前进指针
        int    level;               // 该节点实际所在的最高层
    };

    Node* header;
    int   cur_level;

    int randomLevel() {
        int lvl = 0;
        while ((float)rand() / RAND_MAX < P && lvl < MAX_LEVEL)
            ++lvl;
        return lvl;
    }

public:
    SkipList() : cur_level(0) {
        header = new Node();
        header->level = MAX_LEVEL - 1;
        for (int i = 0; i < MAX_LEVEL; ++i)
            header->forward[i] = nullptr;
    }

    Node* find(int key) {
        Node* cur = header;
        for (int i = cur_level; i >= 0; --i) {
            while (cur->forward[i] && cur->forward[i]->key < key)
                cur = cur->forward[i];
        }
        cur = cur->forward[0];
        if (cur && cur->key == key) return cur;
        return nullptr;
    }

    void insert(int key, int value) {
        Node* update[MAX_LEVEL];
        Node* cur = header;

        for (int i = cur_level; i >= 0; --i) {
            while (cur->forward[i] && cur->forward[i]->key < key)
                cur = cur->forward[i];
            update[i] = cur;
        }

        int lvl = randomLevel();
        if (lvl > cur_level) {
            for (int i = cur_level + 1; i <= lvl; ++i)
                update[i] = header;
            cur_level = lvl;
        }

        Node* newNode = new Node();
        newNode->key   = key;
        newNode->value = value;
        newNode->level = lvl;

        for (int i = 0; i <= lvl; ++i) {
            newNode->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = newNode;
        }
    }

    void erase(int key) {
        Node* update[MAX_LEVEL];
        Node* cur = header;

        for (int i = cur_level; i >= 0; --i) {
            while (cur->forward[i] && cur->forward[i]->key < key)
                cur = cur->forward[i];
            update[i] = cur;
        }

        Node* target = cur->forward[0];
        if (!target || target->key != key) return;

        for (int i = 0; i <= cur_level; ++i) {
            if (update[i]->forward[i] != target) break;
            update[i]->forward[i] = target->forward[i];
        }

        delete target;
        while (cur_level > 0 && !header->forward[cur_level])
            --cur_level;
    }
};
```

### 13.4 C++ 标准库对应容器

| 项 | 内容 |
|---|------|
| **容器** | 无直接对应（`std::map` 底层是红黑树，功能类似但实现不同） |
| **游戏开发注意事项** | |

- **游戏中的潜在应用**：排行榜系统（有序数据 + 快速范围查询）、时间轴管理器（按时间排序的事件）。
- **实际使用较少**：游戏客户端中跳表不如排序数组 + 二分查找常用。跳表的优势在并发场景（如服务端），客户端单线程环境下红黑树或排序数组通常更合适。
- **面试考点**：跳表常作为"红黑树替代方案"被问到，需要理解其概率平衡思想和与红黑树的对比。
- **内存碎片**：与链表类似，逐节点分配，碎片问题严重。

---

# 第一部分总结：游戏客户端考点小结

| 数据结构 | 时间复杂度（查找/插入/删除） | 缓存友好 | 内存碎片 | 游戏核心应用 |
|---------|--------------------------|---------|---------|------------|
| Vector | O(1)/O(1)*O(n)/O(n)*O(n) | 极好 | 低 | 几乎所有场景 |
| 链表 | O(n)/O(1)/O(1) | 极差 | 严重 | 对象池 free-list |
| 栈 | -/O(1)/O(1) | 好 | 低 | 状态机、UI 导航 |
| 队列/循环队列 | -/O(1)/O(1) | 好 | 无 | 消息队列、输入缓冲 |
| 哈希表 | O(1)/O(1)/O(1) | 好(开放寻址) | 看实现 | 资源管理、实体查询 |
| BST | O(log n)/O(log n)/O(log n) | 差 | 严重 | 理论基础上 |
| 红黑树/AVL | O(log n)/O(log n)/O(log n) | 差 | 严重 | 有序容器（非运行时关键路径） |
| 堆 | O(1) peek/O(log n)/O(log n) | 好 | 低 | A* 寻路、优先级调度 |
| 图 | 看实现 | 看实现 | 看实现 | 寻路、AI 决策 |
| 并查集 | O(α(n))≈O(1) | 极好 | 无 | 阵营、编组、连通性 |
| Trie | O(L)/O(L)/O(L) | 一般 | 看实现 | 敏感词、命令补全 |
| 四叉树/八叉树 | O(log n) 查询 | 一般 | 看实现 | 场景管理、碰撞宽相 |
| 跳表 | O(log n)/O(log n)/O(log n) | 差 | 严重 | 排行榜（较少用） |

---

# 第二部分：数据结构基础操作与工程应用

---

## 1. 动态数组 Vector — 基础操作

### 1.1 push_back — 尾部插入

**操作逻辑**：在数组末尾追加一个元素。如果容量不足，先扩容（通常 2 倍），再写入。

**伪代码**：
```cpp
void push_back(T value) {
    if (size_ == capacity_) {
        grow(capacity_ * 2);     // 分配新内存、搬移、释放旧内存
    }
    data_[size_++] = value;
}
```

**游戏适用场景**：
- 每帧收集需要渲染的对象到渲染列表
- 构建粒子系统的活跃粒子列表
- 技能 buff 列表的追加

**不适用**：需要在中间插入的场景（如按优先级排序的列表，应直接插入排序位置或使用堆）。

**性能要点**：
- 均摊 O(1)，但单次扩容是 O(n)
- 游戏循环中应 `reserve()` 预分配，避免帧内扩容卡顿
- 扩容时所有元素 move，对大对象开销显著

---

### 1.2 pop_back — 尾部删除

**操作逻辑**：将 size 减 1，析构末尾元素，不释放内存。

**伪代码**：
```cpp
void pop_back() {
    if (size_ > 0) {
        data_[--size_].~T();
    }
}
```

**游戏适用场景**：
- 栈式资源管理（如场景加载栈）
- 撤销操作的历史记录回退

**不适用**：需要删除中间元素的场景。

---

### 1.3 insert — 指定位置插入

**操作逻辑**：在 index 位置插入元素，需要将 index 之后的所有元素后移一位。

**伪代码**：
```cpp
void insert(size_t index, T value) {
    if (size_ == capacity_) grow(capacity_ * 2);
    for (size_t i = size_; i > index; --i)
        data_[i] = std::move(data_[i - 1]);  // 后移
    data_[index] = value;
    ++size_;
}
```

**游戏适用场景**：
- 按渲染优先级排序的绘制队列（按 depth 插入）
- 回合制游戏中的行动顺序列表（按速度排序插入）

**不适用**：频繁在中间插入的大量数据（应使用链表或 free-list）。

**性能要点**：O(n)，n 为插入点后的元素数。大量元素时开销大。

---

### 1.4 erase — 指定位置删除

**操作逻辑**：删除 index 位置的元素，将之后的元素前移。

**伪代码**：
```cpp
void erase(size_t index) {
    for (size_t i = index; i < size_ - 1; ++i)
        data_[i] = std::move(data_[i + 1]);  // 前移
    --size_;
    data_[size_].~T();
}
```

**游戏适用场景**：
- 移除已销毁的游戏实体（但更好的做法是 swap-and-pop）

**不适用**：频繁删除的场景。

**性能优化 — swap-and-pop**：
```cpp
// 不关心顺序时，O(1) 删除
void eraseUnordered(size_t index) {
    data_[index] = std::move(data_[size_ - 1]);
    --size_;
}
```
游戏中大量使用此技巧：粒子消亡、子弹命中、实体销毁等不关心顺序的场景。

---

### 1.5 reserve / resize — 容量管理

**操作逻辑**：
- `reserve(n)`：预分配至少 n 个元素的容量，不改变 size。
- `resize(n)`：改变 size 到 n，多余元素析构或新增默认构造元素。

**游戏适用场景**：
- 帧开始时 `render_list.reserve(estimated_count)` 避免帧内扩容
- 对象池初始化时 `resize(pool_size)` 预创建所有对象

**性能要点**：
- `reserve` 是游戏开发中**最重要的性能优化手段之一**
- 在已知大致数量的场景下必须使用

---

## 2. 链表 — 基础操作

### 2.1 push_front / push_back — 头部/尾部插入

**操作逻辑**：创建新节点，修改指针将其插入链表头部或尾部。

**伪代码**：
```cpp
void push_front(T value) {
    Node* node = new Node(value);
    node->next = head;
    head = node;
}

void push_back(T value) {
    Node* node = new Node(value);
    if (!tail) { head = tail = node; return; }
    tail->next = node;
    node->prev = tail;
    tail = node;
}
```

**游戏适用场景**：
- 对象池的 free-list：用链表串联空闲槽位
- 事件监听器列表：注册/注销频繁，遍历较少

**不适用**：需要频繁遍历的场景（缓存不友好）。

---

### 2.2 erase — 删除指定节点

**操作逻辑**：已知节点指针时，修改前后节点的指针跳过该节点，O(1) 完成。

**伪代码**：
```cpp
void erase(Node* node) {
    if (node->prev) node->prev->next = node->next;
    else head = node->next;
    if (node->next) node->next->prev = node->prev;
    else tail = node->prev;
    delete node;
}
```

**游戏适用场景**：
- 从事件监听器列表中移除某个监听器
- 从活跃实体列表中移除已消亡的实体

**性能要点**：O(1)（已持有指针时），但 `delete` 操作可能产生内存碎片。游戏中建议用对象池替代 `new/delete`。

---

### 2.3 traverse — 遍历

**操作逻辑**：从 head 开始，沿 next 指针逐个访问。

**伪代码**：
```cpp
void for_each(void (*func)(T&)) {
    Node* cur = head;
    while (cur) {
        func(cur->data);
        cur = cur->next;
    }
}
```

**游戏适用场景**：
- 遍历所有活跃 buff
- 遍历渲染队列

**性能要点**：
- 缓存极不友好，每个节点可能在不同 cache line
- 大数据量时比 vector 遍历慢 5-10 倍
- 游戏中能用 vector 遍历的场景不要用链表

---

## 3. 栈 Stack — 基础操作

### 3.1 push — 入栈

**操作逻辑**：在栈顶压入元素。

**游戏适用场景**：
- UI 界面栈：打开新界面 push，关闭时 pop（如背包→商店→确认弹窗）
- 游戏状态机：主菜单→加载→游戏中→暂停
- 脚本虚拟机：函数调用栈、表达式求值栈

**不适用**：需要随机访问中间元素的场景。

---

### 3.2 pop — 出栈

**操作逻辑**：移除栈顶元素。

**游戏适用场景**：
- 关闭当前 UI 界面，回到上一级
- 退出游戏状态（暂停→游戏中）
- 撤销操作（undo stack）

---

### 3.3 top — 查看栈顶

**操作逻辑**：返回栈顶元素的引用，不弹出。

**游戏适用场景**：
- 获取当前活跃的 UI 界面
- 获取当前游戏状态

---

## 4. 队列 Queue / 循环队列 — 基础操作

### 4.1 enqueue — 入队

**操作逻辑**：在队尾追加元素。循环队列中 tail 索引取模绕回。

**游戏适用场景**：
- **输入事件队列**：键盘/鼠标/手柄事件按序入队，主循环逐帧处理
- **网络包队列**：接收到的网络包入队，游戏逻辑帧中按序处理
- **渲染命令队列**：主线程生成渲染命令入队，渲染线程出队执行
- **动画队列**：多个动画按序排队播放

**不适用**：需要按优先级处理的场景（应使用优先队列/堆）。

**性能要点**：
- 循环队列无内存分配，适合高频场景
- 固定大小的环形缓冲区是游戏中最推荐的实现

---

### 4.2 dequeue — 出队

**操作逻辑**：从队首取出元素。循环队列中 head 索引取模绕回。

**游戏适用场景**：
- 主循环中处理输入事件
- 渲染线程消费渲染命令

---

### 4.3 front / empty / size — 查询操作

**游戏适用场景**：
- 检查消息队列是否为空（决定是否继续处理）
- 获取队列中待处理事件的数量

---

## 5. 哈希表 — 基础操作

### 5.1 insert / put — 插入键值对

**操作逻辑**：计算 key 的哈希值，映射到桶索引，处理冲突后写入。

**游戏适用场景**：
- **资源管理器**：`HashMap<ResourceID, Resource*>` 管理已加载资源
- **实体管理器**：`HashMap<EntityID, Entity*>` 快速查找实体
- **配置表**：`HashMap<string, ConfigEntry>` 存储游戏配置
- **技能系统**：`HashMap<SkillID, SkillData>` 查询技能数据

**不适用**：需要有序遍历的场景（应使用排序数组或树）。

**性能要点**：
- 平均 O(1)，但 rehash 时 O(n)
- 游戏运行期应预分配 `reserve()`，避免 rehash 卡顿
- 哈希函数质量直接影响性能，避免大量冲突

---

### 5.2 find / get — 查找

**操作逻辑**：计算 key 的哈希，定位桶，遍历冲突链/探测序列找到目标。

**游戏适用场景**：
- 每帧根据 ID 查找实体（比遍历 vector 快得多）
- 根据资源路径查找已缓存的资源
- 根据 FName 查找属性值

---

### 5.3 erase — 删除

**操作逻辑**：定位 key 后标记删除（开放寻址法的墓碑）或从链表中摘除（链地址法）。

**游戏适用场景**：
- 实体销毁时从管理器中移除
- 资源卸载时从缓存中删除

**性能要点**：开放寻址法的墓碑会降低后续查找性能，需要定期 rehash 清理。

---

## 6. BST — 基础操作

### 6.1 insert — 插入

**操作逻辑**：从根开始，按 key 大小关系向左或向右走，到达空位时插入。

**游戏适用场景**：
- 理论基础上（实际使用平衡版本）
- 理解线段树、区间树等高级结构的前置知识

**不适用**：直接在游戏中使用（可能退化为 O(n)）。

---

### 6.2 search — 查找

**操作逻辑**：从根开始，按 key 大小关系走，找到目标或到达空位。

**不适用**：同 insert，应使用平衡版本或哈希表。

---

### 6.3 inorder — 中序遍历

**操作逻辑**：递归遍历左子树→根→右子树，得到有序序列。

**游戏适用场景**：
- 需要有序输出时（如排行榜的调试输出）
- 构建 BVH 时的排序步骤

---

## 7. 平衡二叉树 — 基础操作

### 7.1 insert — 插入（自平衡）

**操作逻辑**：先按 BST 规则插入，然后通过旋转（AVL）或旋转+变色（红黑树）恢复平衡。

**游戏适用场景**：
- `std::map` 在非关键路径的使用（如编辑器工具、配置解析）
- 需要有序容器且插入删除频繁的场景

**不适用**：
- 运行时高频场景（内存碎片 + 缓存不友好）
- 只需查找不需有序时（哈希表更快）

**性能要点**：O(log n) 保证，但常数因子大。逐节点分配导致缓存不友好。

---

### 7.2 erase — 删除（自平衡）

**操作逻辑**：按 BST 规则删除，然后通过旋转/变色恢复平衡。

**游戏适用场景**：同 insert。

---

### 7.3 lower_bound / upper_bound — 范围查询

**操作逻辑**：查找第一个 ≥ key 或 > key 的元素。

**游戏适用场景**：
- 查找距离玩家 ≥ D 的最近实体
- 查找时间轴上 ≥ 当前时间的下一个关键帧

---

## 8. 堆 — 基础操作

### 8.1 insert — 插入（sift up）

**操作逻辑**：将新元素放在数组末尾，然后不断与父节点比较交换，上浮到正确位置。

**伪代码**：
```cpp
void insert(T val) {
    data_[size_++] = val;
    siftUp(size_ - 1);   // 上浮
}
```

**游戏适用场景**：
- **A\* 寻路 open list**：每次取 f 值最小的节点扩展
- **任务调度器**：按优先级调度游戏逻辑任务
- **LOD 系统**：按距离排序，优先处理近处物体
- **Top-K 查询**：找出伤害最高的 N 个单位

**不适用**：需要随机访问中间元素的场景。

**性能要点**：
- O(log n) 插入，数组实现缓存友好
- A* 寻路中堆操作是性能瓶颈，需要高效实现
- 允许重复元素的"懒惰堆"比支持 decrease_key 的堆更实用

---

### 8.2 extractMin / extractMax — 取出堆顶

**操作逻辑**：取走堆顶，将末尾元素放到堆顶，sift down 到正确位置。

**游戏适用场景**：
- A* 寻路中取出 f 值最小的节点
- 事件系统中取出最近要触发的事件
- 优先级任务调度中取出最高优先级任务

---

### 8.3 peekMin / peekMax — 查看堆顶

**操作逻辑**：直接返回 `data_[0]`，O(1)。

**游戏适用场景**：
- 检查下一个要处理的事件时间（判断是否需要触发）
- 获取当前最高优先级任务的优先级

---

## 9. 图 — 基础操作

### 9.1 addEdge — 添加边

**操作逻辑**：在邻接表中，向 `adj[from]` 追加一条边记录。

**游戏适用场景**：
- 构建导航图（waypoint graph）
- 构建 AI 行为树的转移关系
- 构建技能链/连招系统

---

### 9.2 BFS — 广度优先搜索

**操作逻辑**：用队列实现，从起点开始逐层扩展。

**伪代码**：
```cpp
void bfs(int start) {
    Queue<int> q;
    bool visited[MAX_V] = {};
    q.enqueue(start);
    visited[start] = true;
    while (!q.empty()) {
        int u = q.dequeue();
        for (Edge e : adj[u]) {
            if (!visited[e.to]) {
                visited[e.to] = true;
                q.enqueue(e.to);
            }
        }
    }
}
```

**游戏适用场景**：
- **无权图最短路径**：网格地图上的最短步数
- **洪水填充**：计算某个区域的可到达范围
- **社交系统**：查找 N 度好友关系

---

### 9.3 Dijkstra / A* — 最短路径

**操作逻辑**：用优先队列（最小堆）实现，每次取距离最小的节点扩展。

**游戏适用场景**：
- **A\* 寻路**：游戏客户端最核心的算法之一
  - 适用：网格地图、NavMesh 上的路径规划
  - 不适用：动态障碍物频繁变化的场景（需要频繁重算）
- **技能伤害传导**：从目标向周围传导伤害，距离衰减

**性能要点**：
- A* 的堆操作是主要开销，需要高效的最小堆实现
- 大型地图上考虑 JPS（Jump Point Search）或 HPA*（分层路径查找）优化
- 分帧计算：长路径的寻路可以分摊到多帧执行，避免单帧卡顿

---

## 10. 并查集 — 基础操作

### 10.1 find — 查找根节点

**操作逻辑**：沿 parent 指针向上走到根节点，同时做路径压缩。

**游戏适用场景**：
- 判断两个单位是否在同一阵营/编组
- 判断地图上两个区域是否连通
- 网络同步中判断两个对象是否属于同一兴趣管理区域

---

### 10.2 unite — 合并集合

**操作逻辑**：找到两个元素的根节点，将一棵树挂到另一棵下（按秩合并）。

**游戏适用场景**：
- 合并两个阵营/编组
- 地图区域连通性更新（如桥梁建好后连接两岸区域）
- Kruskal 最小生成树算法（用于程序化地图生成）

---

### 10.3 connected — 判断连通

**操作逻辑**：`find(x) == find(y)`。

**游戏适用场景**：
- 判断两个玩家是否在同一队伍
- 判断两个地图区域是否可达

**性能要点**：近乎 O(1)，是最高效的数据结构操作之一。

---

## 11. Trie — 基础操作

### 11.1 insert — 插入字符串

**操作逻辑**：从根开始，逐字符沿对应分支走，不存在则创建新节点，末尾标记 isEnd。

**游戏适用场景**：
- 构建敏感词库
- 构建 GM 命令字典
- 构建技能/物品名称索引

---

### 11.2 search — 精确查找

**操作逻辑**：从根开始，逐字符走，到达末尾时检查 isEnd 标记。

**游戏适用场景**：
- 检查玩家输入是否为有效命令
- 检查字符串是否在字典中

---

### 11.3 startsWith — 前缀查找

**操作逻辑**：从根开始，逐字符走，能走完整个前缀即返回 true。

**游戏适用场景**：
- **聊天敏感词过滤**：检测消息中是否包含以某前缀开头的敏感词
- **GM 命令自动补全**：输入 `/kill` 自动匹配 `/kill_all`、`/kill_target` 等
- **搜索建议**：商城搜索框的自动补全

---

## 12. 空间划分结构 — 基础操作

### 12.1 AABB 碰撞检测

**操作逻辑**：检查两个 AABB 在所有轴上是否都有重叠。

**伪代码**：
```cpp
bool testAABB(AABB a, AABB b) {
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
           (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
           (a.min.z <= b.max.z && a.max.z >= b.min.z);
}
```

**游戏适用场景**：
- **碰撞宽相检测**：所有碰撞管线的第一步
- **视锥裁剪**：判断物体是否在相机视野内
- **光照剔除**：判断光源是否影响某个物体

**不适用**：精确碰撞检测（需要 OBB、凸包、GJK 等窄相算法）。

---

### 12.2 四叉树 insert — 插入对象

**操作逻辑**：判断对象属于哪个子节点区域，递归插入。如果当前节点对象数超过阈值且未达最大深度，则分裂。

**游戏适用场景**：
- 2D 游戏中管理所有可碰撞物体
- RTS 游戏中管理大量单位的空间查询

---

### 12.3 四叉树 query — 范围查询

**操作逻辑**：递归检查每个子节点的 bounds 是否与查询范围相交，只进入相交的子节点。

**游戏适用场景**：
- **AOE 技能**：查找技能范围内的所有目标
- **视野管理**：查找玩家视野内的所有实体
- **碰撞检测**：只检测空间上接近的物体对

**性能要点**：
- 将 O(n²) 的暴力碰撞检测降低到 O(n log n)
- 动态对象需要每帧更新树结构，考虑增量更新
- 节点分配建议使用对象池

---

### 12.4 KD 树 nearest — 最近邻查询

**操作逻辑**：从根开始，根据分割轴向下走，到达叶子后回溯检查另一侧是否有更近的点。

**游戏适用场景**：
- **静态场景最近邻查询**：查找离玩家最近的掩体、道具
- **光照探针查询**：找到离当前点最近的光照探针
- **程序化生成**：Poission Disk 采样中的最近邻检查

**不适用**：频繁变化的动态对象集合（重建开销大）。

---

## 13. 跳表 — 基础操作

### 13.1 insert — 插入

**操作逻辑**：找到每层的插入位置，随机决定新节点层数，在每层修改指针。

**游戏适用场景**：
- 排行榜系统（有序 + 快速插入/删除）
- 时间轴事件管理

**不适用**：
- 大多数场景下排序数组 + 二分查找更合适（缓存友好）
- 数据量小时（<1000）直接用 vector 即可

---

### 13.2 find — 查找

**操作逻辑**：从最高层开始，能跳就跳，不能跳就下降到低层。

**游戏适用场景**：
- 排行榜中查找某个玩家的排名
- 时间轴中查找某个时间点的事件

---

### 13.3 range_query — 范围查询

**操作逻辑**：先找到范围起点，然后沿底层链表顺序遍历到范围终点。

**游戏适用场景**：
- 查询排行榜第 M 名到第 N 名的玩家
- 查询时间 T1 到 T2 之间的所有事件

---

# 第二部分总结：操作 × 场景速查表

| 操作 | Vector | 链表 | 栈 | 队列 | 哈希表 | 堆 | 图 | 并查集 | Trie | 空间划分 |
|------|--------|------|-----|------|--------|-----|-----|--------|------|---------|
| 尾部插入 | push_back ✓ | push_back ✓ | push ✓ | enqueue ✓ | insert ✓ | insert ✓ | addEdge | unite ✓ | insert ✓ | insert ✓ |
| 尾部删除 | pop_back ✓ | pop_back ✓ | pop ✓ | dequeue ✓ | erase ✓ | extractMin ✓ | - | - | - | - |
| 随机访问 | O(1) ✓ | O(n) | - | - | O(1) ✓ | O(1) peek | - | - | - | - |
| 中间插入 | O(n) | O(1)* | - | - | - | - | - | - | - | - |
| 范围查询 | O(n) | O(n) | - | - | - | - | BFS/DFS | connected | startsWith ✓ | query ✓ |
| 有序遍历 | 排序后 ✓ | 天然有序 | - | - | 不支持 | 有序出队 | - | - | DFS ✓ | - |
| 最近邻 | - | - | - | - | - | - | Dijkstra | - | - | KD树 ✓ |

*注：✓ 表示该操作在此数据结构上高效或常用；* 表示需要持有节点指针。

---

# 附录：游戏客户端面试高频考点速记

## 内存与缓存

1. **Vector 为什么是游戏中最常用的容器？**
   连续内存 → cache line 命中率高 → 遍历/随机访问极快。

2. **std::list 为什么不推荐在游戏中使用？**
   逐节点分配 → 内存碎片 + cache miss → 遍历性能远差于 vector。

3. **什么时候用 vector，什么时候用链表？**
   99% 的场景用 vector。只有"频繁在中间插入删除 + 不频繁遍历"的场景考虑链表（或 free-list）。

## 扩容与预分配

4. **vector 的扩容机制？为什么是 1.5x 或 2x？**
   2x 保证每次扩容后可以容纳之前所有元素的总和（几何级数），1.5x 更节省内存但扩容次数更多。

5. **游戏中如何避免 vector 扩容？**
   `reserve()` 预分配。在帧开始时估算最大数量并 reserve。

## 哈希表

6. **开放寻址 vs 链地址法，游戏引擎选哪个？**
   开放寻址。扁平数组 → cache 友好，无额外节点分配。UE 的 TMap 就是开放寻址。

7. **哈希冲突的解决方案？**
   链地址法（链表）、开放寻址（线性探测/二次探测）、再哈希。

## 堆与寻路

8. **A\* 寻路为什么用最小堆？**
   每次需要取 f 值最小的节点，最小堆的 extractMin 是 O(log n)。

9. **std::priority_queue 不支持 decrease_key 怎么办？**
   允许重复入队 + 懒惰删除（出队时检查是否为最新版本）。

## 空间划分

10. **四叉树/八叉树在游戏中的用途？**
    场景管理、碰撞宽相、视锥裁剪、空间查询（"找出范围内的所有敌人"）。

11. **AABB 碰撞检测的原理？**
    分离轴定理的特例：两个 AABB 在所有轴上都有重叠 → 碰撞。

## 并查集

12. **并查集的两个优化？**
    路径压缩 + 按秩合并。叠加后均摊 O(α(n)) ≈ O(1)。

---

> **手册结语**：数据结构的选择本质上是在**时间复杂度、空间复杂度、缓存友好性、内存碎片**四个维度之间做权衡。游戏客户端开发的特殊之处在于：**缓存友好性和内存连续性**的优先级远高于学术教材中的强调程度。一个 O(log n) 但缓存友好的结构（如数组上的二分查找），在游戏中往往比 O(1) 但缓存不友好的结构（如哈希表的链表冲突）更快。始终从实际场景出发，而非从理论最优出发。
