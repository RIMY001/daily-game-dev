#include <iostream>
#include <Windows.h>
using namespace std;

int main()
{
	
int a, b;
	cout << "Enter two numbers: ";
	cin >> a >> b;
	cout << "Sum: " << a + b << endl;
	cout << "Difference: " << a - b << endl;
	cout << "Product: " << a * b << endl;
	system("pause");
	return 0;
}