#include <windows.h>
#include <cstdio>

static double NowMs() {
	static LARGE_INTEGER f{};
	if (!f.QuadPart) QueryPerformanceFrequency(&f);
	LARGE_INTEGER t; QueryPerformanceCounter(&t);
	return 1000.0 * double(t.QuadPart) / double(f.QuadPart);
}

int main() {
	constexpr size_t GB = 1ULL << 30;

	//	RESERVE:占一段虚拟地址，不占物理页、不计提交承诺
	double t0 = NowMs();
	void* r1 = VirtualAlloc(nullptr, GB, MEM_RESERVE, PAGE_NOACCESS);
	double t1 = NowMs();
	printf("reserve 1GB		: %.3f ms	ptr=%p\n", t1 - t0, r1);
	VirtualFree(r1, 0, MEM_RELEASE);

	//	RESERVE + COMMIT， 然后只触碰第一页
	t0 = NowMs();
	void* r2 = VirtualAlloc(nullptr, GB, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	volatile unsigned char* p = (volatile unsigned char*)r2;
	p[0] = 1;							//	首次写入 -> 一次缺页异常
	t1 = NowMs();
	printf("commit 1GB + touch page : %.3f ms	ptr=%p\n", t1 - t0, r2);
	VirtualFree(r2, 0, MEM_RELEASE);
	return 0;
}