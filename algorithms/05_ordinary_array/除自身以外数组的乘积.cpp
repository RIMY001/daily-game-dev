#include <iostream>
#include <vector>
using namespace std;

class Solution {
public:
	vector<int> productExceptSelf(vector<int>& nums) {
		int n = nums.size();
		vector<int> answer(n);

		//	从左到右，answer[i]储存左侧前缀积
		answer[0] = 1;
		for (int i = 1; i < n; ++i) {
			answer[i] = answer[i - 1] * nums[i - 1];
		}

		//	从右到左,用变量r积累右侧后缀积，直接乘入answer[i]
		int r = 1;
		for (int i = n - 1; i >= 0; --i) {
			answer[i] *= r;
			r *= nums[i];
		}

		return answer;
	}
};

// =========================== 测试辅助代码 ===========================
void printVector(const vector<int>& v) {
	cout << "[";
	for (size_t i = 0; i < v.size(); ++i) {
		cout << v[i];
		if (i + 1 < v.size()) cout << ", ";
	}
	cout << "]" << endl;
}

int main() {
	Solution sol;

	//	示例 1
	vector<int> test1 = { 1,2,3,4 };
	cout << "输入： "; printVector(test1);
	vector<int> res1 = sol.productExceptSelf(test1);
	cout << "输出： "; printVector(res1);
	cout << endl;

	//	示例 2
	vector<int> test2 = { -1, 1, 0, -3, 3 };
	cout << "输入： "; printVector(test2);
	vector<int> res2 = sol.productExceptSelf(test2);
	cout << "输出： "; printVector(res2);
	cout << endl;

	system("pause");
	return 0;
}