//将数组遍历、冒泡排序封装成独立函数，告别重复代码，工程化写法
#include <iostream>
using namespace std;
//原始代码存在以下问题：
//1. * *全局变量滥用 * *：`arr`、`len` 和 `temp` 被定义为全局变量，导致函数耦合度高且不安全。
//2. * *函数签名不规范 * *：`printAllArr` 和 `BubbleSortArr` 的参数中数组大小写死为 `int arr[5]`，限制了通用性。
//3. * *临时变量位置不当 * *：`temp` 作为全局变量在排序函数中使用，应改为局部变量。
int arr[5] = { 1,6,8,2,4 };
const int len = 5;
int temp;
//数组遍历
void printAllArr(int arr[5],int len)//正确写法应该是(int arr[],int len)
//数组做函数参数，[]里不能填变量
//写成(int arr[5],len)语法允许，但毫无意义。5只是摆设，编译器会直接忽略
//为什么要单独传len?函数内部无法计算数组长度，编译器会把int arr[]当成指针int *arr，只传首元素地址，注意区分&arr是数组指针类型和int*类型不匹配。强行把&arr传给int*形参会报错
{
	for (int i = 0;i < len;i++)
	{
		cout << arr[i] << " ";
	}
	cout << endl;
}
//冒泡排序
void BubbleSortArr(int arr[5],int len)
{
	//int temp;
	for (int i = 0;i < len -1;i++)
	{
		for (int j = 0;j < len - i - 1;j++)
		{
			if (arr[j] > arr[j + 1])
			{
				//前后顺序颠倒，取temp为临时存储变量
				temp = arr[j + 1];
				arr[j + 1] = arr[j];
				arr[j] = temp;
			}
		}
	}
}
int main()
{
	//int arr[5] = { 1,6,8,2,4 };
	//const int len = 5;
	cout << "Original array: " << endl;
	printAllArr(arr,len);
	BubbleSortArr(arr,len);
	cout << "排序后的数组: " << endl;
	printAllArr(arr, len);
	system("pause");
	return 0;
}