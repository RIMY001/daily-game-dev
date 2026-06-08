#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <cstring>
using namespace std;
int main()
{
	char str1[50] = "Hello";
	char str2[50] = "World";
	char str3[50];
	//1.strlen：求有效字符串长度，不含'\0'
	cout << "str1长度: " << strlen(str1) << endl;
	//2.strcpy：字符串拷贝 str2 -> str3
	strcpy(str3, str2);
	cout << "拷贝后str3: " << str3 << endl;
	//3.strcat:字符串拼接 str2接到str1末尾
	strcat(str1, str2);
	cout << "拼接后str1: " << str1 << endl;
	//4。strcmp：字符串比较
	char a[] = "abc";
	char b[] = "abd";
	int ret = strcmp(a, b);
	if (ret == 0)
	{
		cout << "两个字符串相等" << endl;
	}
	else if (ret > 0)
	{
		cout << "a > b" << endl;
	}
	else
	{
		cout << "a < b" << endl;
	}
	system("pause");
	return 0;
}