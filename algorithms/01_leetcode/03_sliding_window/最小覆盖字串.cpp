#include <iostream>
#include <string>
#include <unordered_map>
#include <climits>
using namespace std;

class Solution {
public:
	string minWindow(string s, string t) {
		unordered_map<char, int> need, window;	//	t中每种字符需要的个数；当前窗口中每种字符的个数
		for (char c : t) need[c]++;	

		int left = 0, formed = 0, minLen = INT_MAX, start = 0;	//	分别是：窗口左边界，已满足数量要求的字符种类数，最短合法窗口长度，最短窗口的起始下标

		for (int right = 0; right < (int)s.size(); right++) {
			//	右边界字符入窗
			char c = s[right];
			if (need.count(c)) {	//	只关心t中出现过的字符
				window[c]++;
				if (window[c] == need[c]) formed++;	//	该种类刚好达标
			}

			//	所有种类都达标时，尝试收缩左边界
			while (formed == (int)need.size()) {	//	显式将 size_t 转为 int，告诉编译器："我知道这里有无符号到有符号的窄化转换，
				//但我确认 map 的大小不会超过 INT_MAX（26个字母远小于21亿），这个转换是安全的。"
				
				//	先记录当前合法窗口（可能是目前最短的）
				if (right - left + 1 < minLen) {
					minLen = right - left + 1;
					start = left;
				}

				//	左边界字符移出窗口
				char d = s[left];
				if (need.count(d)) {
					window[d]--;
					if (window[d] < need[d]) formed--;	//	该种类跌破需求量
				}
				left++;
			}
		}

		return minLen == INT_MAX ? "" : s.substr(start, minLen);
	}
};

int main() {
	Solution sol;

	// 示例1
	string s1 = "ADOBECODEBANC", t1 = "ABC";
	cout << "s = " << s1 << " , t = " << t1 << endl;
	cout << "输出： " << sol.minWindow(s1, t1) << endl << endl;

	//	示例2
	string s2 = "a", t2 = "a";
	cout << "s = " << s2 << " , t = " << t2 << endl;
	cout << "输出： " << sol.minWindow(s2, t2) << endl << endl;

	//	示例3
	string s3 = "a", t3 = "aa";
	cout << "s = " << s3 << " , t = " << t3 << endl;
	cout << "输出： \"" << sol.minWindow(s3, t3) << "\"" << endl << endl;

	system("pause");
	return 0;
}