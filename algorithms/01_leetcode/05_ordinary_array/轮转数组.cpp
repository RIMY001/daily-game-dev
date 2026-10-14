#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

class Solution {
public:
	void rotate(vector<int>& nums, int k) {
		int n = (int)nums.size();
		// 【关键】k 取模，避免无效轮转和越界
		k %= n;
		if (k == 0) return;

		reverse(nums.begin(), nums.end());
		reverse(nums.begin(), nums.begin() + k);
		reverse(nums.begin() + k, nums.end());
	}
};

// ====================================== 测试 ======================================
void printVector(const vector<int>& v) {
	cout << "[";
	for (size_t i = 0; i < v.size(); i++) {
		cout << v[i];
		if (i + 1 < v.size()) cout << ",";
	}
	cout << "]" << endl;
}

int main() {
	Solution sol;

	//示例1：
	vector<int> test1 = { 1,2,3,4,5,6,7 };
	cout << "输入： "; printVector(test1);
	sol.rotate(test1, 3);
	cout << "k = 3 输出： "; printVector(test1);
	cout << endl;

	//示例2：
	vector<int> test2 = { -1, -100, 3, 99 };
	cout << "输入： "; printVector(test2);
	sol.rotate(test2, 2);
	cout << "k = 3 输出： "; printVector(test2);
	cout << endl;

	system("pause");
	return 0;
}