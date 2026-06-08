#include <iostream>
#include <Windows.h>
using namespace std;
int main()
{
	int arr[] = { 11,22,33,44,55,66 };
	int len = sizeof(arr) / sizeof(arr[0]);
	cout << " All elements in array: " << endl;
	for (int i = 0; i < len; i++)
	{
		cout << arr[i] << "\t";

	}
	cout << endl;
	system("pause");
	return 0;
	
}