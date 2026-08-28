#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>
#include <unordered_map>

using namespace std;

class Solution {
public:
	int lengthOfLongestSubstring(string s){
		unordered_map<char, int> charIndex;
		int left = 0, maxLen = 0;
		for (int right = 0; right < (int)s.size(); right++) {
			char c = s[right];
			if (charIndex.find(c) != charIndex.end() && charIndex[c] >= left) {
				left = charIndex[c] + 1;
			}
			charIndex[c] = right;
			maxLen = max(maxLen, right - left + 1);
		}
		return maxLen;
	}
};

void test(const string& input, int expected) {
	Solution sol;
	int result = sol.lengthOfLongestSubstring(input);

	cout << "输入：\"" << input << "\"" << endl;
	cout << "期望：" << expected << " | 实际：" << result << endl;
	cout << (result == expected ? "相等" : "不相等") << endl;
	cout << "----------------------------------------------------" << endl;
}

int main() {
	// LeetCode 官方示例
	test("abcabcbb", 3);
	test("bbbbb", 1);
	test("pwwkew", 3);

	// 边界用例
	test("", 0);           // 空字符串
	test("a", 1);          // 单字符
	test("ab", 2);         // 两个不同字符
	test("aa", 1);         // 两个相同字符
	test("abba", 2);       // 经典陷阱用例
	test(" ", 1);          // 空格
	test("dvdf", 3);       // 重复字符不在窗口开头

	system("pause"); // VS 控制台窗口暂停，防止闪退
	return 0;
}