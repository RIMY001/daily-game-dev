#include <iostream>
using namespace std;
int main()
{
	int arr[5] = { 10,20,30,40,50 };
	cout << "Access by index: " << endl;
	cout << arr[0] <<" " << arr[1] << " " << arr[2] << " " << arr[3] << " " << arr[4] << endl;
	cout << "Traverse array with loop: " << endl;
	for (int i = 0; i < 5; i++)//使用sizeof()会更好
	{
		cout << arr[i] << "\t";
	}
	cout << endl;
	arr[2] = 99;
	cout << "After modify: " << arr[2] << endl;
	system("pause");
	return 0;
} 