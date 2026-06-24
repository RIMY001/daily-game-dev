#include <cstdlib>
#include <cstring>
#include <vector>
#include <chrono>
#include <iostream>
#include <iomanip>

// ==================== 1. 简易分级内存池实现 ====================
class MemoryPool {
private:
    static constexpr size_t BLOCK_SIZE = 64;      // 覆盖测试最大尺寸
    static constexpr size_t CHUNK_SIZE = 4096 * 16; // 每次申请64KB，减少系统调用

    struct Block { Block* next; };

    Block* freeList = nullptr;
    std::vector<void*> chunks;

public:
    ~MemoryPool() {
        for (auto ptr : chunks) ::free(ptr);
    }

    void* allocate(size_t size) {
        if (size > BLOCK_SIZE || size == 0) return ::malloc(size);

        if (!freeList) {
            char* newChunk = (char*)::malloc(CHUNK_SIZE);
            if (!newChunk) return nullptr;
            chunks.push_back(newChunk);

            // 将Chunk切分为Block并串成链表
            for (size_t i = 0; i <= CHUNK_SIZE - BLOCK_SIZE; i += BLOCK_SIZE) {
                Block* block = (Block*)(newChunk + i);
                block->next = freeList;
                freeList = block;
            }
        }

        Block* result = freeList;
        freeList = freeList->next;
        return result;
    }

    void deallocate(void* ptr, size_t size) {
        if (!ptr || size > BLOCK_SIZE || size == 0) {
            ::free(ptr);
            return;
        }
        Block* block = (Block*)ptr;
        block->next = freeList;
        freeList = block;
    }
};

// ==================== 2. 测试辅助函数 ====================
// 产生[min, max]区间的随机数
int get_random_number(int min, int max) {
    return rand() % (max - min + 1) + min;
}

// 防止编译器优化掉分配操作的屏障
template<typename T>
void do_not_optimize(T&& value) {
    volatile auto sink = value;
    (void)sink;
}

// ==================== 3. 核心测试用例 ====================
const int TEST_COUNT = 10000000;

long long test_malloc() {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < TEST_COUNT; i++) {
        int size = get_random_number(16, 64);
        int flag = rand() % 2;
        void* p = ::malloc(size);
        do_not_optimize(p);
        if (flag == 1) ::free(p);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

long long test_new() {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < TEST_COUNT; i++) {
        int size = get_random_number(16, 64);
        int flag = rand() % 2;
        // new char[] 对应 malloc，避免构造函数干扰
        char* p = new char[size];
        do_not_optimize(p);
        if (flag == 1) delete[] p;
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

long long test_memory_pool(MemoryPool& pool) {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < TEST_COUNT; i++) {
        int size = get_random_number(16, 64);
        int flag = rand() % 2;
        void* p = pool.allocate(size);
        do_not_optimize(p);
        if (flag == 1) pool.deallocate(p, size);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

// ==================== 4. 主函数：执行10轮取平均值 ====================
int main() {
    srand(42); // 固定种子保证结果可复现

    const int ROUNDS = 10;
    long long malloc_total = 0, new_total = 0, pool_total = 0;

    std::cout << "开始性能测试 (" << TEST_COUNT << " 次/轮, 共 " << ROUNDS << " 轮)...\n";

    for (int r = 0; r < ROUNDS; r++) {
        malloc_total += test_malloc();
        new_total += test_new();

        MemoryPool pool; // 每轮重建内存池，模拟真实生命周期
        pool_total += test_memory_pool(pool);

        std::cout << "  第 " << (r + 1) << "/" << ROUNDS << " 轮完成\n";
    }

    std::cout << "\n========== 测试结果 (单位: ms) ==========\n";
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "malloc 平均耗时: " << (double)malloc_total / ROUNDS << " ms\n";
    std::cout << "new     平均耗时: " << (double)new_total / ROUNDS << " ms\n";
    std::cout << "内存池 平均耗时: " << (double)pool_total / ROUNDS << " ms\n";
    std::cout << "=========================================\n";

    return 0;
}