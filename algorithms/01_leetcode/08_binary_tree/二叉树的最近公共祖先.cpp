#include <iostream>
#include <vector>
#include <queue>

// 定义二叉树节点
struct TreeNode {
    int val;
    TreeNode* left;
    TreeNode* right;
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
};

class Solution {
public:
    TreeNode* lowestCommonAncestor(TreeNode* root, TreeNode* p, TreeNode* q) {
        //  终止：空节点 / 找到p或者q
        if (root == nullptr || root == p || root == q) return root;
        // 后序：左、右
        TreeNode* leftRes = lowestCommonAncestor(root->left, p, q);
        TreeNode* rightRes = lowestCommonAncestor(root->right, p, q);
        // 左右都找到了 → 当前节点是最近公共祖先
        if (leftRes != nullptr && rightRes != nullptr) return root;
        // 哪边有值返回哪边，都没有返回null
        return leftRes ? leftRes : rightRes;
    }
};

// 辅助函数：根据层序遍历数组构建二叉树（用于测试）
TreeNode* buildTree(const std::vector<int>& nodes) {
    if (nodes.empty() || nodes[0] == -1) return nullptr; // -1 表示 null

    TreeNode* root = new TreeNode(nodes[0]);
    std::queue<TreeNode*> q;
    q.push(root);
    int i = 1;
    while (!q.empty() && i < nodes.size()) {
        TreeNode* current = q.front();
        q.pop();

        if (i < nodes.size() && nodes[i] != -1) {
            current->left = new TreeNode(nodes[i]);
            q.push(current->left);
        }
        i++;
        if (i < nodes.size() && nodes[i] != -1) {
            current->right = new TreeNode(nodes[i]);
            q.push(current->right);
        }
        i++;
    }
    return root;
}

// 辅助函数：查找值为 val 的节点（用于测试）
TreeNode* findNode(TreeNode* root, int val) {
    if (root == nullptr) return nullptr;
    if (root->val == val) return root;
    TreeNode* left = findNode(root->left, val);
    if (left != nullptr) return left;
    return findNode(root->right, val);
}

// 主函数：用于测试
int main() {
    // 示例 1: root = [3,5,1,6,2,0,8,null,null,7,4], p = 5, q = 1
    std::vector<int> tree1 = { 3,5,1,6,2,0,8,-1,-1,7,4 };
    TreeNode* root1 = buildTree(tree1);
    TreeNode* p1 = findNode(root1, 5);
    TreeNode* q1 = findNode(root1, 1);
    Solution sol;
    TreeNode* lca1 = sol.lowestCommonAncestor(root1, p1, q1);
    std::cout << "LCA of 5 and 1 is: " << lca1->val << std::endl; // 预期输出 3

    // 示例 2: p = 5, q = 4
    TreeNode* p2 = findNode(root1, 5);
    TreeNode* q2 = findNode(root1, 4);
    TreeNode* lca2 = sol.lowestCommonAncestor(root1, p2, q2);
    std::cout << "LCA of 5 and 4 is: " << lca2->val << std::endl; // 预期输出 5

    // 示例 3: root = [1,2], p = 1, q = 2
    std::vector<int> tree3 = { 1,2 };
    TreeNode* root3 = buildTree(tree3);
    TreeNode* p3 = findNode(root3, 1);
    TreeNode* q3 = findNode(root3, 2);
    TreeNode* lca3 = sol.lowestCommonAncestor(root3, p3, q3);
    std::cout << "LCA of 1 and 2 is: " << lca3->val << std::endl; // 预期输出 1

    return 0;
}