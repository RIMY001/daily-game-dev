#include <iostream>
#include <vector>
#include <string>
using namespace std;

class Solution {
public:
	vector<int> findAnagrams(string s, string p) {
		vector<int> result;
		int sLen = s.size(), pLen = p.size();

		//	p比s长，不可能有异位词
		if (pLen > sLen) return result;

		//	need[i]:p中字符i的频率 - 窗口中字符i的频率
		//	need[i]==0 表示字符i已匹配
		int need[26] = { 0 };
		int matchCount = 0;	//	need[i]==0 的种类数

		//	初始化：统计p的频率
		for (char c : p) {
			need[c - 'a']++;	//	C/C++ 中 char 本质是整数（ASCII码），
								//两个 char 相减得到它们的编码差值。因为题目保证 p 只含小写字母 'a'~'z'，
								//所以 c - 'a' 的结果一定在 [0, 25] 范围内，恰好对应 need[26] 的合法下标。
		}
		//	初始时，need[i]==0 的种类数
		for (int i = 0; i < 26; i++) {
			if (need[i] == 0) matchCount++;
		}

		//	滑动窗口 [left,right]
		int left = 0;
		for (int right = 0; right < sLen; right++) {
			//	=== 右边界字符进入窗口 ===
			int rIdx = s[right] - 'a';
			need[rIdx]--;
			if (need[rIdx] == 0) matchCount++;	//	刚好匹配了
			else if (need[rIdx] == -1) matchCount--;	//	从匹配变成多一个

			//	=== 窗口超过pLen，左边界字符移出窗口 ===
			if (right - left + 1 > pLen) {
				int lIdx = s[left] - 'a';
				need[lIdx]++;
				if (need[lIdx] == 0) matchCount++;	//	刚好匹配了
				else if (need[rIdx] == -1) matchCount--;	//	从匹配变成少一个
				left++;
			}

			//	=== 窗口大小恰为pLen且全部匹配 ===
			if (right - left + 1 == pLen && matchCount == 26) {
				result.push_back(left);
			}

		}
		
		return result;
	}
};

void test(const string& s, const string& p, const vector<int>& expected) {
	Solution sol;
	auto res = sol.findAnagrams(s, p);

	cout << "s=\"" << s << "\",p=\"" << p << "\"" << endl;
	cout << "期望:[";
	for (int i = 0; i < (int)expected.size(); i++) {
		if (i) cout << ","; cout << res[i];
	}
	cout << "]" << (res == expected ? " 相等" : " 不相等") << endl;
	cout << "-----------------------------------------" << endl;
}

int main() {
	test("cbaebabacd", "abc", { 0,6 });
	test("abab", "ab", { 0,1,2 });
	test("aaa", "aa", { 0,1 });
	test("ab", "ba", { 0 });
	test("ab", "abc", {});
	test("", "a", {});
	test("aaaaaaaaaa", "a", { 0,1,2,3,4,5,6,7,8,9 });

	system("pause");
	return 0;
}