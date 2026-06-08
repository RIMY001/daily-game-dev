#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <cstring>
using namespace std;
//主要任务：学会用<cstring>库里的函数给字典排序
int main()
{
	char name[5][20] = {
		"zhang",
		"li",
		"wang",
		"yang",
		"liu"
	};
	int len = 5;
	char temp[20];//交换字符串临时容器
	cout << "排序前名单： " << endl;
	for (int i = 0;i < len;i++)
	{
		cout << name[i] << " ";//cout对char*特殊重载：遇到char*不去打印地址，顺着地址挨个输出字符，直到\0
	}
	cout << endl;
	//冒泡排序：strcmp实现字典排序
	for (int i = 0;i < len - 1;i++)
	{
		for (int j = 0;j < len - 1 - i;j++)
		{
			//name[j] > name[j+1]就交换
			if (strcmp(name[j] , name[j + 1]) > 0)
			{
				strcpy(temp, name[j+1]);
				strcpy(name[j + 1], name[j]);
				strcpy(name[j], temp);
			}
		}
	}
	cout << "字典升序后: " << endl;
	for (int i = 0;i < len;i++)
	{
		cout << name[i] << " ";
	}
	cout << endl;
	system("pause");
	return 0;
}