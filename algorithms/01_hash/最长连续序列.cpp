#include <iostream>
#include <vector>
#include <unordered_set>
#include <algorithm>
using namespace std;

class Solution
{
public:
	int longestConsecutive(vector<int>& nums) {
		// 1.将所有数字放入哈希集合，实现O(1)查找并自动去重
		unordered_set<int> numSet(nums.begin(), nums.end());

		int maxLen = 0;

		// 2.遍历集合中的每个数字
		for (int num : numSet) {
			// 3.核心优化：只有当 num - 1 不存在时， num 才是连续序列的起点
			//   非起点的数字直接跳过，保证整体均摊O(n)
			if (numSet.find(num - 1) == numSet.end()) {
				int curNum = num;
				int curLen = 1;

				// 4.从起点开始，向后逐个查找连续数字
				while (numSet.find(curNum + 1) != numSet.end()) {
					curNum++;
					curLen++;
				}

				// 5.更新最长序列长度
				maxLen = max(maxLen, curLen);
			}
		}

		return maxLen;
	}
};

// 辅助函数：打印一维整数向量
void printVector(const vector<int>& nums) {
	cout << "[";
	bool first = true;
	for (int num : nums) {
		if (!first) cout << ", ";
		first = false;
		cout << num;
	}
	cout << "]";
}

int main()
{
	Solution sol;
	// 示例 1
	vector<int> nums1 = { 100,4,200,1,3,2 };
	cout << "输入: ";
	printVector(nums1);
	cout << "\n输出: " << sol.longestConsecutive(nums1) << endl;

	// 示例 2
	vector<int> nums2 = { 0,3,7,2,5,8,4,6,0,1 };
	cout << "\n输入：";
	printVector(nums2);
	cout << "\n输出: " << sol.longestConsecutive(nums2) << endl;

	// 示例 3
	vector<int> nums3{ 1,0,1,2 };
	cout << "\n输入: ";
	printVector(nums3);
	cout << "\n输出: " << sol.longestConsecutive(nums3) << endl;

	// 边界测试：空数组
	vector<int> nums4{};
	cout << "\n输入: ";
	printVector(nums4);
	cout << "\n输出: " << sol.longestConsecutive(nums4) << endl;

	system("pause");
	return 0;
}