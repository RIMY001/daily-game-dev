//	把一个文件映射进地址空间，再用 ReadFile 读同一文件
//	二者命中的是同一份系统页缓存，不是两份独立拷贝
//	准备测试文件:fsutil file createnew test.bin 1048576

#include <windows.h>
#include <cstdio>

int main(int argc, char** argv) {
	const char* path = argc > 1 ? argv[1] : "test.bin";
	HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, nullptr,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (h == INVALID_HANDLE_VALUE) {
		printf("open fail, last error = %lu\n", GetLastError());
		return 1;
	}
	HANDLE map = CreateFileMappingA(h, nullptr, PAGE_READONLY, 0, 0, nullptr);
	const char* view = (const char*)MapViewOfFile(map, FILE_MAP_READ, 0, 0, 0);
	if (view) {
		printf("mapped base = %p, first byte = 0x%02x\n",
			(const void*)view, (unsigned char)view[0]);
		UnmapViewOfFile(view);
	}
	else {
		printf("map fail, last error = %lu\n", GetLastError());
	}

	char buf[64] = { 0 };
	DWORD got = 0;
	ReadFile(h, buf, sizeof(buf), &got, nullptr);
	printf("ReadFile got = %lu bytes, first = 0x%02x\n", got,
		got ? (unsigned char)buf[0] : 0);
	CloseHandle(map);
	CloseHandle(h);
	return 0;
}