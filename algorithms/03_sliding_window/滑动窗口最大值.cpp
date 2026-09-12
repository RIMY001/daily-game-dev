#include <iostream>
#include <vector>
#include <deque>
using namespace std;

class Solution {
public:
	vector<int> maxSlidingWindow(vector<int>& nums, int k) {
		deque<int> dq;	//	存下标，对应值单调递减
		vector<int> res;

		for (int i = 0; i < nums.size(); i++) {
			//	淘汰队尾比当前小的
			while (!dq.empty() && nums[dq.back()] < nums[i])
				dq.pop_back();

			//	当前下标入队
			dq.push_back(i);

			//	队头过期则弹出
			if (dq.front() < i - k + 1)
				dq.pop_front();

			//	窗口形成后记录最大值
			if (i >= k - 1)
				res.push_back(nums[dq.front()]);

		}
		return res;
	}
};

int main() {
	Solution sol;

	vector<int> nums1 = { 1,3,-1,-3,5,3,6,7 };
	int k1 = 3;
	vector<int> res1 = sol.maxSlidingWindow(nums1, k1);
	cout << "示例1输出：";
	for (int x : res1) cout << x << " ";
	cout << endl;	//	期望：3 3 5 5 6 7

	vector<int> nums2 = { 1 };
	int k2 = 1;
	vector<int> res2 = sol.maxSlidingWindow(nums2, k2);
	cout << "示例2输出：";
	for (int x : res2) cout << x << " ";
	cout << endl;	//	期望：1

	system("pause");
	return 0;
}
