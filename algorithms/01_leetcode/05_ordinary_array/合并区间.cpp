#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

class Solution {
public:
	vector<vector<int>> merge(vector<vector<int>>& intervals) {
		if (intervals.empty()) return {};

		//	先进行排列，vector<int>默认使用字典序，先比较左端点，在比较右端点（升序）
		sort(intervals.begin(), intervals.end());
		vector<vector<int>> merged;

		for (size_t i = 0; i < intervals.size(); ++i) {
			int L = intervals[i][0], R = intervals[i][1];

			//	若merged为空或者当前区间与合并区间不重叠
			if (!merged.size() || merged.back()[1] < L) {
				merged.push_back({ L,R });	//	将当前区间作为合并前区间的末尾区间
			}
			//	有重叠则更新右边界，取更大的为右边界
			else {
				merged.back()[1] = max(merged.back()[1], R);
			}
		}
		return merged;
	}
};

//	================================ 测试 ================================
void printIntervals(const vector<vector<int>>& intervals) {
	cout << "[";
	for (size_t i = 0; i < intervals.size(); ++i) {
		cout << "[" << intervals[i][0] << "," << intervals[i][1] << "]";
		if (i + 1 < intervals.size()) cout << ",";
	}
	cout << "]" << endl;
}

int main() {
	Solution sol;

	//	示例1
	vector<vector<int>> test1 = { {1,3},{2,6},{8,10},{15,18} };
	cout << "输入： "; printIntervals(test1);
	cout << "输出： "; printIntervals(sol.merge(test1));
	cout << endl;

	//	示例2
	vector<vector<int>> test2 = { {1,4}, {4,5} };
	cout << "输入： "; printIntervals(test2);
	cout << "输出： "; printIntervals(sol.merge(test2));
	cout << endl;

	system("pause");
	return 0;
}