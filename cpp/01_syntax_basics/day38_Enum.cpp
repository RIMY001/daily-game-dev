//枚举,罗列有限取值来限制变量

#include <iostream>
using namespace std;
//枚举+结构体的通讯录案例

//定义枚举：性别
enum Gender{OTHER,MAN,WOMAN};

//定义结构体：地址
struct Address
{
	string province;
	string city;
};

//人员结构体（嵌套枚举+结构体）
struct Person
{
	//姓名
	string name;
	//电话
	string phone;
	//性别
	Gender sex;
	//地址
	Address arr;
};

//显示所有信息
void showPerson(Person *p,int n)
{
	cout << "\n=====升级版通讯录=====" << endl;
	for (int i = 0;i < n;i++)
	{
		cout << i + 1 << ": " << endl;
		cout << "姓名： " << (p + i)->name << endl;
		cout << "电话： " << (p + i)->phone << endl;
		cout << "性别： ";
		switch ((p + i)->sex)
		{
		case OTHER: cout << "其他" << endl;break;
		case MAN: cout << "男" << endl;break;
		case WOMAN: cout << "女" << endl;break;
		}
		cout << "地址： " << (p + i)->arr.province << (p + i)->arr.city << "\n" << endl;
	}
}

const int MAX = 5;

int main()
{
	//结构体数组初始化
	Person book[MAX] = {
		{"yuang","157",MAN,{"上海","徐汇"}},
		{"liu","021",MAN,{"上海","徐汇"}},
		{"chen","151",WOMAN,{"湖北","武汉"}}

	};
	int count = 3;
	showPerson(book, 3);
	system("pause");
	return 0;
}

//////1.定义枚举类型：星期
////enum Week
////{
////	//枚举常量默认从0开始，自增1
////	Mon,
////	Tue,
////	Wed,
////	Thu,
////	Fri,
////	Sat,
////	Sun
////};
//
//////2.手动指定枚举数值
//////可以自定义起始值、任意常量值：
////enum Gender
////{
////	Man = 1,
////	Woman = 2,
////	Secret = 0
////};
//
////3.枚举判断+分支语句
//enum Week { Mon, Tue, Wed, Thu, Fri, Sat, Sun };
//
//int main()
//{
//	Week d = Fri;
//	switch (d)
//	{
//	case Mon:cout << "星期一" << endl;break;
//	case Tue:cout << "星期二" << endl;break;
//	case Wed:cout << "星期三" << endl;break;
//	case Thu:cout << "星期四" << endl;break;
//	case Fri:cout << "星期五" << endl;break;
//	case Sat:cout << "星期六" << endl;break;
//	case Sun:cout << "星期日" << endl;break;
//	}
//
//	//2.Gender g = Woman;
//	//cout << g << endl;
//
//	////1.定义枚举变量，只能复制列表内常量
//	//Week day = Mon;
//	////枚举本质是整数，可以输出对应数值
//	//cout << "Mon 对应的数值： " << day << endl;
//	//day = Sat;
//	//cout << "Sat 对应的数值： " << day << endl;
//
//	system("pause");
//	return 0;
//}