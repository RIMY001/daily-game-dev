#include <windows.h>
#include <cstdio>

static volatile unsigned long long g_sink;	//	volatile:防止优化把整个循环删掉

__declspec(noinline) static void UserCall(unsigned long long v) { g_sink += v; }

template<class F>
static double Bench(int n, F&& f) {
	LARGE_INTEGER fr, a, b;
	QueryPerformanceFrequency(&fr);
	QueryPerformanceCounter(&a);
	for (int i = 0; i < n; ++i) f(i);
	QueryPerformanceCounter(&b);
	return 1e9 * double(b.QuadPart - a.QuadPart) / double(fr.QuadPart) / n;
}

int main() {
	constexpr int N = 1000000;
	DWORD handles = 0;

	//	1.纯用户态： 1M 次函数调用
	double nsUser = Bench(N, [](int i) { UserCall(i);});

	//	2.真系统调用： GetProcessHandleCount 内部走 NtQueryInformationProcess
	double nsSys = Bench(N, [&](int) { GetProcessHandleCount(GetCurrentProcess(), &handles); });

	printf("%-24s %7.2f ns/op\n", "user-space call", nsUser);
	printf("%-24s %7.2f ns/op\n", "GetProcessHandelCount", nsSys);
	printf("ratio: %.0fx\n", nsSys / nsUser);
	return 0;

}