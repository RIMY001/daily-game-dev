#define _CRT_SECURE_NO_WARNINGS//这是一个预处理器宏定义，用于禁用 Visual Studio 编译器的安全警告。
#include <iostream>
#include <cstring>
using namespace std;

class Message
{
private:
	char* msg;//堆字符指针
public:
	//构造：堆空间开辟
	Message(const char* str) //const char*类型的值不能用于初始化char*类型的实体
	{
		int len = strlen(str);
		msg = new char[len + 1];
		strcpy(msg, str);
		cout << "堆内存分配完成" << endl;
	}
	//析构：释放堆内存，防止内存泄露
	~Message()
	{
		delete[] msg;
		cout << "堆内存已释放" << endl;
	}
	void show()
	{
		cout << msg << endl;
	}
};

int main()
{
	{
		Message m("Hello OOP");
		m.show();
	}

	system("pause");
	return 0;
}

//class Person
//{
//private:
//	string name;
//public:
//	//有参构造，初始化列表
//	Person(string s) :name(s)
//	{
//		cout << "[构造]" << name << "诞生" << endl;
//	}
//	//唯一析构函数
//	~Person()
//	{
//		cout << "[析构]" << name << "销毁" << endl;
//	}
//	void printName()
//	{
//		cout << "姓名： " << name << endl;
//	}
//};
//
//int main()
//{
//	{
//		Person p("张三");
//		p.printName();
//	}
//
//	system("pause");
//	return 0;
//}

//class Student
//{
//private:
//	int id;
//public:
//	//构造函数
//	Student(int i = 0) :id(i)
//	{
//		cout << "构造函数执行，创建对象" << endl;
//	}
//	//析构函数
//	~Student()
//	{
//		cout << "析构函数执行，销毁对象，id = " << id << endl;
//	}
//	void show()
//	{
//		cout << "学号： " << id << endl;
//	}
//
//};
//
//int main()
//{
//	{//花括号在这里创建了一个局部作用域
//		Student s1(1001);
//		s1.show();
//		cout << "主函数即将结束\n";
//	}//s1在这里超出作用域，析构函数立即调用。否则在return 0之后，main函数真正结束时才被调用，此时控制台已经停止了.
//	//局部变量的生命周期诞生：在执行到声明语句时（如 Student s1(1001);）死亡：在离开它所在的作用域时（遇到匹配的"}"时)
//
//
//	system("pause");
//	return 0;
//}