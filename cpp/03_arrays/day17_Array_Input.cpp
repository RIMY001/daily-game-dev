#include <iostream>
#include <Windows.h>
using namespace std ;
int main()
{
	int arr[5];
	cout << "Enter 5 integers: " << endl;
	int len = sizeof(arr) / sizeof(arr[0]);
	for (int i = 0; i < len; i++)
	{
		cin >> arr[i];
	}
	cout << "Your arrays are: " << endl;
	for (int i = 0; i < len; i++)
	{
		cout << arr[i] << "\t"; 
	}
	cout << endl;
	system("pause");
	return 0;
}
