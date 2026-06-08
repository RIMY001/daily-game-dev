#include <iostream>
using namespace std;
void printNum(int x)
{
	cout << "The number you pass is: " << x << endl;
}
void getSum(int x, int y)
{
	int res = x + y;
	cout << "Sum result: " << res << endl;
}
int main()
{
	int x = 10;
	int y = 25;
	printNum(x);
	getSum(x, y);
	getSum(60, 30);
	system("pause");
	return 0;

}