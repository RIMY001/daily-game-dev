#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

class Solution {
public:
	int firstMissingPositive(vector<int>& nums) {
		int n = nums.size();

		//	进行循环，让每一个i回到i+1的正确位置
		for (int i = 0; i < n; ++i) {
			while (nums[i] >= 1 && nums[i] <= n && nums[nums[i] - 1] != nums[i]) {
				swap(nums[i], nums[nums[i] - 1]);
			}
		}

		//	第一个不在i位置的i+1就是要找的值
		for (int i = 0; i < n; ++i) {
			if (nums[i] != i + 1) { return i + 1; }
		}

		//	遍历全部，没找到一定就是n+1
		return n + 1;
	}
};

//	============================ 测试代码 ============================ 
void printVector(const vector<int>& v) {
	cout << "[";
	for (size_t i = 0; i < v.size(); ++i) {
		cout << v[i];
		if (i + 1 < v.size()) cout << ", ";
	}
	cout << "]";
}

int main() {
	Solution sol;

	//	示例1
	vector<int> test1 = { 1, 2, 0 };
	cout << "输入: "; printVector(test1);
	cout << " → 输出: " << sol.firstMissingPositive(test1) << endl;

	//  示例 2
	vector<int> test2 = { 3, 4, -1, 1 };
	cout << "输入: "; printVector(test2);
	cout << " → 输出: " << sol.firstMissingPositive(test2) << endl;

	//  示例 3
	vector<int> test3 = { 7, 8, 9, 11, 12 };
	cout << "输入: "; printVector(test3);
	cout << " → 输出: " << sol.firstMissingPositive(test3) << endl;

	//  边界：含重复值（验证防死循环）
	vector<int> test4 = { 1, 1, 1, 1 };
	cout << "输入: "; printVector(test4);
	cout << " → 输出: " << sol.firstMissingPositive(test4) << endl;

	//  边界：完美排列 [1..n]
	vector<int> test5 = { 1, 2, 3, 4, 5 };
	cout << "输入: "; printVector(test5);
	cout << " → 输出: " << sol.firstMissingPositive(test5) << endl;

	//  边界：单元素
	vector<int> test6 = { 2 };
	cout << "输入: "; printVector(test6);
	cout << " → 输出: " << sol.firstMissingPositive(test6) << endl;

	system("pause");
	return 0;

}