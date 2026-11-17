#include <iostream>
#include <vector>
#include <queue>
#include <utility> // 用于 std::pair

using namespace std;

class Solution {
public:
    int orangesRotting(vector<vector<int>>& grid) {
        // 处理空网格的边界情况
        if (grid.empty() || grid[0].empty()) {
            return 0;
        }

        int m = grid.size();
        int n = grid[0].size();
        queue<pair<int, int>> rottenQueue; // 存储腐烂橘子的坐标
        int freshOrangesCount = 0;         // 统计新鲜橘子的数量

        // 1. 遍历网格，初始化队列和新鲜橘子计数
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j) {
                if (grid[i][j] == 2) {
                    rottenQueue.push({ i, j }); // 将所有腐烂橘子加入队列
                }
                else if (grid[i][j] == 1) {
                    freshOrangesCount++;      // 统计新鲜橘子
                }
            }
        }

        // 特殊情况：如果没有新鲜橘子，则不需要时间
        if (freshOrangesCount == 0) {
            return 0;
        }

        int minutes = 0;
        // 四个方向：右、左、下、上
        int directions[4][2] = { { 0, 1 },{ 0, -1 },{ 1, 0 },{ -1, 0 } };

        // 2. 开始BFS扩散
        // 只要队列不为空且还有新鲜橘子，就继续扩散
        while (!rottenQueue.empty() && freshOrangesCount > 0) {
            int currentLevelSize = rottenQueue.size(); // 当前分钟时已腐烂的橘子数量
            bool infectedThisMinute = false;           // 标记这一分钟是否发生了感染

            // 处理当前这一层的所有腐烂橘子
            for (int i = 0; i < currentLevelSize; ++i) {
                pair<int, int> currentOrange = rottenQueue.front();
                rottenQueue.pop();
                int r = currentOrange.first;
                int c = currentOrange.second;

                // 检查四个相邻方向
                for (auto& dir : directions) {
                    int nr = r + dir[0]; // 新行坐标
                    int nc = c + dir[1]; // 新列坐标

                    // 检查新坐标是否在网格内且是新鲜橘子
                    if (nr >= 0 && nr < m && nc >= 0 && nc < n && grid[nr][nc] == 1) {
                        grid[nr][nc] = 2;          // 橘子变腐烂
                        freshOrangesCount--;       // 新鲜橘子数量减1
                        rottenQueue.push({ nr, nc }); // 新腐烂的橘子加入队列，用于下一分钟扩散
                        infectedThisMinute = true; // 标记本分钟发生了感染
                    }
                }
            }
            // 如果本分钟有橘子被感染，则时间加1
            if (infectedThisMinute) {
                minutes++;
            }
        }

        // 3. 最终判断
        // 如果新鲜橘子数量为0，说明全部腐烂，返回分钟数；否则说明有橘子无法被感染，返回-1
        return freshOrangesCount == 0 ? minutes : -1;
    }
};

// --- 以下为测试主函数，用于在VS中运行和验证 ---
int main() {
    Solution solution;

    // 测试用例1：示例1
    vector<vector<int>> grid1 = { { 2,1,1 },{ 1,1,0 },{ 0,1,1 } };
    cout << "Test Case 1 (Expected: 4): " << solution.orangesRotting(grid1) << endl;

    // 测试用例2：示例2
    vector<vector<int>> grid2 = { { 2,1,1 },{ 0,1,1 },{ 1,0,1 } }; // 左下角的橘子(2,0)是1，且被0和边界隔离，无法被感染
    cout << "Test Case 2 (Expected: -1): " << solution.orangesRotting(grid2) << endl;

    // 测试用例3：示例3
    vector<vector<int>> grid3 = { { 0,2 } }; // 没有新鲜橘子
    cout << "Test Case 3 (Expected: 0): " << solution.orangesRotting(grid3) << endl;

    // 测试用例4：只有腐烂橘子
    vector<vector<int>> grid4 = { { 2,2 },{ 2,2 } };
    cout << "Test Case 4 (Expected: 0): " << solution.orangesRotting(grid4) << endl;

    // 测试用例5：只有新鲜橘子
    vector<vector<int>> grid5 = { { 1,1 },{ 1,1 } };
    cout << "Test Case 5 (Expected: -1): " << solution.orangesRotting(grid5) << endl;

    // 测试用例6：单行单列
    vector<vector<int>> grid6 = { { 1 } };
    cout << "Test Case 6 (Expected: -1): " << solution.orangesRotting(grid6) << endl;
    vector<vector<int>> grid7 = { { 2 } };
    cout << "Test Case 7 (Expected: 0): " << solution.orangesRotting(grid7) << endl;
    vector<vector<int>> grid8 = { { 0 } };
    cout << "Test Case 8 (Expected: 0): " << solution.orangesRotting(grid8) << endl;

    return 0;
}