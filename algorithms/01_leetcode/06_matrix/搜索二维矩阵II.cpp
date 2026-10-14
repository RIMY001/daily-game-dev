#include <iostream>
#include <vector>
#include <iomanip>
#include <string>
using namespace std;

class Solution {
public:
	bool searchMatrix(vector<vector<int>>& matrix, int target) {
		if (matrix.empty() || matrix[0].empty()) return false;

		int m = matrix.size(), n = matrix[0].size();

		//	从右上角搜索
		int row = 0, col = n - 1;
		while (row < m && col >= 0) {
			int cur = matrix[row][col];
			if (cur == target) return true;
			else if (cur > target) {
				--col;	//	当前值太大，排除当前列
			}
			else {
				++row;	//	当前值太小，排除当前行
			}
		}
		return false;
	}
};

// ========================= 辅助函数 =========================
void printMatrix(const vector<vector<int>>& matrix) {
	if (matrix.empty()) {
		cout << " []" << endl;
		return;
	}
	for (const auto& row : matrix) {
		cout << " [";
		for (size_t j = 0; j < row.size(); ++j) {
			cout << setw(3) << row[j];
			if (j < row.size() - 1) cout << ",";
		}
		cout << " ]" << endl;
	}
}

void runTest(Solution& sol, vector<vector<int>> matrix, int target, bool expected) {
	cout << "矩阵：" << endl;
	printMatrix(matrix);
	cout << "目标值：" << target << endl;

	bool result = sol.searchMatrix(matrix, target);

	cout << "输出：" << (result ? "true" : "false");
	cout << " | 期望：" << (expected ? "true" : "false");
	cout << " | " << (result == expected ? "通过" : "失败") << endl;
	cout << "-----------------------------------------------------" << endl;
}

int main() {
	Solution sol;

	// 测试1: 题目示例 - 存在目标值
	runTest(sol, { {1,4,7,11,15},{2,5,8,12,19},{3,6,9,16,22},{10,13,14,17,24},{18,21,23,26,30} }, 5, true);

	// 测试2: 题目示例 - 不存在目标值
	runTest(sol, { {1,4,7,11,15},{2,5,8,12,19},{3,6,9,16,22},{10,13,14,17,24},{18,21,23,26,30} }, 20, false);

	// 测试3: 空矩阵 (外层为空)
	runTest(sol, {}, 1, false);

	// 测试4: 有行但列为空 (内层为空)
	runTest(sol, { {} }, 1, false);

	// 测试5: 1x1 矩阵 - 命中
	runTest(sol, { {42} }, 42, true);

	// 测试6: 1x1 矩阵 - 未命中
	runTest(sol, { {42} }, 10, false);

	// 测试7: 搜索右上角元素本身
	runTest(sol, { {1,4,7,11,15},{2,5,8,12,19},{3,6,9,16,22},{10,13,14,17,24},{18,21,23,26,30} }, 15, true);

	// 测试8: 搜索左下角元素本身
	runTest(sol, { {1,4,7,11,15},{2,5,8,12,19},{3,6,9,16,22},{10,13,14,17,24},{18,21,23,26,30} }, 18, true);

	system("pause");
	return 0;
}