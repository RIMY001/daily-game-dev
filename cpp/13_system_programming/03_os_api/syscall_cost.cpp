//	对比三个API：共享页读取/真系统普通内存读， 各跑 10 万次。
#include <windows.h>
#include <cstdio>

static volatile unsigned long long g_sink;

template <class F>
double Bench(const char* name, int n, F&& f) {
	LARGE_INTEGER fr, a, b;
	QueryPerformanceFrequency(&fr);
	QueryPerformanceCounter(&a);
	for (int i = 0; i < n; ++i) f();
	QueryPerformanceCounter(&b);
	double ns = 1e9 * double(b.QuadPart - a.QuadPart) / double(fr.QuadPart) / n;
	printf("%-34s %7.1f ns/op\n", name, ns);
	return ns;
}

int main() {
	constexpr int N = 100'000;
	DWORD dummy = 0;
	Bench("普通内存读(volatile)", N, [&] { g_sink += 1; });
	Bench("GetTickCount64 [共享页]", N, [&] { g_sink += GetTickCount64(); });
	Bench("GetProcessHandle [syscall]", N,
		[&] { GetProcessHandleCount(GetCurrentProcess(), &dummy); });
	Bench("CreateFileW+ClossHandle [2 syscall]", 1000,
		[&] { HANDLE h = CreateFileW(L"NUL", GENERIC_READ, 0, nullptr,
			OPEN_EXISTING, 0, nullptr);
	CloseHandle(h); });
	return 0;
}