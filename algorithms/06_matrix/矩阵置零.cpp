#include <iostream>
#include <vector>
#include <iomanip> // 用于格式化输出

using namespace std;

class Solution {
public:
	void setZeroes(vector<vector<int>>& matrix) {
        if (matrix.empty() || matrix[0].empty()) return;

        int m = matrix.size();
        int n = matrix[0].size();

        // 步骤1：单独记录首行首列原始是否含0
        bool firstRowZero = false;
        bool firstColZero = false;

        for (int j = 0; j < n; ++j) {
            if (matrix[0][j] == 0) {
                firstRowZero = true;
                break;
            }
        }
        for (int i = 0; i < m; ++i) {
            if (matrix[i][0] == 0) {
                firstColZero = true;
                break;
            }
        }

        // 步骤2：用首行首列做标记（从索引1开始，避免覆盖原始状态）
        for (int i = 1; i < m; ++i) {
            for (int j = 1; j < n; ++j) {
                if (matrix[i][j] == 0) {
                    matrix[i][0] = 0; // 标记第 i 行需要清零
                    matrix[0][j] = 0; // 标记第 j 列需要清零
                }
            }
        }

        // 步骤3：根据标记清零内部区域（从索引1开始）
        for (int i = 1; i < m; ++i) {
            for (int j = 1; j < n; ++j) {
                if (matrix[i][0] == 0 || matrix[0][j] == 0) {
                    matrix[i][j] = 0;
                }
            }
        }

        // 步骤4：最后处理首行首列（必须在步骤3之后！）
        if (firstRowZero) {
            for (int j = 0; j < n; ++j) {
                matrix[0][j] = 0;
            }
        }
        if (firstColZero) {
            for (int i = 0; i < m; ++i) {
                matrix[i][0] = 0;
            }
        }
	}
};

// ==================== 测试辅助函数 ====================
void printMatrix(const vector<vector<int>>& matrix, const string& label) {
    cout << label << endl;
    for (const auto& row : matrix) {
        cout << "  [";
        for (size_t j = 0; j < row.size(); ++j) {
            cout << setw(2) << row[j];
            if (j < row.size() - 1) cout << ", ";
        }
        cout << "]" << endl;
    }
    cout << endl;
}

int main() {
    Solution sol;

    // 测试用例1
    vector<vector<int>> matrix1 = {
        {1, 1, 1},
        {1, 0, 1},
        {1, 1, 1}
    };
    printMatrix(matrix1, "测试1 - 输入:");
    sol.setZeroes(matrix1);
    printMatrix(matrix1, "测试1 - 输出 (期望: [1,0,1],[0,0,0],[1,0,1]):");

    // 测试用例2
    vector<vector<int>> matrix2 = {
        {0, 1, 2, 0},
        {3, 4, 5, 2},
        {1, 3, 1, 5}
    };
    printMatrix(matrix2, "测试2 - 输入:");
    sol.setZeroes(matrix2);
    printMatrix(matrix2, "测试2 - 输出 (期望: [0,0,0,0],[0,4,5,0],[0,3,1,0]):");

    // 边界测试：首行首列本身含0
    vector<vector<int>> matrix3 = {
        {0, 1, 2},
        {3, 4, 5},
        {6, 7, 0}
    };
    printMatrix(matrix3, "边界测试 - 输入 (首行首列+右下角含0):");
    sol.setZeroes(matrix3);
    printMatrix(matrix3, "边界测试 - 输出 (期望: 整行整列均正确清零):");

    system("pause"); // VS控制台暂停，防止窗口闪退
    return 0;
}