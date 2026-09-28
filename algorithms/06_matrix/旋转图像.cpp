#include <iostream>
#include <vector>
#include <iomanip>

using namespace std;

class Solution {
public:
    void rotate(vector<vector<int>>& matrix) {
        int n = matrix.size();
        if (n == 0) return;

        // 步骤1: 沿主对角线转置（只遍历上三角，避免交换两次恢复原状）
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                swap(matrix[i][j], matrix[j][i]);
            }
        }

        // 步骤2: 每行左右翻转（只遍历左半部分，避免翻转两次恢复原状）
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n / 2; ++j) {
                swap(matrix[i][j], matrix[i][n - 1 - j]);
            }
        }
    }
};

// ==================== 测试辅助函数 ====================
void printMatrix(const vector<vector<int>>& matrix, const string& label) {
    cout << label << endl;
    if (matrix.empty()) {
        cout << "  (空矩阵)" << endl << endl;
        return;
    }
    for (const auto& row : matrix) {
        cout << "  [";
        for (size_t j = 0; j < row.size(); ++j) {
            cout << setw(3) << row[j];
            if (j < row.size() - 1) cout << ",";
        }
        cout << " ]" << endl;
    }
    cout << endl;
}

int main() {
    Solution sol;

    // 测试1: 标准 3x3
    vector<vector<int>> m1 = { {1,2,3},{4,5,6},{7,8,9} };
    printMatrix(m1, "测试1 原始 (3x3):");
    sol.rotate(m1);
    printMatrix(m1, "测试1 旋转后:");

    // 测试2: 4x4
    vector<vector<int>> m2 = { {5,1,9,11},{2,4,8,10},{13,3,6,7},{15,14,12,16} };
    printMatrix(m2, "测试2 原始 (4x4):");
    sol.rotate(m2);
    printMatrix(m2, "测试2 旋转后:");

    // 测试3: 1x1 矩阵（边界情况）
    vector<vector<int>> m3 = { {42} };
    printMatrix(m3, "测试3 原始 (1x1):");
    sol.rotate(m3);
    printMatrix(m3, "测试3 旋转后:");

    // 测试4: 2x2 矩阵（最小偶数阶）
    vector<vector<int>> m4 = { {1,2},{3,4} };
    printMatrix(m4, "测试4 原始 (2x2):");
    sol.rotate(m4);
    printMatrix(m4, "测试4 旋转后:");

    system("pause");
    return 0;
}