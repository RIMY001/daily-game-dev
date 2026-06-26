#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <cstring>
using namespace std;

//class Msg
//{
//private:
//	char* text;
//public:
//	//普通构造
//	Msg(const char* str)
//	{
//		int len = strlen(str);
//		text = new char[len + 1];
//		strcpy(text, str);
//	}
//	//拷贝构造（深拷贝）
//	Msg(const Msg& m)
//	{
//		int len = strlen(m.text);
//		text = new char[len + 1];
//		strcpy(text, m.text);
//	}
//	//赋值运算符重载 深拷贝
//	Msg& operator=(const Msg& m)
//	{
//		//1.自赋值判断
//		if (this == &m)
//		{
//			return *this;
//		}
//		//2.释放自身旧内存
//		delete[] text;
//		//3.开辟新内存
//		int len = strlen(m.text);
//		text = new char[len + 1];
//		strcpy(text, m.text);
//		//4.返回自身
//		return *this;
//	}
//	void show()
//	{
//		cout << text << endl;
//	}
//	~Msg()
//	{
//		delete[] text;
//		cout << "内存释放" << endl;
//	}
//};
//
//int main()
//{
//	Msg m1("C++赋值重载");
//	Msg m2("测试文本");
//	cout << "赋值前m2： ";
//	m2.show();
//	m2 = m1;//触发重载operator=
//	cout << "赋值后m2： ";
//	m2.show();
//	//自赋值测试不会崩溃
//	m2 = m2;
//
//	system("pause");
//	return 0;
//}

class Student
{
private:
	int id;
	string name;
public:
	Student(int i = 0,string n = "张"):id(i),name(n){}
	//赋值运算符重载
	Student& operator = (const Student &s)
	{
		cout << "赋值运算符重载执行" << endl;
		//拷贝成员
		this->id = s.id;
		this->name = s.name;
		//返回自身，支持连续赋值
		return *this;
	}
	void show()
	{
		cout << id << " " << name << endl;
	}
};

void test01()
{
	int id1 = 1002;
	int id2;
	id2 = id1;
	cout << id2 << endl;
}

int main()
{
	Student s1(1001, "李");
	Student s2;
	s2.show();
	//两个已存在对象赋值，触发operator=
	s2 = s1;
	s2.show();
	//普通赋值能否运作
	test01();
	//连续赋值测试
	Student s3;
	s3 = s2 = s1;
	s3.show();

	system("pause");
	return 0;
}