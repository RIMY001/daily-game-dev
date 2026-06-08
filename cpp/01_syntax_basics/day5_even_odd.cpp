#include <iostream>
#include <Windows.h>
using namespace std;
int main()
{
	int num;
	cout << "Enter a number: " << endl;
	cin >> num;
	if( num > 0 )
	{ 
		cout << "It's positive." << endl;
	}
	else if (num == 0)
	{
		cout << "I's zero." << endl;
	}
	else
	{
		cout << "It's negative." << endl;

	}
	if (num % 2 == 0)
	{
		cout << "It's even." << endl;


	}
	else
	{
		cout << "It's odd." << endl;
	}
	system ("pause");
	return 0;
}