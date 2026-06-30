#include <iostream>
#include <string>
using namespace std;

class Person
{
protected:
	string name;
	int age;
public:
	Person(string n, int a) :name(n), age(a) {}
	void show()
	{
		cout << "姓名： " << name << " 年龄： " << age << endl;
	}
};

class Student:public Person
{
private:
	int sid;
public:
	Student(string n, int a, int id) :Person(n,a), sid(id) {}
	void print()
	{
		show();
		cout << "学号： " << sid << endl;
	}

};

class Teacher:public Person
{
private:
	string job;
public:
	Teacher(string n, int a, string j) :Person(n, a), job(j) {}
	void print()
	{
		show();
		cout << "授课科目： " << job << endl;
	}
};

int main()
{
	Student s("zhang", 18, 2001);
	Teacher t("liu", 35, "数学");
	s.print();
	t.print();

	system("pause");
	return 0;
}

//class Base
//{
//public:
//	void func()
//	{
//		cout << "父类func" << endl;
//	}
//};
//
//class Son:public Base
//{
//public:
//	void func()
//	{
//		cout << "子类func" << endl;
//	}
//};
//
//int main()
//{
//	Son s;
//	s.func();//调用子类
//	s.Base::func();//强制访问父类同名函数
//
//	system("pause");
//	return 0;
//}

//class Base
//{
//public:
//	Base() { cout << "父类构造" << endl; }
//	~Base() { cout << "父类析构" << endl; }
//};
//class Son :public Base
//{
//public:
//	Son() { cout << "子类构造" << endl; }
//	~Son() { cout << "子类析构" << endl; }
//};
//
//int main()
//{
//	{ Son s; }
//
//	system("pause");
//	return 0;
//}

////父类：人
//class Person
//{
//protected:
//	string name;
//	int age;
//public:
//	Person(string n, int a) : name(n), age(a) {}
//	void showBase()
//	{
//		cout << "姓名： " << name << " 年龄： " << age << endl;
//	}
//
//};
//
////子类：学生
//class Student :public Person
//{
//private:
//	int id;
//public:
//	//子类构造：调用父类构造初始化父类成员
//	Student(string n, int a, int i) :Person(n, a), id(i) {}
//	//子类独有方法
//	void showStu()
//	{
//		showBase();//直接复用父类函数
//		cout << "学号： " << id << endl;
//	}
//};
//
//int main()
//{
//	Student s("zhang", 20, 1001);
//	s.showStu();
//
//	system("pause");
//	return 0;
//}