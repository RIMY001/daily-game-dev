#include <iostream>
#include <Windows.h>
using namespace std;
int main()
{
	int choice;
	cout << "===== Menu =====" << endl;
	cout << "1.Study" << endl;
	cout << "2.Play" << endl;
	cout << "3.Sleep" << endl;
	cout << "Ender your choice: " << endl;
	cin >> choice;

	switch (choice)
	{
	case 1:
		cout << "You choose to study." << endl;
		break;
	case 2:
		cout << "You choose to play." << endl;
		break;
	case 3:
		cout << "You choose to sleep." << endl;
		break;
	default:
		cout << "Invalid choice." << endl;
		break;

	}
	system("pause");
	return 0;

	
}