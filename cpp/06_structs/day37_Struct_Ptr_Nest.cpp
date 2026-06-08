#include <iostream>
#include <string>
using namespace std;

////定义结构体
//struct Person {
//	string name;
//	int age;
//};
//
//const int MAX = 5;
//
////形参结构体指针（数组退化而来）
//void showPerson(Person* p, int n)
//{
//	for (int i = 0; i < n; i++)
//	{
//		cout << i + 1 << "." << "姓名： " << (p+i)->name << " "
//			<< "年龄： " << (p+i)->age << endl;
//	}
//}
//
//int main()
//{
//	////定义结构体变量
//	//Person p1;
//	//p1.name = "Jim";
//	//p1.age = 18;
//	////定义结构体指针，指向p
//	//Person* p = &p1;
//	////两种访问成员方法
//	//cout << "【变量.成员】" << endl;
//	//cout << p1.name << " " << p1.age << endl;
//	//cout << "【指针->成员】" << endl;
//	//cout << p->name << " " << p->age << endl;
//	////通过指针修改成员
//	//p->name = "Andy";
//	//p->age = 21;
//	//cout << "\n指针修改后: " << p1.name << " " << p1.age << endl;
//
//	//结构体数组
//	Person arr[MAX] = {
//		{"张三",18},
//		{"李四",19},
//		{"王二",18},
//		{"刘大",17},
//		{"杨五",18}
//	};
//	showPerson(arr, 5);
//
//	system("pause");
//	return 0;
//}

//子结构体：地址信息
struct Address {
	string province;
	string city;
};

//主结构体：嵌套 Adddress
struct Person {
	string name;
	string phone;
	struct Address addr;//嵌套结构体成员
};

int main()
{
	Person p;
	//逐层访问嵌套成员:变量.子结构体.成员
	p.name = "安迪";
	p.phone = "123";
	p.addr.province = "山东";
	p.addr.city = "济南";
	cout << "姓名： " << p.name << endl;
	cout << "电话： " << p.phone << endl;
	cout << "地址： " << p.addr.province << " " << p.addr.city << endl;
	//结合结构体指针访问嵌套成员
	Person* p2 = &p;
	cout << "\n指针访问" << endl;
	cout << p2->addr.province << " " << p2->addr.city << endl;
	
	system("pause");
	return 0;
}