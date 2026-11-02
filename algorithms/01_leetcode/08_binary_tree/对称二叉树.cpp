#include <iostream>
#include <queue>

//	二叉树节点定义
struct TreeNode {
    int val;
    TreeNode* left;
    TreeNode* right;
    TreeNode() : val(0), left(nullptr), right(nullptr) {}
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
    TreeNode(int x, TreeNode* left, TreeNode* right) : val(x), left(left), right(right) {}
};

//	方案一：递归法（时间O(n)空间O(h)）
class Solution1 {
public:
	bool isSymmetric(TreeNode* root) {
        if (root == nullptr) return true;

        //  辅助函数，判断子树是不是镜像
        return check(root->left, root->right);
	}

    bool check(TreeNode* p, TreeNode* q) {
        if (p == nullptr && q == nullptr) return true;
        if (p == nullptr || q == nullptr) return false;
        return p->val == q->val && check(p->left, q->right) && check(p->right, q->left);
    }
};

//	方案二：迭代法 （时间O(n)空间O(n)）
class Solution2 {
public:
	bool isSymmetric(TreeNode* root) {
        if (root == nullptr) return true;

        std::queue<TreeNode*> q;
        q.push(root->left);
        q.push(root->right);
        while (!q.empty()) {
            auto p = q.front(); q.pop();
            auto t = q.front(); q.pop();

            if (p == nullptr && t == nullptr) continue;
            if (p == nullptr || p == nullptr) return false;
            if (p->val != t->val) return false;
            //  按镜像顺序交叉压入队列
            q.push(p->left); q.push(t->right);
            q.push(p->right);q.push(t->left);
        }
        return true;
	}
};
// ==================== 4. 辅助函数：释放树内存（防止内存泄漏）====================
void deleteTree(TreeNode* root) {
    if (!root) return;
    deleteTree(root->left);
    deleteTree(root->right);
    delete root;
}

// ==================== 5. 主函数：测试用例 ====================
int main() {
    // 【示例 1】 root = [1,2,2,3,4,4,3] (对称树)
    TreeNode* root1 = new TreeNode(1);
    root1->left = new TreeNode(2);
    root1->right = new TreeNode(2);
    root1->left->left = new TreeNode(3);
    root1->left->right = new TreeNode(4);
    root1->right->left = new TreeNode(4);
    root1->right->right = new TreeNode(3);

    // 【示例 2】 root = [1,2,2,null,3,null,3] (非对称树)
    TreeNode* root2 = new TreeNode(1);
    root2->left = new TreeNode(2);
    root2->right = new TreeNode(2);
    root2->left->right = new TreeNode(3);
    root2->right->right = new TreeNode(3);

    Solution1 sol_recursive;
    Solution2 sol_iterative;

    // 测试递归法
    std::cout << "=== 方案一：递归法 ===" << std::endl;
    std::cout << "示例 1 输出: " << (sol_recursive.isSymmetric(root1) ? "true" : "false") << std::endl;
    std::cout << "示例 2 输出: " << (sol_recursive.isSymmetric(root2) ? "true" : "false") << std::endl;

    // 测试迭代法
    std::cout << std::endl << "=== 方案二：迭代法 (队列) ===" << std::endl;
    std::cout << "示例 1 输出: " << (sol_iterative.isSymmetric(root1) ? "true" : "false") << std::endl;
    std::cout << "示例 2 输出: " << (sol_iterative.isSymmetric(root2) ? "true" : "false") << std::endl;

    // 释放动态分配的内存
    deleteTree(root1);
    deleteTree(root2);

    return 0;
}