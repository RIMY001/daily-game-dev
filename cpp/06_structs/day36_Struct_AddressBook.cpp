#include <iostream>
#include <string>
using namespace std;

//1.定义结构体
struct Person {
	string name;
	string phone;
};
const int MAX = 4;

//2.显示所有联系人
void showAll(Person p[],int n)
{
	cout << "\n===== 通讯录列表 =====" << endl;
	for (int i = 0; i < n; i++)
	{
		cout << i + 1
			<< "姓名： " << p[i].name << "\t"
			<< "电话： " << p[i].phone << endl;
	}
}

//3.按姓名冒泡排序
void sortByName(Person p[], int n)
{
	for (int i = 0; i < n - 1; i++)
	{
		for (int j = 0; j < n - i - 1; j++)
		{
			if (p[j].name > p[j + 1].name)
			{
				Person temp = p[j];
				p[j] = p[j + 1];
				p[j + 1] = temp;
			}

		}
	}
}

//4.按姓名查找联系人
void findPerson(Person p[], int n, string key)
{
	bool isFind = false;
	for (int i = 0; i < n; i++)
	{
		if (key == p[i].name) 
		{
			cout << "姓名： " << p[i].name << "\n"
				<< "电话： " << p[i].phone << endl;
			isFind = true;
			break;
		}

	}
	if (!isFind)
		cout << "查无此人" << endl;
}

int main()
{
	//定义结构体数组：存放多个联系人
	Person people[MAX] = {
		{"Zhang","111"},
		{"Li","222"},
		{"Wang","333"},
		{"Liu","444"}
	};
	int count = MAX;
	showAll(people, count);
	//排序
	cout << "\n----- 按姓名排序后 ----- " << endl;
	sortByName(people, count);
	showAll(people, count);
	//查找
	string searchName;
	cout << "\n请输入要查找的联系人： " << endl;
	cin >> searchName;
	findPerson(people, count, searchName);

	system("pause");
	return 0;
}