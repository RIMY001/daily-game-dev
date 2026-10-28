#include <iostream>
#include <algorithm>
#include <queue>

struct TreeNode {
	int val;
	TreeNode* left;
	TreeNode* right;
	TreeNode() : val(0), left(nullptr), right(nullptr) {}
	TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
	TreeNode(int x, TreeNode* left, TreeNode* right) : val(x), left(left), right(right) {}
};

class Solution1 {
public:
	int maxDepth(TreeNode* root) {
		if (root == nullptr) return 0;

		int leftDepth = maxDepth(root->left);
		int rightDepth = maxDepth(root->right);
		return std::max(leftDepth, rightDepth) + 1;
	}
};

class Solution2 {
public:
	int maxDepth(TreeNode* root) {
		if (root == nullptr) return 0;

		std::queue<TreeNode*> q;
		q.push(root);
		int depth = 0;

		while (!q.empty()) {
			int size = q.size();    //  当前层的节点数量
			depth++;   //  每处理一层，深度 +1

			//  将当前层的所有节点出队，并将它们的子节点入队
			for (int i = 0; i < size; ++i) {
				TreeNode* node = q.front();
				q.pop();
				if (node->left) q.push(node->left);
				if (node->right) q.push(node->right);
			}
		}
		return depth;
	}
};

int main() {
	TreeNode* root = new TreeNode(3);
	root->left = new TreeNode(9);
	root->right = new TreeNode(20);
	root->right->left = new TreeNode(15);
	root->right->right = new TreeNode(7);

	Solution1 sol1;
	int depth1 = sol1.maxDepth(root);
	std::cout << "二叉树的最大深度为(递归）: " << depth1 << std::endl; // 期望输出: 3
	Solution2 sol2;
	int depth2 = sol2.maxDepth(root);
	std::cout << "二叉树的最大深度为(迭代（层序遍历））: " << depth2 << std::endl; // 期望输出: 3

	// 释放内存
	delete root->right->right;
	delete root->right->left;
	delete root->right;
	delete root->left;
	delete root;

	system("pause");
	return 0;
}