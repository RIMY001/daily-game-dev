#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

class Solution {
public:
	int trap(vector<int>& height) {
		//	边界条件：如果柱子子数量小于3，无法形成凹槽接水
		if (height.size() < 3) return 0;

		int left = 0;	//	左指针，从最左边开始
		int right = height.size() - 1;	//	右指针，从最右边开始

		int leftMax = 0;	//	记录左指针左边（含）的最高柱子
		int rightMax = 0;	//记录右指针右边（含）的最高柱子

		int water = 0;	//	总接水量

		//	当左右指针未相遇时，持续向中间移动
		while (left < right) {
			//	更新左右两侧的最大高度
			leftMax = max(leftMax, height[left]);
			rightMax = max(rightMax, height[right]);

			//	核心逻辑：木桶效应，水量由较低的一侧决定
			if (leftMax < rightMax) {
				//	左边较低，计算左指针位置的积水量
				water += (leftMax - height[left]);
				left++;	//	左指针右移
			}
			else
			{
				//	右边较低或相等，计算右指针位置的积水量
				water += (rightMax - height[right]);
				right--;	//	右指针左移
			}
		}
		

		return water;
	}
};

//	辅助函数：打印向量 （用于测试）
void printVector(const vector<int>& nums) {
	cout << "[";
	bool first = true;
	for (int num : nums) {
		if (!first) cout << ", ";
		first = false;
		cout << num;
	}
	cout << "]";
}

int main() {
	Solution sol;

	vector<int> h1 = { 0,1,0,2,1,0,1,3,2,1,2,1 };
	cout << "输入： ";
	printVector(h1);
	cout << "\n输出： " << sol.trap(h1) << "（预期： 6)\n\n";

	vector<int> h2 = { 4,2,0,3,2,5 };
	cout << "输入： ";
	printVector(h2);
	cout << "\n输出： " << sol.trap(h2) << "（预期： 9)\n";

	system("pause");
	return 0;
}