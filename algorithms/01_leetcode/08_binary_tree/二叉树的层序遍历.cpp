#include <iostream>
#include <vector>
#include <queue>

// 定义二叉树节点结构
struct TreeNode {
    int val;
    TreeNode* left;
    TreeNode* right;
    TreeNode() : val(0), left(nullptr), right(nullptr) {}
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
    TreeNode(int x, TreeNode* left, TreeNode* right) : val(x), left(left), right(right) {}
};

class Solution {
public:
    std::vector<std::vector<int>> levelOrder(TreeNode* root) {
        std::vector<std::vector<int>> result;

        // 1. 边界条件：如果根节点为空，直接返回空结果
        if (root == nullptr) {
            return result;
        }

        // 2. 初始化队列，用于存储待遍历的节点
        std::queue<TreeNode*> q;
        q.push(root);

        // 3. 当队列不为空时，继续遍历
        while (!q.empty()) {
            // 获取当前层的节点数量
            int size = q.size();
            // 用于存储当前层节点值的列表
            std::vector<int> level;

            // 遍历当前层的所有节点
            for (int i = 0; i < size; ++i) {
                // 取出队列头部的节点
                TreeNode* node = q.front();
                q.pop(); // 弹出头部节点

                // 将当前节点的值加入当前层列表
                level.push_back(node->val);

                // 如果存在左子节点，加入队列
                if (node->left != nullptr) {
                    q.push(node->left);
                }
                // 如果存在右子节点，加入队列
                if (node->right != nullptr) {
                    q.push(node->right);
                }
            }

            // 将当前层的结果添加到最终结果中
            result.push_back(level);
        }

        return result;
    }
};

// 辅助函数：根据题目示例构建二叉树
int main() {
    Solution sol;

    TreeNode* root = new TreeNode(3);
    root->left = new TreeNode(9);
    root->right = new TreeNode(20);
    root->right->left = new TreeNode(15);
    root->right->right = new TreeNode(7);

    std::vector<std::vector<int>> output = sol.levelOrder(root);

    std::cout << "输出: [";
    for (size_t i = 0; i < output.size(); ++i) {
        std::cout << "[";
        for (size_t j = 0; j < output[i].size(); ++j) {
            std::cout << output[i][j];
            if (j < output[i].size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]";
        if (i < output.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;

    // 释放内存（简单示例，实际复杂树需要递归释放）
    delete root->left;
    delete root->right->left;
    delete root->right->right;
    delete root->right;
    delete root;

    return 0;
}