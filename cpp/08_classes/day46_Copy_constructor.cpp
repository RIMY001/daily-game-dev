#include <iostream>
#include <string>
using namespace std;

enum Sex{MAN,WOMAN};
class Student
{
private:
	int id;
	string name;
	Sex sex;
public:
	//普通构造
	Student(int i, string n, Sex s = MAN):id(i),name(n),sex(s)
	{
		cout << "普通构造" << endl;
	}
	//拷贝构造
	Student(const Student& s)
	{
		id = s.id;
		name = s.name;
		sex = s.sex;
		cout << "拷贝构造" << endl;
	}
	void show()
	{
		cout << id << " " << name << " ";
		if (sex == MAN) cout << "男\n";
		else cout << "女\n";
	}
	~Student()
	{
		cout << "析构函数销毁： " << name << endl;
	}
};

int main()
{
	Student s1(2001, "zhang");
	Student s2(s1);
	s1.show();
	s2.show();

	system("pause");
	return 0;
}


//class Msg
//{
//private:
//	char* text;
//public:
//	//普通构造
//	Msg(const char * str)
//	{
//		int len = strlen(str);
//		text = new char[len + 1];
//		strcpy(text, str);
//		cout << "普通构造函数 开辟堆内存" << endl;
//	}
//	//深拷贝构造
//	Msg(const Msg &m)
//	{
//		int len = strlen(m.text);
//		text = new char[len + 1];
//		strcpy(text, m.text);
//		cout << "深拷贝构造函数 新建独立堆内存" << endl;
//	}
//	void print()
//	{
//		cout << text << endl;
//	}
//	~Msg()
//	{
//		delete[] text;
//		cout << "释放堆内存" << endl;
//	}
//};
//
//int main()
//{
//	Msg m1("DeepCopy Test");
//	Msg m2 = m1;
//	m1.print();
//	m2.print();
//
//	system("pause");
//	return 0;
//}
//class Student
//{
//private:
//	int id;
//	string name;
//public:
//	Student(int i,string s) :id(i), name(s)
//	{
//		cout << "普通构造函数" << endl;
//	}
//	Student(const Student & s)
//	{
//		cout << "拷贝函数执行" << endl;
//		id = s.id;
//		name = s.name;
//	}
//	void test()
//	{
//		cout << id <<  " " << name << endl;
//	}
//};
//
//int main()
//{
//	Student s1(1001, "yang");
//	Student s2 = s1;
//	s1.test();
//	s2.test();
//
//	system("pause");
//	return 0;
//}