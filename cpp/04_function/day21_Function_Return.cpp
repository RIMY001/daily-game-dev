#include <iostream>
using namespace std;
int getSum(int a, int b)
{
	int result = a + b;
	return result;
}
int getMax(int m, int n)
{
	if (m > n)
	{
		return m;
	}
	else
	{
		return n;
	}
}
int main()
{
	int sum = getSum(12, 28);
	cout << "Total sum: " << sum << endl;
	int maxNum = getMax(56, 33);
	cout << "Max number: " << maxNum << endl;
	cout << "Direct calculate: " << getSum(9, 16) << endl;
	system ("pause");
	return 0;

}