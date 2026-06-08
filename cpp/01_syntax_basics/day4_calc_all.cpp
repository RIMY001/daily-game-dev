#include <iostream>
#include <Windows.h>
using namespace std;
int main()
{
	double a, b;
	cout << "Enter the first number: ";
	cin >> a;
	cout << "Enter the second number: ";
	cin >> b;
	cout << "\n===== Calculation Results =====" << endl;
	cout << "Addition: " << a << " + " << b << " = " << a + b << endl;
	cout << "Subtraction: " << a << " - " << b << " = " << a - b << endl;
	cout << "Multiplication: " << a << " * " << b << " = " << a * b << endl;
	if (b != 0)
	{
		cout << "Division: " << a << " / " << b << " = " << a / b << endl;
	}
	else
	{
		cout << "Division: Division by zero is not allowed!" << endl;
	}
	if ((int)a == a && (int)b == b && (int)b != 0)
	{
		cout << "Modulus: " << (int)a << " % " << (int)b << " = " << (int)a % (int)b << endl;

	}
	else
	{
		cout << "Modulus: Modulus only works for integers, or divisor is zero!" << endl;
	}
	system("pause");
	return 0;

}