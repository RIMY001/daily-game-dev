#include <iostream>
#include <vector>
#include <iomanip>

using namespace std;

class Solution {
public:
    vector<int> spiralOrder(vector<vector<int>>& matrix) {
        vector<int> result;
        if (matrix.empty() || matrix[0].empty()) return result;

        int top = 0, bottom = matrix.size() - 1;
        int left = 0, right = matrix[0].size() - 1;

        while (top <= bottom && left <= right) {
            // 1. 从左到右遍历顶行
            for (int j = left; j <= right; ++j)
                result.push_back(matrix[top][j]);
            ++top;

            // 2. 从上到下遍历右列
            for (int i = top; i <= bottom; ++i)
                result.push_back(matrix[i][right]);
            --right;

            // 3. 从右到左遍历底行（必须检查防止重复）
            if (top <= bottom) {
                for (int j = right; j >= left; --j)
                    result.push_back(matrix[bottom][j]);
                --bottom;
            }

            // 4. 从下到上遍历左列（必须检查防止重复）
            if (left <= right) {
                for (int i = bottom; i >= top; --i)
                    result.push_back(matrix[i][left]);
                ++left;
            }
        }
        return result;
    }
};

// ==================== 测试辅助函数 ====================
void printMatrix(const vector<vector<int>>& matrix) {
    for (const auto& row : matrix) {
        cout << "  [";
        for (size_t j = 0; j < row.size(); ++j) {
            cout << setw(3) << row[j];
            if (j < row.size() - 1) cout << ",";
        }
        cout << " ]" << endl;
    }
}

void printResult(const vector<int>& res) {
    cout << "  输出: [";
    for (size_t i = 0; i < res.size(); ++i) {
        cout << res[i];
        if (i < res.size() - 1) cout << ", ";
    }
    cout << "]" << endl << endl;
}

int main() {
    Solution sol;

    // 测试1: 标准 3x3
    vector<vector<int>> m1 = { {1,2,3},{4,5,6},{7,8,9} };
    cout << "测试1 (3x3):" << endl;
    printMatrix(m1);
    printResult(sol.spiralOrder(m1));

    // 测试2: 3x4 矩形
    vector<vector<int>> m2 = { {1,2,3,4},{5,6,7,8},{9,10,11,12} };
    cout << "测试2 (3x4):" << endl;
    printMatrix(m2);
    printResult(sol.spiralOrder(m2));

    // 测试3: 单行矩阵（易错点）
    vector<vector<int>> m3 = { {1, 2, 3} };
    cout << "测试3 (单行):" << endl;
    printMatrix(m3);
    printResult(sol.spiralOrder(m3));

    // 测试4: 单列矩阵（易错点）
    vector<vector<int>> m4 = { {1},{2},{3} };
    cout << "测试4 (单列):" << endl;
    printMatrix(m4);
    printResult(sol.spiralOrder(m4));

    // 测试5: 1x1 矩阵
    vector<vector<int>> m5 = { {42} };
    cout << "测试5 (1x1):" << endl;
    printMatrix(m5);
    printResult(sol.spiralOrder(m5));

    system("pause");
    return 0;
}