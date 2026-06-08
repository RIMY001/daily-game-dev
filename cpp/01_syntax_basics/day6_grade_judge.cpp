#include <iostream>
#include <Windows.h>
using namespace std;
int main()
{
	int score;
	cout << "Enter yoour score (0 - 100): ";
	cin >> score;
	if (score < 0 || score > 100)
	{
		cout << "Invalid score! Please enter a number between 0 and 100." << endl;

	}
	else
	{
		if (score >= 90)
		{
			cout << "Grade: A" << endl;

		}
		else if (score >= 80)
		{
			cout << "Grade: B" << endl;

		}
		else if (score >= 70)
		{
			cout << "Grade: C" << endl;

		}
		else if (score >= 60)
		{
			cout << "Grade: D" << endl;


		}
		else
		{
			cout << "Grade: F" << endl;

		}
	}
	system("pause");
	return 0;
}

