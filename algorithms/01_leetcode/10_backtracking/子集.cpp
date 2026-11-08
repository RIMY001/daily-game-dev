#include <iostream>
#include <vector>

using namespace std;

class Solution {
public:
    vector<vector<int>> subsets(vector<int>& nums) {
        vector<vector<int>> result; // 用于存放所有的子集结果
        vector<int> current_subset; // 用于记录当前正在构建的子集

        // 从索引 0 开始回溯
        backtrack(nums, 0, current_subset, result);

        return result;
    }

private:
    // 回溯函数
    // index: 当前正在考察 nums 数组中的第几个元素
    void backtrack(vector<int>& nums, int index, vector<int>& current_subset, vector<vector<int>>& result) {
        // 1. 终止条件：当考察完所有元素时，将当前构建的子集加入结果集
        if (index == nums.size()) {
            result.push_back(current_subset); // 此时会拷贝一份 current_subset 放入 result
            return;
        }

        // 2. 决策一：不选当前元素 (nums[index])
        // 直接递归处理下一个元素，current_subset 不变
        backtrack(nums, index + 1, current_subset, result);

        // 3. 决策二：选当前元素 (nums[index])
        // 将当前元素加入正在构建的子集中
        current_subset.push_back(nums[index]);
        // 递归处理下一个元素
        backtrack(nums, index + 1, current_subset, result);

        // 4. 回溯（撤销选择）：为了不影响后续的其他分支（例如回到“不选”的状态），需要弹出刚才加入的元素
        current_subset.pop_back();
    }
};

// --- 辅助函数：用于在控制台打印二维向量（子集列表） ---
void printSubsets(const vector<vector<int>>& subsets) {
    cout << "输出: [";
    for (size_t i = 0; i < subsets.size(); ++i) {
        cout << "[";
        for (size_t j = 0; j < subsets[i].size(); ++j) {
            cout << subsets[i][j];
            if (j < subsets[i].size() - 1) {
                cout << ", ";
            }
        }
        cout << "]";
        if (i < subsets.size() - 1) {
            cout << ", ";
        }
    }
    cout << "]" << endl;
}

// --- 主函数：用于本地测试 ---
int main() {
    Solution sol;

    // 示例 1
    vector<int> nums1 = { 1, 2, 3 };
    cout << "示例 1:" << endl;
    cout << "输入: nums = [1, 2, 3]" << endl;
    vector<vector<int>> res1 = sol.subsets(nums1);
    printSubsets(res1);

    cout << "----------------------" << endl;

    // 示例 2
    vector<int> nums2 = { 0 };
    cout << "示例 2:" << endl;
    cout << "输入: nums = [0]" << endl;
    vector<vector<int>> res2 = sol.subsets(nums2);
    printSubsets(res2);

    return 0;
}