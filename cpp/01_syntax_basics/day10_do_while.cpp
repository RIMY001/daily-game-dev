#include <iostream>
#include <Windows.h>
using namespace std;
int main()
{
	int i = 1;
	do
	{
		cout << "Nember: " << i++ << endl;
	
	} while (i <= 10);
	system("pause");
	return 0;
}