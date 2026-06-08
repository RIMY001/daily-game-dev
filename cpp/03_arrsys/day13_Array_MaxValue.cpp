#include <iostream>
#include <Windows.h>
using namespace std;
int main()
{
	int numbers[] = { 12,45,7,89,23,56,3 };
	int length = sizeof(numbers) / sizeof(numbers[0]);
	int maxValue = numbers[0];
	for (int i = 1; i < length; i++)
		if (numbers[i] > maxValue)
		{
			maxValue = numbers[i];
		}
	cout << "The maximum value in the array is: " << maxValue << endl;
	system("pause");
	return 0;
}