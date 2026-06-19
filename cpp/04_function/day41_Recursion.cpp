//联想出来的问题1.递归与循环各自的优劣；2.数组传递的本质是指针传递；3.普通数组与vector容器；4.std::vector与std::array;5.指针传递与引用传递；6.std::array与普通数组

#include <iostream>
using namespace std;

//arr数组，下标从0到n-1求和
int getSum(int arr[], int n)
{
	//递归出口：没有元素
	if (n == 0)
		return 0;
	//递推:当前元素+前面所有元素的和
	return arr[n - 1] + getSum(arr, n - 1);
}

int main()
{
	int a[5] = { 1,2,3,4,5 };
	cout << getSum(a, 5) << endl;

	system("pause");
	return 0;
}