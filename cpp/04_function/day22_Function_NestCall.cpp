#include <iostream>
using namespace std;
int add(int a, int b)
{
	return a + b;
}
int mul(int a, int b)
{
	return a * b;

}
int calculation(int x, int y, int z)
{
	int sum = add(x, y);
	int result = mul(sum, z);
	return result;
}
int main()
{
	int num1 = 2;
	int num2 = 3;
	int num3 = 4;
	int finalAns = calculation(num1, num2, num3);
	cout << "Final result: " << finalAns << endl;
	system("pause");
	return 0;
}
