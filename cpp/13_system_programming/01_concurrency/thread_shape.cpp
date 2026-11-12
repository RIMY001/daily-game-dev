//	两个工作线程各自打印：自己的 TID、TLS 变量地址、栈上局部变量地址、以及共享全局变量的地址
#include <Windows.h>
#include <cstdio>

int g_shared_heap_side = 0;	//	全局：所有线程共享
thread_local int t_tls = 0;	//	线程局部：每个线程独立

DWORD WINAPI Worker(LPVOID param) {
	int local_on_stack = 0;	//	栈上局部变量：每个线程独立
	t_tls = reinterpret_cast<intptr_t>(param);	//	写入各自的那一份
	printf("worker #%lld	TID=%-6lu TLS=%p	stack=%p	global=%p\n",
		(long long)(intptr_t)param, GetCurrentThreadId(),
		(void*)&t_tls, (void*)&local_on_stack, (void*)&g_shared_heap_side);
	return 0;
}

int main() {
	printf("main		TID=%-6lu TLS=%p\n", GetCurrentThreadId(), (void*)&t_tls);
	HANDLE h[2];
	for (int i = 0; i < 2; ++i) {
		h[i] = CreateThread(nullptr, 0, Worker, (LPVOID)(intptr_t)(i + 1), 0, nullptr);
	}
	WaitForMultipleObjects(2, h, TRUE, INFINITE);
	CloseHandle(h[0]); CloseHandle(h[1]);
	return 0;
}