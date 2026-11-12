#include <iostream>
#include <vector>
#include <string>

class Solution {
public:
    std::vector<std::string> generateParenthesis(int n) {
        std::vector<std::string> result;
        std::string current;
        // 调用回溯函数，初始左括号和右括号使用数均为 0
        backtrack(result, current, 0, 0, n);
        return result;
    }

private:
    // 回溯函数
    // result: 存储所有有效组合的向量
    // current: 当前正在构建的字符串
    // left: 已使用的左括号数量
    // right: 已使用的右括号数量
    // n: 总括号对数
    void backtrack(std::vector<std::string>& result, std::string& current, int left, int right, int n) {
        // 终止条件：当字符串长度达到 2n 时（即 left == n 且 right == n）
        if (current.length() == 2 * n) {
            result.push_back(current);
            return;
        }

        // 1. 如果左括号数量小于 n，可以添加左括号
        if (left < n) {
            current.push_back('(');
            backtrack(result, current, left + 1, right, n);
            current.pop_back(); // 回溯，撤销上一步
        }

        // 2. 如果右括号数量小于左括号数量，可以添加右括号
        if (right < left) {
            current.push_back(')');
            backtrack(result, current, left, right + 1, n);
            current.pop_back(); // 回溯，撤销上一步
        }
    }
};

int main() {
    Solution sol;
    int n = 3;
    std::vector<std::string> res = sol.generateParenthesis(n);

    std::cout << "n = " << n << " 时的有效括号组合：" << std::endl;
    for (const std::string& s : res) {
        std::cout << "\"" << s << "\"" << (s == res.back() ? "" : ", ");
    }
    std::cout << std::endl;

    n = 1;
    res = sol.generateParenthesis(n);
    std::cout << "n = " << n << " 时的有效括号组合：" << std::endl;
    for (const std::string& s : res) {
        std::cout << "\"" << s << "\"" << std::endl;
    }

    return 0;
}