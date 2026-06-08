#include <iostream>
#include <Windows.h>
using namespace std;
int main()
{
	int arr[] = { 10,20,30,40,50 };
	int len = sizeof(arr) / sizeof(arr[0]);
	int sum = 0;
	for (int i = 0; i < len; i++)
	{
		sum = sum + arr[i];
	}
	cout << "Sum of array is: " << sum << endl;
	system("pause");
	return 0;
}