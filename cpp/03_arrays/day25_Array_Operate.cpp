#include <iostream>
using namespace std;
int main()
{
	int score[6];
	int maxNum;
	int minNum;
	int sum = 0;
	cout << "Enter six numbers: " << endl;
	for (int i = 0; i < 6; i++)
	{
		cin >> score[i];
	}
	cout << "All elements: " << endl;
	for (int i = 0; i < 6; i++)
	{
		cout << score[i] << "\t";
	}
	cout << endl;
    maxNum = score[0];
	minNum = score[0];
	for (int i = 0; i < 6; i++)
	{
		if (maxNum < score[i])
		{
			maxNum = score[i];
		}
		if (minNum > score[i])
		{
			minNum = score[i];
		}
	}
	cout << "The max number is : " << maxNum << endl;
	cout << "The min number is : " << minNum << endl;
	for (int i = 0; i < 6; i++)
	{
		sum += score[i];
	}
	cout << " The sum is : " << sum << endl;
	system("pause");
	return 0;
}