# Memory_benchmark总结

## 一、 核心知识点全景解析

### 1. 内存管理与底层机制

•**malloc / free** (C语言内存API)：直接向操作系统申请和释放内存。系统调用开销极大，是内存池要解决的核心痛点。

•**new char[] / delete[]** (C++内存API)：new 在底层其实也是调用了 malloc，但还额外做了两件事：计算内存大小、调用构造函数。在这里用 new char[] 是为了测试 C++ 原生分配的性能。

•**::** (全局作用域解析符)：例如 ::malloc。明确告诉编译器去全局命名空间找 malloc，防止被类内部可能存在的同名函数干扰。

### 2. 数据结构与指针运算

•**单向链表** (Singly Linked List)：代码中的 struct Block { Block* next; }; 是内存池的灵魂。它利用空闲内存块本身来存储下一个空闲块的地址（即“嵌入指针”），实现了 0 额外空间开销的链表管理。

•**指针算术运算** (Pointer Arithmetic)：newChunk + i。因为 newChunk 是 char*（占1字节），所以 + i 就是精确地向后移动 i 个字节。这是将大块内存“切分”成小块的核心语法。

•**强制类型转换 **(Block*)：将一块普通的 char 内存，强行解释为 Block 结构体，从而可以使用 ->next 来串联链表。

### 3. 常用且好用的关键字

•**static constexpr**：

    ◦static：属于类，不属于对象。

    ◦constexpr：编译期常量。在编译阶段就确定了值（如 64 和 65536），不会在运行时占用内存，且能用于数组大小等编译期计算。

•**volatile**：防优化神器。告诉编译器“这个变量的值可能会在程序外部被改变，不要自作聪明地把它优化掉，必须每次都从内存读取/写入”。

•**auto**：类型推导。for (auto ptr : chunks) 中，编译器自动推断出 ptr 是 void* 类型，让代码更简洁。

•**template<typename T>**：模板。do_not_optimize 函数可以接收任何类型的参数，实现了代码的泛型复用。

### 4. 测试与工程化工具

•**随机数种子** srand(42)：rand() 生成的是伪随机数。传入固定的种子 42，能保证每次运行程序时，生成的随机数序列完全一致。这是做性能基准测试（Benchmark）的前提，否则每次测试条件不同，结果没有可比性。

•**高精度计时器** std::chrono：C++11 引入的时间库。high_resolution_clock 是最高精度的时钟，配合 duration_cast 可以精确到毫秒/微秒，比 C 语言的 time() 精确得多。

•**格式化输出** <iomanip>：std::fixed << std::setprecision(1) 控制浮点数输出格式，保留一位小数，提升输出的专业度。

## 二、 核心方法与思想总结

### 1. **空间换时间** (Space-Time Tradeoff)

这是内存池最核心的思想。通过预先向操作系统申请一大块内存（64KB），把昂贵的“系统调用”平摊到成百上千次小对象分配上。用少量的内存浪费（内部碎片），换取了几十倍的速度提升。

### 2. **O(1) 时间复杂度的分配与回收**

传统的 malloc 在空闲内存变多时，需要遍历寻找合适的块，时间复杂度可能是 O(N)。而内存池使用头插法维护空闲链表，无论是分配（从链表头摘下一个节点）还是回收（把节点插回链表头），都只需要修改一两个指针，时间复杂度永远是 O(1)。

### 3. **防御性编程** (Defensive Programming)

代码中大量使用了边界检查：size == 0、size > BLOCK_SIZE、if (!newChunk) 等。在底层内存操作中，任何越界或空指针都会导致程序崩溃（Segmentation Fault），防御性检查是保证系统稳定性的基石。

### 4. **科学的基准测试方法论**

•多轮测试取平均：消除操作系统调度、CPU 缓存冷热等环境干扰。

•防编译器优化：现代编译器极其聪明，如果检测到分配了内存却没有使用，会直接删掉分配代码。do_not_optimize 确保了测试的真实性。

•对象生命周期模拟：在循环内创建 MemoryPool pool;，确保每轮测试都经历完整的“初始化 -> 高频分配释放 -> 析构清理”过程，最贴近真实业务场景。

## 三、 核心流程与变量状态变化图示

下面以 MemoryPool 的分配与回收为例，展示核心变量 freeList 和内存块的状态流转：

```

【阶段 1：初始化与切分内存】

当 freeList == nullptr 时，触发扩容：

申请 64KB 内存 (newChunk)

---------------------------------------------------------

| Block 0 | Block 1 | Block 2 | ... | Block N |

---------------------------------------------------------

通过 for 循环头插法串联：

freeList -> [Block N] -> [Block N-1] -> ... -> [Block 0] -> nullptr



【阶段 2：用户请求分配 (allocate)】

用户调用 allocate(32)

1. 检查 size <= 64，通过。

2. 检查 freeList != nullptr，池中有空闲块。

3. result = freeList;           // 保存当前链表头 (Block N)

4. freeList = freeList->next;   // 链表头后移，指向 Block N-1

5. return result;               // 将 Block N 交给用户



状态变化：

freeList -> [Block N-1] -> [Block N-2] -> ... -> nullptr

(用户手中持有了 Block N)



【阶段 3：用户释放内存 (deallocate)】

用户调用 deallocate(Block N, 32)

1. 检查 size <= 64，属于池内内存。

2. block = (Block*)ptr;         // 将用户内存重新视为 Block 节点

3. block->next = freeList;      // 让 Block N 指向当前的链表头 (Block N-1)

4. freeList = block;            // 链表头重新指向 Block N



状态变化：

freeList -> [Block N] -> [Block N-1] -> [Block N-2] -> ... -> nullptr

(Block N 被完美回收，等待下一次分配)



【阶段 4：生命周期结束 (析构)】

MemoryPool 对象离开作用域，触发 ~MemoryPool()

遍历 chunks 数组，对每一块向系统申请的 64KB 内存调用 ::free()

内存彻底归还给操作系统。

```

### 变量流转总结：

•**chunks** (vector)：像一个“仓库账本”，只进不出。每次向系统申请新大块内存时 push_back，仅在析构时统一 free。

•**freeList (Block)**：像一个“旋转门”。allocate 时从门里拿走一个块（指针后移），deallocate 时把用过的块塞回门里（指针前移）。它永远指向当前最快能拿到的空闲内存。



