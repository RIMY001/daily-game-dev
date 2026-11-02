#include <iostream>
#include <queue>
#include <algorithm> // For std::swap

// 定义二叉树节点结构体
struct TreeNode {
    int val;
    TreeNode* left;
    TreeNode* right;
    TreeNode() : val(0), left(nullptr), right(nullptr) {}
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
    TreeNode(int x, TreeNode* left, TreeNode* right) : val(x), left(left), right(right) {}
};

// --- 解法1: 递归实现 ---
class SolutionRecursive {
public:
    TreeNode* invertTree(TreeNode* root) {
        // 基本情况：如果节点为空，直接返回
        if (root == nullptr) {
            return nullptr;
        }

        // 递归地翻转左子树和右子树
        TreeNode* left = invertTree(root->left);
        TreeNode* right = invertTree(root->right);

        // 交换左右子节点
        root->left = right;
        root->right = left;

        // 返回当前节点
        return root;
    }
};

// --- 解法2: 迭代实现 (BFS) ---
class SolutionIterative {
public:
    TreeNode* invertTree(TreeNode* root) {
        // 基本情况：如果树为空，直接返回
        if (root == nullptr) {
            return nullptr;
        }

        // 创建一个队列用于BFS
        std::queue<TreeNode*> q;
        q.push(root);

        while (!q.empty()) {
            TreeNode* current = q.front();
            q.pop();

            // 交换当前节点的左右子节点
            std::swap(current->left, current->right);

            // 如果左子节点不为空，加入队列
            if (current->left != nullptr) {
                q.push(current->left);
            }
            // 如果右子节点不为空，加入队列
            if (current->right != nullptr) {
                q.push(current->right);
            }
        }
        return root;
    }
};

// --- 辅助函数: 根据层序遍历数组创建树 ---
// 例如: {4,2,7,1,3,6,9}
TreeNode* createTree(const std::vector<int>& values) {
    if (values.empty()) {
        return nullptr;
    }

    TreeNode* root = new TreeNode(values[0]);
    std::queue<TreeNode*> q;
    q.push(root);
    int i = 1;

    while (!q.empty() && i < values.size()) {
        TreeNode* current = q.front();
        q.pop();

        // 处理左子节点
        if (i < values.size()) {
            if (values[i] != -1) { // 假设-1代表null
                current->left = new TreeNode(values[i]);
                q.push(current->left);
            }
            i++;
        }

        // 处理右子节点
        if (i < values.size()) {
            if (values[i] != -1) { // 假设-1代表null
                current->right = new TreeNode(values[i]);
                q.push(current->right);
            }
            i++;
        }
    }
    return root;
}

// --- 辅助函数: 层序遍历打印树 ---
void printTree(TreeNode* root) {
    if (root == nullptr) {
        std::cout << "[]";
        return;
    }

    std::vector<int> result;
    std::queue<TreeNode*> q;
    q.push(root);

    while (!q.empty()) {
        TreeNode* current = q.front();
        q.pop();

        if (current == nullptr) {
            // 对于非最后一层的nullptr，我们可能需要记录，但为了简化输出，这里只打印非空节点
            // 如果要精确匹配LeetCode的序列化格式，需要更复杂的逻辑
            continue;
        }

        result.push_back(current->val);

        if (current->left != nullptr || current->right != nullptr) {
            q.push(current->left);
            q.push(current->right);
        }
    }

    std::cout << "[";
    for (size_t i = 0; i < result.size(); ++i) {
        std::cout << result[i] << (i == result.size() - 1 ? "" : ",");
    }
    std::cout << "]";
}

// --- 主函数: 用于测试 ---
int main() {
    // 示例 1
    std::vector<int> input1 = { 4,2,7,1,3,6,9 };
    TreeNode* root1 = createTree(input1);
    std::cout << "--- 示例 1 ---" << std::endl;
    std::cout << "输入: ";
    printTree(root1);
    std::cout << std::endl;

    SolutionRecursive sol_rec;
    TreeNode* flipped_root1_rec = sol_rec.invertTree(root1);
    std::cout << "递归输出: ";
    printTree(flipped_root1_rec);
    std::cout << "  (时间复杂度: O(N), 空间复杂度: O(H))" << std::endl;

    // 重新创建树以测试迭代方法
    TreeNode* root1_iter = createTree(input1);
    SolutionIterative sol_iter;
    TreeNode* flipped_root1_iter = sol_iter.invertTree(root1_iter);
    std::cout << "迭代输出: ";
    printTree(flipped_root1_iter);
    std::cout << "  (时间复杂度: O(N), 空间复杂度: O(N))" << std::endl;
    std::cout << std::endl;

    // 示例 2
    std::vector<int> input2 = { 2,1,3 };
    TreeNode* root2 = createTree(input2);
    std::cout << "--- 示例 2 ---" << std::endl;
    std::cout << "输入: ";
    printTree(root2);
    std::cout << std::endl;

    TreeNode* flipped_root2_rec = sol_rec.invertTree(root2);
    std::cout << "递归输出: ";
    printTree(flipped_root2_rec);
    std::cout << std::endl;

    TreeNode* root2_iter = createTree(input2);
    TreeNode* flipped_root2_iter = sol_iter.invertTree(root2_iter);
    std::cout << "迭代输出: ";
    printTree(flipped_root2_iter);
    std::cout << std::endl;

    // 示例 3
    std::vector<int> input3 = {};
    TreeNode* root3 = createTree(input3);
    std::cout << "--- 示例 3 ---" << std::endl;
    std::cout << "输入: ";
    printTree(root3);
    std::cout << std::endl;

    TreeNode* flipped_root3_rec = sol_rec.invertTree(root3);
    std::cout << "递归输出: ";
    printTree(flipped_root3_rec);
    std::cout << std::endl;

    TreeNode* root3_iter = createTree(input3);
    TreeNode* flipped_root3_iter = sol_iter.invertTree(root3_iter);
    std::cout << "迭代输出: ";
    printTree(flipped_root3_iter);
    std::cout << std::endl;

    return 0;
}