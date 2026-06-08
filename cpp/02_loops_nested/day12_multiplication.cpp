#include <iostream>
#include <Windows.h>
using namespace std;
int main()
{
	for (int row = 1; row <= 9; row++)
	{
		for (int col = 1; col <= row; col++)
		{
			cout << col << " * " << row << " = " << col * row << "\t";
		}
		cout << endl;
	}
	system("pause");
	return 0;
}