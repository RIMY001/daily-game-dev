#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;

class Solution
{
public:
	vector<vector<string>> groupAnagrams(vector<string>& strs) {
		unordered_map<string, vector<string>> mp;

		for (const string& s : strs) {
			string key = s;
			sort(key.begin(), key.end()); // 排序后作为键
			mp[key].emplace_back(s);	  // 原始字符归入对应组
		}

		// 收集所有分组结果
		vector<vector<string>> result;
		result.reserve(mp.size());
		for (auto& [key, group] : mp) {
			result.emplace_back(std::move(group));
		}

		return result;

	}
};

// 辅助函数：格式化打印二维字符串向量
void printResult(const vector<vector<string>>& groups) {
	cout << "[";
	for (size_t i = 0; i < groups.size(); ++i) {
		cout << "[";
		for (size_t j = 0; j < groups[i].size(); ++j) {
			cout << "\"" << groups[i][j] << "\"";
			if (j + 1 < groups[i].size()) cout << ", ";
		}
		cout << "]";
		if (i + 1 < groups.size()) cout << ", ";
	}
	cout << "]" << endl;
}

int main()
{
	Solution sol;

	// 示例 1
	vector<string> strs1 = { "eat","tea","tan","ate","nat","bat" };
	cout << "输入： [\"eat\",\"tea\",\"tan\",\"ate\",\"nat\",\"bat\"]" << endl;
	cout << "输出： ";
	printResult(sol.groupAnagrams(strs1));

	// 示例 2
	vector<string> strs2 = { "" };
	cout << "\n输入： [\"\"]" << endl;
	cout << "输出： ";
	printResult(sol.groupAnagrams(strs2));

	// 示例 3
	vector<string> strs3 = { "a" };
	cout << "\n输入： [\"a\"]" << endl;
	cout << "输出： ";
	printResult(sol.groupAnagrams(strs3));

	system("pause");
	return 0;
}