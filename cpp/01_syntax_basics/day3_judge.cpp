#include <iostream>
#include <Windows.h>
using namespace std;
int main()
{
	int num;
	cout << "Enter an integer: ";
	cin >> num;
	if (num > 0)
	{
		cout << "You entered a positive number" << endl;

	}
	else if (num < 0)
	{
		cout << "You entered a negative number" << endl;

	}
	else
	{
		cout << " You entered zero " << endl;
	}
	system("pause");
	return 0;
}