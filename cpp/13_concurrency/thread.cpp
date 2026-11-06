#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

//------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------线程局部储存
thread_local int threadData = 0;

void threadFunction() {
	threadData = 42;	//	每个线程都有自己的 threadData 副本
	std::cout << "Thread data:" << threadData << std::endl;
}

int main() {
	std::thread t1(threadFunction);
	std::thread t2(threadFunction);
	t1.join();
	t2.join();

	return 0;
}
//------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------原子操作、内存序、CAS
//std::atomic<int> count(0);
//std::mutex print_mtx;
//
//void increment() {
//	//	relaxed. 单纯计数
//	auto old = count.fetch_add(1, std::memory_order_relaxed);
//	//读取当前值 → 根据当前值做 if 判断 → 条件成立才写新值。这种【读 - 判断 - 写】复合原子逻辑，要 CAS 循环，不能用 fetch_add 等。
//	std::lock_guard<std::mutex> lk(print_mtx);	//	这个锁只是管打印，不保护 count，count 依然是 atomic 无锁计数。
//	std::cout << "线程" << std::this_thread::get_id()
//		<< " 旧值:" << old << " 加1\n";
//	//	补充：原子 API 和内存序 memory_order
//	//	T fetch_add(T arg, memory_order)作用:原子地拿到当前值，加上 arg，再写回去;返回值：修改之前的旧值
//	//	同类常用原子 API:fetch_sub减; fetch_and / fetch_or / fetch_xor位运算; store(val)原子写; load()原子读
//	//	原子不只是 “原子”，还有CPU 缓存、指令重排的问题，内存序就是用来约束编译器 + CPU 能不能乱排指令
//}
//
//int main() {
//	std::thread t1(increment);
//	std::thread t2(increment);
//	t1.join();
//	t2.join();
//	std::cout << "最终 count = " << count << std::endl;
//}
//------------------------------------------------------------------------------------------
//----------------------------------------------------------------条件变量 condition_variable
//std::mutex mtx;
//std::condition_variable cv;
//bool ready = false;
//int shared_data = 0;
//
//void workerThread() {
//	std::cout << "子线程启动，准备等待信号\n";
//	std::unique_lock<std::mutex> lk(mtx);
//	// wait 做三件事：
//	// 1. 先判断谓词 ready 是否为true；false就解锁mtx，阻塞在这里休眠
//	// 2. 收到notify唤醒后，重新抢到锁，再检查一次谓词
//	// 3. 谓词返回true，wait返回，继续往下走
//	cv.wait(lk, []() {return ready; });
//
//	//	拿到锁，安全访问共享数据
//	std::cout << "子线程收到通知，开始工作，shared_data = " << shared_data << "\n";
//	shared_data += 100;
//	std::cout << "子线程工作结束，shared_data = " << shared_data << "\n";
//}
//
//
//int main() {
//	std::cout << "主线程启动" << std::endl;
//	std::thread worker(workerThread);
//
//	//	模拟主线程做一些准备工作
//	std::this_thread::sleep_for(std::chrono::seconds(1));
//
//	{
//		std::lock_guard<std::mutex> lk(mtx);
//		std::cout << "主线程准备数据中\n";
//		shared_data = 666;
//		ready = true;
//	}	//	lock_guard 出花括号，自动解锁
//
//	std::cout << "主线程发送 notify_one 通知子线程\n";
//	cv.notify_one();
//
//	worker.join();	//	等待子线程执行完毕
//	std::cout << "主线程全部结束， 最终 shared_data = " << shared_data << "\n";
//
//	return 0;
//}
//------------------------------------------------------------------------------------------
//-----------------------------------------------------------------lock_guard 和 unique_lock
//int a = 0;
//std::mutex mtx;
//
//void safeFunctionWithLockGuard() {
//	//	访问或修改共享数据
//	for (int i = 0; i < 10000; ++i) {
//		std::lock_guard<std::mutex> lk(mtx);
//		a += 1;
//	}
//}
//
//void safeFunctionWithUniqueLock() {
//	std::unique_lock<std::mutex> ul(mtx);
//	//	访问或修改共享数据
//	//	ul.unlock();	//	可选：手动解锁
//}
//
//int main() {
//	std::thread t1(safeFunctionWithLockGuard);
//	std::thread t2(safeFunctionWithLockGuard);
//	t1.join();
//	t2.join();
//	std::cout << a << std::endl;
//
//	return 0;
//}
//------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------互斥锁
//int a = 0;
//std::mutex mtx;	
//void fun() {
//	for (int i = 0; i < 10000; ++i) {
//		mtx.lock();	//	锁定互斥锁
//		a += 1;	//	访问共享数据
//		mtx.unlock();	//	释放互斥锁
//	}
//}
//
//int main() {
//	std::thread thread1(fun);
//	std::thread thread2(fun);
//	thread1.join();
//	thread2.join();
//	std::cout << a << std::endl;
//
//	return 0;
//}
//------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------创建线程
////	一个简单函数，作为线程的入口函数
//void foo(int z) {
//	for (int i = 0; i < z; ++i) {
//		std::cout << "线程使用函数指针作为可调用参数\n";
//	}
//}
//
////	可调用对象的类定义
//class ThreadObj {
//public:
//	void operator()(int x) const {
//		for (int i = 0; i < x; ++i) {
//			std::cout << "线程使用函数对象作为可调用参数\n";
//		}
//	}
//};
//
//int main() {
//	std::cout << "线程 1、2、3 独立运行" << std::endl;
//
//	//	使用函数指针创建线程
//	std::thread th1(foo, 3);
//
//	//	使用函数对象创建线程
//	std::thread th2(ThreadObj(), 3);
//
//	//	使用 lambda 表达式创建线程
//	std::thread th3([](int x) {
//		for (int i = 0; i < x; ++i) {
//			std::cout << "线程使用 lambda 表达式作为可调用参数\n";
//		}
//	}, 3);
//
//	//	等待所有线程完成
//	th1.join();
//	th2.join();
//	th3.join();
//	//	线程的执行顺序由操作系统的调度算法决定，多个线程会并发运行，输出可能交错
//}