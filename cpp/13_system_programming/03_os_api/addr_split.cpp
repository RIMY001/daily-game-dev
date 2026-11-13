//	目的：拿到系统页大小，把一个局部变量的虚拟地址拆成页号与页内偏移
//	MMU做的事：低12位是偏移，高位拿去逐级查表
#include <windows.h>
#include <cstdio>

int main() {
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	size_t page = si.dwPageSize;

	int local = 0;
	size_t addr = (size_t)&local;
	size_t base = addr & ~(page - 1);	//	页对齐基址
	size_t off = addr & (page - 1);	//	页内偏移

	printf("page size = %zu bytes\n", page);
	printf("sample VA = %p\n", (void*)addr);
	printf("page base = %p\n", (void*)base);
	printf("offset	  = %zu\n", off);
	return 0;
}