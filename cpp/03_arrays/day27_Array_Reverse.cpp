#include <iostream>
using namespace std;
int main()
{
	int arr[7] = { 4,7,1,3,9,5,11 };
	
	cout << "Original array: " << endl;
	for (int i = 0; i < 7; i++)
	{
		cout << arr[i] << " ";
		
		
	}
	cout << endl;
	for (int i = 0; i < 7-1-i; i++)
	{
		int temp = arr[i];
		arr[i] = arr[7 - 1 - i];
		arr[7 - 1 - i] = temp;
	}
	cout << "Reversed array: " << endl;
	for (int i = 0; i < 7; i++)
	{
		cout << arr[i] << " ";
	}
	cout << endl;
	system("pause");
	return 0;
}