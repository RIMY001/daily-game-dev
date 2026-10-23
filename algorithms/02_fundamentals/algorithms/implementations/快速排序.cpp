#include <iostream>
#include <vector>
#include <utility>
#include <cassert>
#include <algorithm>

//	====================================== Lomuto ======================================

//	Lomuto 分区
int lomuto_partition(std::vector<int>& arr, int lo, int hi) {
	//	最右元素作为基准值
	int pivot = arr[hi];

	//	i 为边界，左边为小于区
	int i = lo - 1;	//	初始小于区为空

	//	j 从 lo 开始扫描，将小于 pivot 的元素移到小于区，大于 pivot 的元素不管
	for (int j = lo; j < hi; ++j) {
		if (arr[j] < pivot) {
			//	小于区扩展,此时 arr[i] 为小于区边界的下一元素（大于 pivot），arr[j] 小于 pivot ，
			++i;
			//	交换使得 arr[i] 为小于区边界
			std::swap(arr[j], arr[i]);
		}
	}

	//	 i + 1 为 pivot 的最终索引位置
	std::swap(arr[i + 1], arr[hi]);

	return i + 1;
}

//	基于 Lomoto 分区的快速排序
void lomuto_quicksort(std::vector<int>& arr, int lo, int hi) {
	//	递归终止条件：区间为0或1
	if (lo >= hi) return;

	//	执行分区，获得 pivot 最终位置 p
	int p = lomuto_partition(arr, lo, hi);

	//	p - 1 和 p + 1 变为新的区间边界
	lomuto_quicksort(arr, lo, p - 1);
	lomuto_quicksort(arr, p + 1, hi);
}


//	====================================== Hoare ======================================

//	Hoare 分区
int hoare_partition(std::vector<int>& arr, int lo, int hi) {
	//	选最左侧元素为基准值
	int pivot = arr[lo];

	int L = lo - 1;
	int R = hi + 1;

	while (true) {
		//	do-while 保证至少移动一步，避免死循环
		//	while 循环先判断再移动。当 pivot 是区间最大 / 最小值，或者大量等于 pivot 时，容易卡死死循环
		//	`arr = [2, 2, 2, 2]`，pivot = 2`，swap交换后死循环
	
		do { ++L; } while (arr[L] < pivot);	//	跳过小于 pivot 的元素
		do { --R; } while (arr[R] > pivot);	//	跳过大于 pivot 的元素

		if (L >= R) return R;	// arr[lo..R] 中的元素都 <= pivot  arr[R+1..hi] 中的元素都 >= pivot

		std::swap(arr[L], arr[R]);	// 交换后 L 和 R 位置的元素已正确归边，下一轮循环会继续推进
	}
}

//	基于 Hoare 分区的快速排序
void hoare_quicksort(std::vector<int>& arr, int lo, int hi) {
	if (lo >= hi) return;

	int p = hoare_partition(arr, lo, hi);

	// 因为 hoare_partition 返回的不是 pivot 确切位置，所以递归区间必须包含返回值 p，即[lo, p] 而非[lo, p - 1]。
	hoare_quicksort(arr, lo, p);
	hoare_quicksort(arr, p + 1, hi);
}

//	====================================== DNF 三向切分 ======================================

/** 【核心思想】
 * 将数组分成三个区域：
 *   arr[lo .. lt-1]  : < pivot  （小于区）
 *   arr[lt .. gt]    : == pivot （等于区）
 *   arr[gt+1 .. hi]  : > pivot  （大于区）
 * 
 * 等于区的元素一次性归位，永远不再参与后续递归！
 * 当大量元素相等时，性能可从 O(N log N) 提升至 O(N)。*/

//	DNF分区
std::pair<int, int> dnf_partition(std::vector<int>& arr, int lo, int hi) {
	int pivot = arr[lo];

	int lt = lo;	//	小于区右边界（不含），初始时小于区为空
	int gt = hi;	//	大于区左边界（不含），初始时大于区为空
	int i = lo + 1;	//	i 为当前待检查元素的指针，从等于区的起始位置开始

	//	i 越过 gt 时，所有元素都分类完毕
	while (i <= gt) {
		if (arr[i] < pivot) {
			std::swap(arr[lt], arr[i]);	//	此时 lt 指向交换前 arr[i] 的元素（这个数小于基准点），i 指向的元素等于基准点
			++lt;	//	因为不包含故 lt 右移
			++i;	//	同理，右移
		}
		else if (arr[i] > pivot) {
			std::swap(arr[gt], arr[i]);
			--gt;
			//	i 不动！交换前的 arr[gt] 没被检查过
		}
		else {
			++i;	//	直接拓展等于区
		}
	}

	return { lt,gt };
}

//	基于 DNF 分区的快速排序
void dnf_quicksort(std::vector<int>& arr, int lo, int hi) {
	if (lo >= hi) return;

	//	获取等于区边界
	auto [eq_lo, eq_hi] = dnf_partition(arr, lo, hi);

	dnf_quicksort(arr, lo, eq_lo - 1);
	dnf_quicksort(arr, eq_hi + 1, hi);
}

// ============================= 测试与验证工具 =============================
void print_array(const std::vector<int>& arr, const char* label) {
	std::cout << label << ":[";
	for (size_t i = 0; i < arr.size(); ++i) {
		std::cout << arr[i];
		if (i + 1 < arr.size()) std::cout << ", ";
	}
	std::cout << "]" << std::endl;
}

bool is_sorted(const std::vector<int>& arr) {
	for (size_t i = 1; i < arr.size(); ++i) {
		if (arr[i] < arr[i - 1]) return false;
	}

	return true;
}

int main() {
	const std::vector<int> original = { 6, 1, 2, 7, 9, 3, 4, 5, 10, 8, 6, 6, 3 };

	std::cout << "==========快速排序三种分区方案演示 ==========" << std::endl;
	print_array(original, "原始数组");
	std::cout << std::endl;

	// --- 测试 Lomuto ---
	{
		std::vector<int> arr = original;	//	拷贝一份独立数据
		lomuto_quicksort(arr, 0, static_cast<int>(arr.size()) - 1);
		print_array(arr, "Lomuto 排序结果");
		assert(is_sorted(arr));	//	验证排序正确性
	}

	// --- 测试 Hoare ---
	{
		std::vector<int> arr = original;	//	拷贝一份独立数据
		hoare_quicksort(arr, 0, static_cast<int>(arr.size()) - 1);
		print_array(arr, "Hoare 排序结果");
		assert(is_sorted(arr));	//	验证排序正确性
	}

	// --- 测试 DNF 三向切分 ---
	{
		std::vector<int> arr = original;	//	拷贝一份独立数据
		dnf_quicksort(arr, 0, static_cast<int>(arr.size()) - 1);
		print_array(arr, "DNF 排序结果");
		assert(is_sorted(arr));	//	验证排序正确性
	}

	std::cout << std::endl;
	return 0;
}
