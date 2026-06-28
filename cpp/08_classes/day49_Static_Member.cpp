#include <iostream>
#include <string>
using namespace std;

class Student
{
private:
	string name;
	static int stuCount;
public:
	Student(string n) :name(n)
	{
		stuCount++;
	}
	static void showCount()
	{
		cout << "一共创建了： " << stuCount << "名学生" << endl;
	}
};
int Student::stuCount = 0;

int main()
{
	Student s1("zhang");
	Student s2("yang");
	Student s3("li");
	Student::showCount();

	system("pause");
	return 0;
}

//class Person
//{
//private:
//	static int total;
//public:
//	static void add()
//	{
//		total++;
//	}
//	static void print()
//	{
//		cout << total << endl;
//	}
//};
//
//int Person::total = 0;
//
//int main()
//{
//	Person p1;
//	Person p2;
//	//p1.total;
//	//Person::total;
//	Person::add();
//	Person::add();
//	Person::print();
//
//	system("pause");
//	return 0;
//}

//class Student
//{
//public:
//	int id;
//	static int num;
//	Student()
//	{
//		cout << ++num << endl;
//	}
//	//静态成员函数
//	static void showNum()
//	{
//		cout << "当前对象总数： " << num << endl;
//		//cout << id << endl;静态成员函数无法访问非静态成员,没有this指针
//	}
//};
//int Student::num = 0;
//
//int main()
//{
//	Student s1;
//	Student s2;
//	//优先使用类名调用
//	Student::showNum();
//
//
//	system("pause");
//	return 0;
//}