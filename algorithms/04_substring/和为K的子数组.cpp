#include <iostream>
#include <vector>
#include <unordered_map>

using namespace std;

class Solution {
public:
	int subarraySum(vector<int>& nums, int k) {
		unordered_map<int, int> prefixCount;	//	前缀和 -> 出现次数
		prefixCount[0] = 1;	//	前缀和为0出现1次（空前缀，处理从下标0开始的子数组）

		int sum = 0;	//	当前前缀和
		int count = 0;	//	结果

		for (int num : nums) {
			sum += num;

			//	查找之前有多少个前缀和 == sum - k
			auto it = prefixCount.find(sum - k);
			if (it != prefixCount.end()) {
				count += it->second;
			}

			//	将当前前缀和加入哈希表
			prefixCount[sum]++;
		}

		return count;
	}
};

void test(vector<int> nums, int k, int expected) {
	Solution sol;
	int res = sol.subarraySum(nums, k);
	cout << "k=" << k << " 期望：" << expected
		<< " 实际：" << res
		<< (res == expected ? " 相等" : " 不相等") << endl;
}

int main() {
	test({ 1,1,1 }, 2, 2);
	test({ 1,2,3 }, 3, 2);
	test({ 1,-1,1,-1,1 }, 0, 6);
	test({ 0,0,0 }, 0, 6);
	test({ 1 }, 1, 1);
	test({ 1 }, 2, 0);
	test({ -1,-1,-1 }, -2, 2);

	system("pause");
	return 0;
}