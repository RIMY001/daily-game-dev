#include <iostream>
#include <unordered_map>
#include <vector>
using namespace std;

class Solution {
public:
	vector<int> twoSum(vector<int>& nums, int target) {
		unordered_map<int, int> map;//值 -> 下标

		for (int i = 0; i < nums.size(); i++) {
			int complement = target - nums[i];

			//在已遍历的元素中查找互补值

			auto it = map.find(complement);
			if (it != map.end()) {
				return { it->second,i };
			}

			//未找到则存入哈希表，供后续元素查找
			map[nums[i]] = i;
		}

		return {};
	}
};

// =================== 本地测试辅助函数 ===================
void printVector(const vector<int>& v)
{
	cout << "[";
	for (size_t i = 0; i < v.size(); i++) {
		cout << v[i];
		if (i < v.size() - 1) cout << ", ";//避免尾随逗号
	}
	cout << "]" << endl;
}

// ============== main 函数（本地调试入口） ================
int main()
{
	Solution sol;

	// 测试用例 1
	vector<int> nums1 = { 2,7,11,15 };
	int target1 = 9;
	cout << "Test 1: ";
	printVector(sol.twoSum(nums1, target1));

	// 测试用例 2
	vector<int> nums2 = { 3,2,4 };
	int target2 = 6;
	cout << "Test 2: ";
	printVector(sol.twoSum(nums2, target2));

	// 测试用例 3(重复元素边界情况）
	vector<int> nums3 = { 3,3 };
	int target3 = 6;
	cout << "Test 3: ";
	printVector(sol.twoSum(nums3, target3));


	system("pause");
	return 0;
}