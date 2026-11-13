//	目的：从低地址到高地址遍历本进程地址空间，打印每一段的状态
//	状态只有三种：FREE（未占用）/RESERVE（保留）/COMMIT（已提交）
#include <windows.h>
#include <cstdio>

int main() {
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	printf("page size:		  = %lu KB\n", si.dwPageSize / 1024);
	printf("alloc granularity = %lu KB\n", si.dwAllocationGranularity / 1024);

	MEMORY_BASIC_INFORMATION mbi;
	unsigned char* addr = nullptr;
	int cntFree = 0, cntReserve = 0, cntCommit = 0;

	while (VirtualQuery(addr, &mbi, sizeof(mbi))) {
		const char* state = "OTHER";
		if (mbi.State == MEM_FREE) { state = "FREE"; ++cntFree; }
		else if (mbi.State == MEM_RESERVE) { state = "RESERVE"; ++cntReserve; }
		else if (mbi.State == MEM_COMMIT) { state = "COMMIT"; ++cntCommit; }

		if (cntFree + cntReserve + cntCommit <= 12) {
			printf("%-8s base=%p size=%8llu KB prot=0x%lx\n",
				state, mbi.BaseAddress,
				(unsigned long long)mbi.RegionSize / 1024,
				(unsigned long long)mbi.Protect);
		}
		addr = (unsigned char*)mbi.BaseAddress + mbi.RegionSize;
		if (addr == nullptr) break;
	}
	printf("runs:free=%d reserve=%d commit=%d\n",
		cntFree, cntReserve, cntCommit);
	return 0;
}
