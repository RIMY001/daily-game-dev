#include <iostream>
#include <vector>
#include <string>

using namespace std;

class Solution {
private:
    // 数字到字母的映射表，索引对应数字
    // 0和1不对应字母，留空占位
    const vector<string> phoneMap = {
        "",     // 0
        "",     // 1
        "abc",  // 2
        "def",  // 3
        "ghi",  // 4
        "jkl",  // 5
        "mno",  // 6
        "pqrs", // 7
        "tuv",  // 8
        "wxyz"  // 9
    };

    // 回溯函数
    // digits: 输入的数字字符串
    // index: 当前处理到digits中的第几个数字（索引）
    // currentCombination: 当前已构建的字母组合
    // result: 存储所有最终组合的结果容器（引用传递）
    void backtrack(const string& digits, int index, string currentCombination, vector<string>& result) {
        // 终止条件：如果当前索引等于数字字符串长度，说明所有数字都已处理完毕
        if (index == digits.length()) {
            result.push_back(currentCombination); // 将完整组合加入结果集
            return;
        }

        // 获取当前数字字符，并将其转换为整数索引
        char digitChar = digits[index];
        int digit = digitChar - '0'; // 减去 `'0'`，相当于减去基准偏移，得到对应的整型数值

        // 获取该数字对应的字母字符串
        const string& letters = phoneMap[digit];

        // 遍历当前数字对应的所有字母
        for (char letter : letters) {
            // 递归调用：索引加1，当前组合追加当前字母
            // 注意：currentCombination + letter 是按值传递，递归返回后不影响本轮循环的currentCombination
            backtrack(digits, index + 1, currentCombination + letter, result);
        }
    }

public:
    vector<string> letterCombinations(string digits) {
        vector<string> result;

        // 边界情况处理：如果输入为空字符串，则没有字母组合
        if (digits.empty()) {
            return result; // 返回空vector
        }

        // 开始回溯搜索，从索引0开始，当前组合为空字符串
        backtrack(digits, 0, "", result);

        return result;
    }
};

// ================= 本地测试入口 =================
int main() {

    Solution solution;

    // 测试用例 1
    string digits1 = "23";
    vector<string> result1 = solution.letterCombinations(digits1);

    cout << "输入: digits = \"" << digits1 << "\"" << endl;
    cout << "输出: [";
    for (size_t i = 0; i < result1.size(); ++i) {
        cout << "\"" << result1[i] << "\"";
        if (i < result1.size() - 1) cout << ",";
    }
    cout << "]" << endl << endl;

    // 测试用例 2
    string digits2 = "2";
    vector<string> result2 = solution.letterCombinations(digits2);

    cout << "输入: digits = \"" << digits2 << "\"" << endl;
    cout << "输出: [";
    for (size_t i = 0; i < result2.size(); ++i) {
        cout << "\"" << result2[i] << "\"";
        if (i < result2.size() - 1) cout << ",";
    }
    cout << "]" << endl;

    // 保持控制台窗口不自动关闭
    cout << "\n按任意键退出...";
    cin.get();
    return 0;
}