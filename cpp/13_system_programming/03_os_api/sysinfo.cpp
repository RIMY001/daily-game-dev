//	打印四个“任何操作系统都必须回答”的问题：
//	页多大、几个逻辑核、本进程开了多少句柄、当前线程栈大概多大
#include <Windows.h>
#include <cstdio>

int main() {
	SYSTEM_INFO si{};
	GetNativeSystemInfo(&si);
	printf("page size		: %lu bytes\n", si.dwPageSize);
	printf("allocation unit : %lu bytes\n", si.dwAllocationGranularity);
	printf("logical cores	: %lu\n", si.dwNumberOfProcessors);

	DWORD handles = 0;
	GetProcessHandleCount(GetCurrentProcess(), &handles);
	printf("open handles	: %lu\n", handles);

	//	栈顶附近取一个局部变量地址：与 TEB 里的 StackBase 一比就知道用了多少
	int probe = 0;
	printf("stack probe		: %p\n", (void*)&probe);
	return 0;
}