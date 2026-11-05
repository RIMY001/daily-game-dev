#include <iostream>
#include <vector>
#include <climits> // 用于 LLONG_MIN, LLONG_MAX
#include <algorithm> // 用于 std::vector

using namespace std;

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
    // --- 方法一：递归传递上下界 (推荐) ---

    // 辅助递归函数，带上下界参数
    // minVal: 当前节点值必须严格大于此值
    // maxVal: 当前节点值必须严格小于此值
    bool isValidBSTHelper(TreeNode* root, long long minVal, long long maxVal) {
        // 空节点视为有效BST
        if (root == nullptr) {
            return true;
        }

        // 检查当前节点值是否在合法范围内
        // 注意是严格大于minVal且严格小于maxVal
        if (root->val <= minVal || root->val >= maxVal) {
            return false;
        }

        // 递归检查左右子树，并更新边界
        // 左子树：上界变为当前节点值 (root->val)，下界不变
        // 右子树：下界变为当前节点值 (root->val)，上界不变
        return isValidBSTHelper(root->left, minVal, root->val) &&
            isValidBSTHelper(root->right, root->val, maxVal);
    }

    bool isValidBST(TreeNode* root) {
        // 初始调用，根节点无限制，范围为(long long)最小值到最大值
        // 使用long long是为了避免节点值等于INT_MIN或INT_MAX时与边界冲突
        return isValidBSTHelper(root, LLONG_MIN, LLONG_MAX);
    }

    // --- 方法二：中序遍历 ---

    void inorderTraversal(TreeNode* root, vector<int>& result) {
        if (root == nullptr) {
            return;
        }
        inorderTraversal(root->left, result);  // 遍历左子树
        result.push_back(root->val);           // 访问根节点
        inorderTraversal(root->right, result); // 遍历右子树
    }

    bool isValidBSTInorder(TreeNode* root) {
        vector<int> traversalResult;
        inorderTraversal(root, traversalResult);

        // 检查遍历结果是否严格递增
        for (size_t i = 1; i < traversalResult.size(); ++i) {
            if (traversalResult[i] <= traversalResult[i - 1]) { // 必须严格大于前一个
                return false;
            }
        }
        return true;
    }
};

// --- 主函数，用于本地测试 ---
int main() {
    Solution sol;

    // 示例1: [2,1,3] -> true
    //     2
    //    / \
    //   1   3
    TreeNode* root1 = new TreeNode(2, new TreeNode(1), new TreeNode(3));
    cout << "示例1 (root=[2,1,3]): " << (sol.isValidBST(root1) ? "true" : "false") << endl; // 应输出 true
    // 清理内存 (简单示例，实际项目需用智能指针或析构函数)
    delete root1->left; delete root1->right; delete root1;

    // 示例2: [5,1,4,null,null,3,6] -> false
    //     5
    //    / \
    //   1   4
    //      / \
    //     3   6
    TreeNode* root2 = new TreeNode(5,
        new TreeNode(1),
        new TreeNode(4, new TreeNode(3), new TreeNode(6)));
    cout << "示例2 (root=[5,1,4,null,null,3,6]): " << (sol.isValidBST(root2) ? "true" : "false") << endl; // 应输出 false
    delete root2->left; delete root2->right->left; delete root2->right->right; delete root2->right; delete root2;

    // 自定义测试: 根10，左5(右12) -> false (12在左子树但大于10)
    //      10
    //     /
    //    5
    //     \
    //      12
    TreeNode* root3 = new TreeNode(10, new TreeNode(5, nullptr, new TreeNode(12)), nullptr);
    cout << "自定义测试 (root=[10,5,null,12]): " << (sol.isValidBST(root3) ? "true" : "false") << endl; // 应输出 false
    delete root3->left->right; delete root3->left; delete root3;

    // 自定义测试: 单节点 -> true
    TreeNode* root4 = new TreeNode(1);
    cout << "单节点测试 (root=[1]): " << (sol.isValidBST(root4) ? "true" : "false") << endl; // 应输出 true
    delete root4;

    // 自定义测试: 相等节点 -> false (左子树必须严格小于)
    //    5
    //   /
    //  5
    TreeNode* root5 = new TreeNode(5, new TreeNode(5), nullptr);
    cout << "相等节点测试 (root=[5,5]): " << (sol.isValidBST(root5) ? "true" : "false") << endl; // 应输出 false
    delete root5->left; delete root5;

    return 0;
}