#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>
using namespace std;

class Solution {
public:
	int maxSubArray(vector<int>& nums) {
		assert(!nums.empty() && "输入数组不能为空");

		int currentSum = 0, globalMax = nums[0];	//	前者为以当前i结尾的最大子数和；后者为全局最大子数和

		for (const auto &x : nums) {
			currentSum = max(currentSum + x, x);	//	要么延续之前的，要么重新开始
			globalMax = max(globalMax, currentSum);	//	更新全局
		}

		return globalMax;
	}
};

//-------------  测试  -------------
void runTests() {
	Solution sol;
	//	测试1
	{
		vector<int> nums = { -2, 1, -3, 4, -1, 2, 1, -5, 4 };
		int result = sol.maxSubArray(nums);
		cout << "测试1： " << result << " （期望 6）" << endl;
		assert(result == 6);
	}

	//	测试2
	{
		vector<int> nums = {-3, -5, -1, -8 };
		int result = sol.maxSubArray(nums);
		cout << "测试2： " << result << " （期望 -1）" << endl;
		assert(result == -1);
	}

	//	测试3
	{
		vector<int> nums = {42};
		int result = sol.maxSubArray(nums);
		cout << "测试3： " << result << " （期望 42）" << endl;
		assert(result == 42);
	}
}

int main() {
	runTests();

	system("pause");
	return 0;
}