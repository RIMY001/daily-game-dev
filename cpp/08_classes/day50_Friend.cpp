#include <iostream>
#include <string>
using namespace std;

class Student
{
private:
	int id;
	string name;
	static int count;
public:
	Student(int i, string n) :id(i), name(n) { count++; }
	friend void showInfo(const Student& s);
	friend class Manager;
};
int Student::count = 0;
//全局友元函数
void showInfo(const Student& s)
{
	cout << s.id << " " << s.name << endl;
}
//友元类
class Manager
{
public :
	void getCount()
	{
		cout << "当前学生人数： " << Student::count << endl;
	}
};

int main()
{
	Student s1(1001, "zhang");
	Student s2(1002, "wang");
	showInfo(s1);
	Manager m;
	m.getCount();

	system("pause");
	return 0;
}

//
//class Student
//{
//private:
//	int score;
//public:
//	Student(int sc):score(sc){}
//	friend class Teacher;
//};
//class Teacher
//{
//public:
//	void lookStu(const Student& s)
//	{
//		//可以直接访问Student所有私有成员
//		cout << s.score << endl;
//	}
//};
//
//int main()
//{
//	Student s(99);
//	Teacher t;
//	t.lookStu(s);
//
//	system("pause");
//	return 0;
//}

//class Student;//前置类声明
//class Teacher
//{
//public:
//	void lookStu(const Student& s);
//};
//class Student
//{
//private:
//	int score;
//public:
//	Student(int sc):score(sc){}
//	friend void Teacher::lookStu(const Student& s);
//};
//void Teacher::lookStu(const Student& s)
//{
//	cout << "学生分数： " << s.score << endl;
//}
//
//int main()
//{
//	Student stu(98);
//	Teacher t;
//	t.lookStu(stu);
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
//	Student(int i,string n):id(i),name(n){}
//	//声明全局函数作为友元
//	friend void printInfo(Student s);
//};
////全局友元函数，可以直接访问私有成员
//void printInfo(Student s)
//{
//	cout << "学号：" <<s.id << " " << "姓名：" << s.name << endl;
//}
//
//int main()
//{
//
//	Student s(1001, "张");
//	printInfo(s);
//
//	system("pause");
//	return 0;
//}