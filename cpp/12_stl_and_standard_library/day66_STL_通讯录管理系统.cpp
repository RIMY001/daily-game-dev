#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;

//联系人结构体
struct Person
{
	string name;
	string phone;
	string addr;
};

//全局通讯录容器
vector<Person> addressBook;

//菜单
void showMenu()
{
	cout << "==== 通讯录管理系统 ====" << endl;
	cout << "==== 1. 添加联系人 =====" << endl;
	cout << "==== 2. 显示所有联系人 =" << endl;
	cout << "==== 3. 查找联系人 =====" << endl;
	cout << "==== 4. 删除联系人 =====" << endl;
	cout << "==== 5. 修改联系人 =====" << endl;
	cout << "==== 6. 按姓名排序 =====" << endl;
	cout << "==== 0. 退出系统   =====" << endl;
	cout << "------------------------" << endl;
}

//添加联系人
void addPerson()
{
	Person p;
	cout << "请输入姓名： ";
	cin >> p.name;
	cout << "请输入电话： ";
	cin >> p.phone;
	cout << "请输入地址： ";
	cin >> p.addr;
	addressBook.push_back(p);
	cout << "添加成功！\n\n";
}

//打印单个联系人
void printPerson(const Person& p)
{
	cout << "姓名： " << p.name
		<< " 电话： " << p.phone
		<< " 地址： " << p.addr << endl;
}

//显示全部
void showAll()
{
	if (addressBook.empty())
	{
		cout << "通讯录为空！\n\n";
		return;
	}
	for_each(addressBook.begin(), addressBook.end(), printPerson);
	cout << endl;
}

//查找返回迭代器
vector<Person>::iterator findByName(const string& name)
{
	return find_if(addressBook.begin(), addressBook.end(), [&](const Person& p) {return p.name == name;});
}

//查找联系人
void searchPerson()
{
	string name;
	cout << "输入要查找的姓名: ";
	cin >> name;
	auto it = findByName(name);
	if (it != addressBook.end())
	{
		printPerson(*it);
	}
	else
	{
		cout << "未找到该联系人\n";
	}
	cout << endl;
}

//删除联系人
void deletePerson()
{
	string name;
	cout << "输入要删除的姓名： ";
	cin >> name;
	auto it = findByName(name);
	if (it != addressBook.end())
	{
		addressBook.erase(it);
		cout << "删除成功！\n";
	}
	else
	{
		cout << "联系人不存在\n";
	}
	cout << endl;
}

//修改联系人
void modifyPerson()
{
	string name;
	cout << "请输入要修改的联系人的姓名： ";
	cin >> name;
	auto it = findByName(name);
	if (it != addressBook.end())
	{
		cout << "新电话： ";
		cin >> it->phone;
		cout << "新地址： ";
		cin >> it->addr;
		cout << "修改完成！\n";
	}
	else
	{
		cout << "联系人不存在\n";
	}
	cout << endl;

}

//按姓名升序排序
void sortBook()
{
	sort(addressBook.begin(), addressBook.end(), [](const Person& a, const Person& b) {return a.name < b.name;});
	cout << "排序完成!\n\n";
}

int main()
{
	int select;
	while (true)
	{
		showMenu();
		cin >> select;
		switch (select)
		{
		case 1:
			addPerson();break;
		case 2:
			showAll();break;
		case 3:
			searchPerson();break;
		case 4:
			deletePerson();break;
		case 5:
			modifyPerson();break;
		case 6:
			sortBook();break;
		case 0:
			cout << "欢迎下次使用，程序退出\n";
			return 0;
		default:
			cout << "输入错误，请重新选择!\n\n";
			break;
		}
	}

	system("pause");
	return 0;
}