#include <iostream>
#include <vector>
#include <functional>

//	Solution 类实现岛屿数量逻辑
class Solution {
public:
	int numIslands(std::vector<std::vector<char>>& grid) {
		if (grid.empty() || grid[0].empty()) return 0;

		int rows = grid.size();
		int cols = grid[0].size();
		int count = 0;

		//	定义深度优先探索（DFS）Lambda 函数
		std::function<void(int, int)> dfs = [&](int r, int c) {
			//	边界检查与条件检测
			if (r < 0 || r >= rows || c < 0 || c >= cols || grid[r][c] == '0') {
				return;
			}

			//	标记当前陆地已访问
			grid[r][c] = '0';

			//	向四个方向递归探索
			dfs(r + 1, c);
			dfs(r - 1, c);
			dfs(r, c + 1);
			dfs(r, c - 1);
		};

		//	遍历整个网格
		for (int r = 0; r < rows; ++r) {
			for (int c = 0; c < cols; ++c) {
				if (grid[r][c] == '1') {
					count++;	//	发现新岛屿
					dfs(r, c);	//触发 DFS ,淹没这座岛
				}
			}
		}
		return count;
	}
};

// 主函数用于构建数据与测试
int main() {
	Solution solution;

	//	测试1
	std::vector<std::vector<char>> grid1 = {
		{'1', '1', '1', '1', '0'},
		{'1', '1', '0', '1', '0'},
		{'1', '1', '0', '0', '0'},
		{'0', '0', '0', '0', '0'}
	};
	std::cout << "示例 1 输出：" << solution.numIslands(grid1) << std::endl;

	std::vector<std::vector<char>> grid2 = {
		{'1', '1', '0', '0', '0'},
		{'1', '1', '0', '0', '0'},
		{'0', '0', '1', '0', '0'},
		{'0', '0', '0', '1', '1'}
	};
	std::cout << "示例 2 输出: " << solution.numIslands(grid2) << std::endl;

	system("pause"); 
	return 0;
}