#include <iostream>
#include <string>
using namespace std;

enum Sex
{
	MAN,
	WOMAN
};
class Student
{
private:
	int id;
	string name;
	Sex sex;
	mutable int printTime;
public:
	Student(int i, string n, Sex s) :id(i), name(n), sex(s),printTime(0) {}
	//普通成员函数
	void setName(string n)
	{
		name = n;
	}
	//常成员函数
	void info() const
	{
		printTime++;
		cout << id << " " << name << " ";
		if (sex == MAN) cout << "男";
		else cout << "女";
		cout << " " << printTime << endl;;
	}
};

int main()
{
	Student s1(4001, "王阳", WOMAN);
	s1.setName("王洋");
	s1.info();
	s1.info();
	Student s2(4002, "张展", MAN);
	s2.info();
	s2.info();
	s2.info();

	system("pause");
	return 0;
}

//class Student
//{
//private:
//	int id;
//	string name;
//	mutable int visitCot;//mutable可变变量
//public:
//	Student(int i, string n) :id(i), name(n) ,visitCot(0){}
//	//常成员函数
//	void show() const
//	{
//		visitCot++;
//		cout << id << " " << name << " " << visitCot << endl;
//	}
//};
//
//int main()
//{
//	const Student s1(3001, "杨");
//	s1.show();
//
//	system("pause");
//	return 0;
//}

//class Test
//{
//public:
//	void func()
//	{
//		cout << "普通成员函数" << endl;
//	}
//	void func() const
//	{
//		cout << "常成员函数" << endl;
//	}
//
//};
//
//int main()
//{
//	Test r1;
//	r1.func();
//	Test r2;
//	r2.func();
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
//	Student(int i, string n) :id(i), name(n) {}
//	//普通成员函数：可修改成员
//	void setName(string n)
//	{
//		name = n;
//	}
//	//常成员函数：函数末尾加const，只读，不可改成员
//	void show() const 
//	{
//		cout << id << " " << name << endl;
//	}
//};
//
//int main() 
//{
//	//普通对象
//	Student s1(1001, "张");
//	s1.setName("李");
//	s1.show();
//	//常对象
//	const Student s2(1002, "刘");
//	//s2.setName("刘扬");//报错:常对象不能调用普通成员函数
//	s2.show();
//
//	system("pause");
//	return 0;
//}