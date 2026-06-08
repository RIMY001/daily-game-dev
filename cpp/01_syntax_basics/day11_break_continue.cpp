#include <iostream>
#include <Windows.h>
using namespace std;
int main()
{
	cout << "===== 1.break demo: Exit loop when i equals 5 =====\n";
	for (int i = 1; i <= 10; i++)
	{
		if (i == 5)
		{
			cout << "Reached i=5, executing break to exit the loop!\n";
			break;
		}
		cout << "Current i = " << i << endl;
	}
	cout << "\n===== 2.continue demo: Skip iteration when i equals 5 =====\n";
	for (int i = 1; i <= 10; i++)
	{
		if (i ==5)
		{ 
			cout << "Reached i=5,executing continue to skip this iteration!\n";
			continue;
		}
		cout << "Current i = " << i << endl;
	}
	system("pause");
		return 0;
}