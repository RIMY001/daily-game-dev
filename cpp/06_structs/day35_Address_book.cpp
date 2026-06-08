#include <iostream>
#include <string>
using namespace std;

//全局变量
const int MAX = 5;

//打印全部联系人
void showAll(string name[],string phone[],int n)
{
	cout << "=====通讯录=====" << endl;
	for (int i = 0;i < n;i++)
	{
		cout << i + 1 << ".姓名： " << name[i]
			<< "   电话： " << phone[i] << endl;
	}
}

//按姓名升序排列
void sortByName(string name[],string phone[], int n)
{
	for (int i = 0;i < n - 1;i++)
	{
		for (int j = 0;j < n - 1 - i;j++)
		{
			if (name[j] > name[j + 1])
			{
				string n = name[j];
				string p = phone[j];
				name[j] = name[j + 1];
				phone[j] = phone[j + 1];
				name[j + 1] = n;
				phone[j + 1] = p;
			}
		}
	}
}

//按姓名查找
void findByName(string name[],string phone[],int n,string key)
{
	int flog = 0;
	for (int i = 0;i < n;i++)
	{
		if (name[i] == key)
		{
			cout << "找到： " << name[i] << " " << phone[i] << endl;
			flog = 1;
			break;
		}
	}
	if (flog == 0)
		cout << "查无此人" << endl;
}

int main()
{
	string name[MAX] = {
		"章 衡","刘","杨","王","吴"
	};
	string phone[MAX] = {
		"1111","2222","3333","4444","5555"
	};
	showAll(name,phone,MAX);
	//排序
	sortByName(name,phone,MAX);
	cout << "排序后： " << endl;
	showAll(name,phone,MAX);
	//查找
	string search;
	cout << "请输入要查找的联系人： " << endl;
	getline(cin, search);
	findByName(name,phone,MAX,search);

	system("pause");
	return 0;
}