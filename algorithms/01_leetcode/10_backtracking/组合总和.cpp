#include <iostream>
#include <vector>
#include <algorithm>
#include <ranges> // 引入 ranges 库，C++20/17 支持

// 1. 核心解题类 
class Solution {
public:
    std::vector<std::vector<int>> combinationSum(std::vector<int>& candidates, int target) {
        // 先对数组进行排序，这是回溯剪枝的必要前提
        std::sort(candidates.begin(), candidates.end());

        std::vector<std::vector<int>> results; // 存储所有合法的组合
        std::vector<int> currentCombination;   // 存储当前正在探索的组合

        // 开始回溯
        backtrack(candidates, target, 0, currentCombination, results);

        return results;
    }

private:
    void backtrack(const std::vector<int>& candidates, int remainingTarget, int startIndex,
        std::vector<int>& currentCombination, std::vector<std::vector<int>>& results) {
        // 基线条件 1: 剩余目标值为 0，说明找到了一个合法组合
        if (remainingTarget == 0) {
            results.push_back(currentCombination);
            return;
        }

        // 遍历候选数组，从 startIndex 开始以避免产生重复的组合（如 [2,3] 和 [3,2]）
        for (int i = startIndex; i < candidates.size(); ++i) {
            // 剪枝: 因为数组是升序的，如果当前数字已经大于剩余目标值，后面的数字肯定也大了，直接跳出循环
            if (candidates[i] > remainingTarget) {
                break;
            }

            // 做出选择: 将 candidates[i] 加入当前组合
            currentCombination.push_back(candidates[i]);

            // 递归探索: 继续用当前数字 candidates[i] 去尝试 (允许重复使用，所以索引仍是 i)
            backtrack(candidates, remainingTarget - candidates[i], i, currentCombination, results);

            // 撤销选择: 回溯，将 candidates[i] 从当前组合中移除
            currentCombination.pop_back();
        }
    }
};

// 2. 打印二维向量的辅助函数
void printCombinations(const std::vector<std::vector<int>>& combinations) {
    std::cout << "[";
    bool firstOuter = true;
    for (const auto& combo : combinations) {
        if (!firstOuter) {
            std::cout << ", ";
        }
        std::cout << "[";
        bool firstInner = true;
        for (int num : combo) {
            if (!firstInner) {
                std::cout << ",";
            }
            std::cout << num;
            firstInner = false;
        }
        std::cout << "]";
        firstOuter = false;
    }
    std::cout << "]" << std::endl;
}

// 3. 主函数 (Main Driver)
int main() {
    // 测试用例 1
    std::vector<int> candidates1 = { 2, 3, 6, 7 };
    int target1 = 7;

    // 测试用例 2
    std::vector<int> candidates2 = { 2, 3, 5 };
    int target2 = 8;

    Solution solution;

    std::cout << "Test Case 1: candidates = [2,3,6,7], target = 7\n";
    auto res1 = solution.combinationSum(candidates1, target1);
    printCombinations(res1);

    std::cout << "\nTest Case 2: candidates = [2,3,5], target = 8\n";
    auto res2 = solution.combinationSum(candidates2, target2);
    printCombinations(res2);

    return 0;
}