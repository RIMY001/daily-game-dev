#include <cstdlib>
#include <cstring>
#include <vector>
#include <chrono>
#include <iostream>
#include <iomanip>

// ============== 1. 简易分级内存池实现 =================
class MemoryPool {
	//###	定义一个内存池类
private:
	static constexpr size_t BLOCK_SIZE = 64;//覆盖测试最大尺寸
	//###	static:静态成员，属于类本身，而不是某个具体对象
	//###	constexpr:常量表达式，编译期就确定好值了，不能修改。相当于C语言里的#define，但更安全
	//###	size_t:C++中专门用来表示内存大小、数组索引的无符号整型类型
	static constexpr size_t CHUNK_SIZE = 4096 * 16;//每次申请64KB,减少系统调用

	struct Block { Block* next; };
	//###	定义了一个结构体，里面只有一个成员Block* next。这是一个非常典型的单向链表节点

	Block* freeList = nullptr;
	//###	定义了一个指针，初始化为空（nullptr）。它永远指向当前空闲内存块链表的最开头
	std::vector<void*>chunks;
	//###	void*是无类型指针，它可以指向任何内存地址。这个数组用来记录我们向操作系统申请的所有大块内存的起始地址，方便最后统一释放

public:
	~MemoryPool() {
	//###	当MenmoryPool对象的生命周期结束时，它会自动被调用
		for (auto ptr : chunks) ::free(ptr);
	}
	//###	for(aut ptr : chunks):这是C++11引入的范围for循环（类似与python的for）。会让编译器自动推断ptr的类型（在这里就是void*）。这句话的意思是：遍历chunks数组里的每一个指针。
	//###	::free(ptr);: ::叫做全局作用域解析符。free是C语言标准库的释放内存函数。加上::是为了明确告诉编译器：“我要调用的是C语言全局的free函数，而不是其他类里可能同名的函数”。

	void* allocate(size_t size) {
	//###	void*allocate(size_t size):分配函数，需要传入的字节数，返回一个void*指针（因为内存池不知道用户到底要存什么类型的指针，所以返回通用指针）
		if (size > BLOCK_SIZE || size == 0)return ::malloc(size);
	//###	降级机制：如果用户要的内存大于64字节，或者size等于0（防御性编程），内存池就不管了，直接调用C语言的::malloc(size)去帮用户去系统申请

		if (!freeList) {
	//###	if(!freeList):如果链表头是空的（说明池子里没内存了）
			char* newChunk = (char*)::malloc(CHUNK_SIZE);
	//###	像系统一次性申请4096字节的大内存。（char*）是强制类型转换，把void*转成字符指针，方便后面做指针运算
			if (!newChunk) return nullptr;
			chunks.push_back(newChunk);
	//###	把这块大内存的起始地址放进数组里记下来
			
			for (size_t i = 0; i <= CHUNK_SIZE - BLOCK_SIZE; i += BLOCK_SIZE) {
				Block* block = (Block*)(newChunk + i);
				block->next = freeList;
				freeList = block;
			}
	//###	切分内存：用for循环，每次步进64字节（BLOCK_SIZE）
	//###	指针运用的安全边界：这里用了 <= CHUNK_SIZE - BLOCK_SIZE。为什么要这么写？因为如果只写 < CHUNK_SIZE当i走到最后64字节时，可能会越界。减去一个BLOCK_SIZE能确保切分出来的每一块内存都在合法范围内
	//###	newChunk + i:指针运算！ newChunk是char*,所以+i就是往后移动i个字节。(BLOCK*)把这段内存强行当成Block结构体来看待
	//###	头插法建链表：block->next = freeList;freeList = block;。把新切出来的块，插到链表的最前面。循环结束后，这4096字节就被串成了一列“火车车厢”。
		}
		Block* result = freeList;
		freeList = freeList->next;
		return result;
	//###	分配给用户：把链表头(freeList)摘下来，让freeList指向下一个节点，然后把摘下来的节点地址返回给用户
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
	//###	归还内存：如果用户还回来的内存属于池子管的范围，我们不调用free还给操作系统，而是把它重新挂到freeList链表的最前面（block->next = freeList;freeList = block;）
	//###	核心思想：内存池的核心思想就在于“借而不还（给系统），内部循环使用”。这避免了频繁和操作系统打交道，所以速度极快。
};

// ============= 2.测试辅助函数 =================
// 产生[min,max]区间的随机数
int get_random_number(int min, int max) {
	return rand() % (max - min + 1) + min;
}

// 防止编译器优化掉分配操作的屏障
template <typename T>
void do_not_optimize(T&& value) {
	volatile auto sink = value;
	(void)sink;
}
	//###	volatile:这是一个关键字，意思是“易变的”。他强制告诉编译器：“这个变量sink随时可能在外部被改变，你千万不要把它优化掉，必须老老实实地执行赋值操作！”
	//###	(void)sink;: 把sink强转为void，意思是“我故意不用这个变量，你别给我报警告”。

// ============= 3.核心测试用例 ===============
const int TEST_COUNT = 10000000;

long long test_malloc(){
	auto start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < TEST_COUNT; i++) {
		int size = get_random_number(16, 64);
		int flag = rand() % 2;
		void* p = ::malloc(size);
		do_not_optimize(p);
		if (flag == 1)::free(p);
	}
	auto end = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}
	//###	long long:返回一个长整型，因为1000万次操作的毫秒数可能很大，普通的int可能会溢出
	//###	封装思想：把计时和测试逻辑写成一个独立的函数，返回耗时。这样main函数就会变得非常干净

long long test_new() {
	auto start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < TEST_COUNT; i++) {
		int size = get_random_number(16, 64);
		int flag = rand() % 2;
		char* p = new char[size];
		do_not_optimize(p);
		if (flag == 1)::delete[] p;
	}
	auto end = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

long long test_memory_pool(MemoryPool& pool) {
	auto start = std::chrono::high_resolution_clock::now();
	//###	在栈上创建一个内存池对象
	//###	计时起点：获取当前的高精度时间
	for (int i = 0; i < TEST_COUNT; i++) {
		int size = get_random_number(16, 64);
		int flag = rand() % 2;
		void* p = pool.allocate(size);
		do_not_optimize(p);
		if (flag == 1) pool.deallocate(p, size);
	}
	//###	模拟真实场景：循环一千万次。每次随机要16~64字节的内存，然后掷硬币(rand()%2)，如果是1就立即把内存还给对象池。
	//###	这是在疯狂测试内存池的分配和回收速度
	auto end = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

// ================== 4.主函数执行10轮取平均值 =======================
int main() {
	srand(42);//固定种子
	//###	初始化随机数种子。42是固定的，这样每次运行程序产生的随机数序列都一样，方便测试对比

	const int ROUNDS = 10;
	long long malloc_total = 0, new_total = 0, pool_total = 0;

	std::cout << "开始性能测试(" << TEST_COUNT << "次/轮，共" << ROUNDS << "轮）...\n";

	for (int r = 0;r < ROUNDS;r++) {
		malloc_total += test_malloc();
		new_total    += test_new();

		MemoryPool pool;//每轮重建内存池，模拟真实生命周期
		pool_total   += test_memory_pool(pool);
	//###	对象的生命周期：注意MemoryPool pool;是写在for循环里的。这意味着每跑完一轮测试,pool就会离开作用域，析构函数~MemoryPool()就会被自动调用一次，把内存清理干净。下一轮再重新创建。这完美模拟了现实中“创造池->疯狂分配释放->销毁池”的真实场景

		std::cout << "  第 " << (r + 1) << "/" << ROUNDS << " 轮完成\n";
	}

	std::cout << "\n=========== 测试结果 （单位：ms） =============\n";
	std::cout << std::fixed << std::setprecision(1);
	std::cout << "malloc 平均耗时： " << (double)malloc_total / ROUNDS << "ms\n";
	//###	std::fixed << std::setprecision(1)：这是控制输出格式的语法。fix表示用普通小数格式（而不是科学计数法），setprecision(1)表示保留1位小数
	//###	(double)malloc_total:强制类型转换。因为两个整数相除会丢掉小数部分，先转成(double)再除以ROUNDS，才能得到精准的平均值
	std::cout << "new    平均耗时： " << (double)new_total / ROUNDS << "ms\n";
	std::cout << "内存池  平均耗时： " << (double)pool_total / ROUNDS << "ms\n";
	std::cout << "================================================\n";

	return 0;
}
