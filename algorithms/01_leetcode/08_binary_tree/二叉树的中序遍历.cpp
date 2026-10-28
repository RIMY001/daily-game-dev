#include <iostream>
#include <vector>
#include <stack>

//	二叉树节点定义
struct TreeNode {
	int val;
	TreeNode* left;
	TreeNode* right;
	TreeNode() : val(0), left(nullptr), right(nullptr) {}
	TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
	TreeNode(int x, TreeNode* left, TreeNode* right) : val(x), left(left), right(right) {}
};

//	递归实现
class SolutionRecursive {
public:
	std::vector<int> inorderTraversal(TreeNode* root) {
		std::vector<int> result;
		inorder(root, result);
		return result;
	}

private:
	void inorder(TreeNode* node, std::vector<int>& res) {
		if (node == nullptr) return;

		//	左子树遍历
		inorder(node->left, res);
		//	访问根节点
		res.push_back(node->val);
		//	右子树遍历
		inorder(node->right, res);
	}
};

//	迭代法（使用栈）实现
class SolutionIterative {
public:
	std::vector<int> inorderTraversal(TreeNode* root) {
		std::vector<int> result;
		std::stack<TreeNode*> stk;
		TreeNode* curr = root;

		while (curr != nullptr || !stk.empty()) {
			//	一路向左，将所有有左子节点入栈
			while (curr != nullptr) {
				stk.push(curr);
				curr = curr->left;
			}

			//	弹出栈顶（最左节点）并访问
			curr = stk.top();
			stk.pop();
			result.push_back(curr->val);

			//	转向右子树
			curr = curr->right;
		}
		return result;
	}
};

// _______________ 辅助函数 _______________
void printVector(const std::vector<int>& vec) {
	std::cout << "[";
	for (size_t i = 0; i < vec.size(); ++i) {
		std::cout << vec[i];
		if (i < vec.size() - 1) std::cout << ",";
	}
	std::cout << "]" << std::endl;
}

//	_______________ 主函数 _______________
int main() {
	//	构建二叉树
	TreeNode* root = new TreeNode(1);
	root->right = new TreeNode(2);
	root->right->left = new TreeNode(3);

	//	测试递归法
	SolutionRecursive sol1;
	std::vector<int> res1 = sol1.inorderTraversal(root);
	std::cout << "递归法结果：";
	printVector(res1);

	//	测试迭代法
	SolutionIterative sol2;
	std::vector<int> res2 = sol2.inorderTraversal(root);
	std::cout << "迭代法结果: ";
	printVector(res2);

	//	释放内存
	delete root->right->left;
	delete root->right;
	delete root;
	//顺序现在是对的（先子后父），但如果树结构变了（比如 `root->right` 是空），直接访问 `root->right->left` 会空指针崩溃。

	return 0;
}