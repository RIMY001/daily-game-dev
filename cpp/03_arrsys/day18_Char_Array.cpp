#include <iostream>
#include <Windows.h>
using namespace std;
int main()
{
	char text[50] = " I study C++ every day.";
	cout << "Whole string: " << text << endl;
	cout << "Single characters: ";
	for ( int i = 0 ; text[i] != '\0' ; i++ )

	{
		cout  << text[i] << " ";

	}
	cout << endl;
	system("pause");
	return 0;
}