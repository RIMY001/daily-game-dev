#include <iostream>
#include <vector>

using namespace std;

class Solution {
public:
    vector<vector<int>> permute(vector<int>& nums) {
        vector<vector<int>> result; // 存放最终的所有排列结果
        vector<int> path;           // 存放当前正在构建的排列路径
        vector<bool> used(nums.size(), false); // 标记数字是否已被使用

        backtrack(nums, path, used, result);
        return result;
    }

private:
    void backtrack(vector<int>& nums, vector<int>& path, vector<bool>& used, vector<vector<int>>& result) {
        // 终止条件：当前路径的长度等于数组的长度，说明构建完了一个完整的全排列
        if (path.size() == nums.size()) {
            result.push_back(path); // 将当前路径的一个副本加入到结果集中
            return;
        }

        // 遍历数组中的每一个数字，尝试将其加入当前路径
        for (int i = 0; i < nums.size(); ++i) {
            // 如果当前数字已经被使用过，跳过
            if (used[i]) {
                continue;
            }

            // 1. 做选择：将当前数字加入路径，并标记为已使用
            path.push_back(nums[i]);
            used[i] = true;

            // 2. 递归进入下一层，继续寻找下一个位置的数字
            backtrack(nums, path, used, result);

            // 3. 撤销选择（回溯）：将当前数字移除，并标记为未使用
            //    这样在下一轮循环中，该数字才能被其他分支再次使用
            path.pop_back();
            used[i] = false;
        }
    }
};

// --- 辅助函数：用于在控制台打印二维数组（全排列结果） ---
void printPermutations(const vector<vector<int>>& permutations) {
    cout << "输出: [";
    for (size_t i = 0; i < permutations.size(); ++i) {
        cout << "[";
        for (size_t j = 0; j < permutations[i].size(); ++j) {
            cout << permutations[i][j];
            if (j < permutations[i].size() - 1) {
                cout << ", ";
            }
        }
        cout << "]";
        if (i < permutations.size() - 1) {
            cout << ", ";
        }
    }
    cout << "]" << endl;
}

// --- 主函数：用于本地测试 ---
int main() {
    Solution sol;

    // 示例 1: nums = [1, 2, 3]
    vector<int> nums1 = { 1, 2, 3 };
    cout << "示例 1:" << endl;
    cout << "输入: nums = [1, 2, 3]" << endl;
    vector<vector<int>> res1 = sol.permute(nums1);
    printPermutations(res1); // 预期输出包含6种排列

    cout << "----------------------" << endl;

    // 示例 2: nums = [0, 1]
    vector<int> nums2 = { 0, 1 };
    cout << "示例 2:" << endl;
    cout << "输入: nums = [0, 1]" << endl;
    vector<vector<int>> res2 = sol.permute(nums2);
    printPermutations(res2); // 预期输出包含2种排列

    cout << "----------------------" << endl;

    // 示例 3: nums = [1]
    vector<int> nums3 = { 1 };
    cout << "示例 3:" << endl;
    cout << "输入: nums = [1]" << endl;
    vector<vector<int>> res3 = sol.permute(nums3);
    printPermutations(res3); // 预期输出包含1种排列

    return 0;
}