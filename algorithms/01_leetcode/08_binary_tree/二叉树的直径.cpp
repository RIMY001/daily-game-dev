#include <iostream>
#include <algorithm> // 用于 std::max

// LeetCode 提供的二叉树节点定义
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
    // 用于存储最大直径的变量
    int max_diameter;

    int diameterOfBinaryTree(TreeNode* root) {
        max_diameter = 0; // 每次调用时重置
        dfs(root);
        return max_diameter;
    }

private:
    // 深度优先搜索辅助函数
    // 返回值：以当前节点为根的子树的最大深度
    int dfs(TreeNode* node) {
        // 递归终止条件：节点为空，深度为0
        if (node == nullptr) {
            return 0;
        }

        // 递归计算左子树的深度
        int left_depth = dfs(node->left);
        // 递归计算右子树的深度
        int right_depth = dfs(node->right);

        // 经过当前节点的路径长度 = 左子树深度 + 右子树深度
        // 更新全局最大直径
        max_diameter = std::max(max_diameter, left_depth + right_depth);

        // 返回当前子树的深度
        return 1 + std::max(left_depth, right_depth);
    }
};

// --- 以下为在 Visual Studio 中测试用的主函数 ---
int main() {
    // 示例 1: root = [1,2,3,4,5]
    // 构造二叉树:
    //     1
    //    / \
    //   2   3
    //  / \
    // 4   5
    TreeNode* root1 = new TreeNode(1);
    root1->left = new TreeNode(2);
    root1->right = new TreeNode(3);
    root1->left->left = new TreeNode(4);
    root1->left->right = new TreeNode(5);

    Solution sol1;
    int result1 = sol1.diameterOfBinaryTree(root1);
    std::cout << "示例 1 输出: " << result1 << std::endl; // 预期输出: 3

    // 示例 2: root = [1,2]
    // 构造二叉树:
    //   1
    //  /
    // 2
    TreeNode* root2 = new TreeNode(1);
    root2->left = new TreeNode(2);

    Solution sol2;
    int result2 = sol2.diameterOfBinaryTree(root2);
    std::cout << "示例 2 输出: " << result2 << std::endl; // 预期输出: 1

    // 清理内存 (在实际项目中建议使用智能指针)
    delete root1->left->left;
    delete root1->left->right;
    delete root1->left;
    delete root1->right;
    delete root1;

    delete root2->left;
    delete root2;

    return 0;
}