//	目的：打印栈变量、堆分配、模块映像三个典型位置的虚拟地址，
//	栈在高址、堆在中间、模块映像另有基址
#include <windows.h>
#include <cstdio>
#include <cstdlib>

int main() {
	int onStack = 0;
	void* heap = malloc(256);
	HMODULE mod = GetModuleHandle(nullptr);

	printf("stack var = %p\n", (void*)&onStack);
	printf("heap ptr = %p\n", heap);
	printf("module base = %p\n", (void*)mod);

	SYSTEM_INFO si;	//`SYSTEM_INFO` 是 Win32 预定义结构体，专门用来存放**当前 CPU、内存地址空间的硬件 / 系统信息**
	GetSystemInfo(&si);	//接收 `SYSTEM_INFO*`，传入结构体的地址，然后操作系统把系统信息填入结构体
	printf("user top = %p\n", (void*)si.lpMaximumApplicationAddress);

	free(heap);
	return 0;
}
//	栈向下增长指的是**入栈时地址数值减小**，不代表栈内存块的地址一定高于堆；现代 OS 开启 ASLR 后，栈、堆、模块的虚拟地址位置是随机的