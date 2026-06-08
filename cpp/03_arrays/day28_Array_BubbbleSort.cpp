#include <iostream>
using namespace std;
int main()
{
	int arr[8];
	int temp;
	//让用户输入数组元素
	cout << "Please enter numbers to fill the array: " << endl;
	for (int i = 0; i < 8; i++)
	{
		cin >> arr[i];
		
	}
	//打印初始数组
	cout << "Original Array: " << endl;
	for (int i = 0; i < 8; i++)
	{
		cout << arr[i] << " ";

	}
	cout << endl;
	//进行冒泡排序
	for (int i = 0; i < 8 - 1; i++)
	{
		for (int j = 0; j < 8 - i - 1; j++)
		{
			if (arr[j] > arr[j + 1])
			{
				temp = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1] = temp;
			}
		}
	}
	//打印排序后的数组
	cout << "Array : " << endl;
	for (int i = 0; i < 8; i++)
	{
		cout << arr[i] << " ";
	}
	system("pause");
	return 0;
}