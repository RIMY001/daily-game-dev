#include <iostream>
#include <string>
using namespace std;

class Student
{
private:
	int id;
	string name;
public:
	void show()
	{
		cout << "学号： " << id
			<< " 姓名： " << name << endl;
		
	}
};

int main()
{
	Student s;
	s.show();//整型的是脏数据，字符串的是空

	system("pause");
	return 0;
}


//enum Sex{MAN,WOMAN};
//class Student
//{
//private:
//	int id;
//	string name;
//	Sex sex;
//public:
//	//无参构造
//	Student() :id(0), name("未知"), sex(MAN) {};
//	//三参构造
//	Student(int i, string n, Sex s) :id(i), name(n), sex(s) {};
//	void show()
//	{
//		cout << "学号： " << id
//			<< " 姓名： " << name
//			<< " 性别： ";
//		if (sex == MAN)
//		{
//			cout << "男" << endl;
//		}
//		else cout << "女" << endl;
//	}
//};
//
//int main()
//{
//	Student s1;
//	Student s2(1001, "liu", MAN);
//	Student s3(1002, "wang", WOMAN);
//	s1.show();
//	s2.show();
//	s3.show();
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
//	//初始化列表
//	Student(int i, string n) :id(i), name(n) {};
//	void show()
//	{
//		cout << id << " " << name << endl;
//	}
//};
//
//int main()
//{
//	Student s(3001, "刘");
//	s.show();
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
//	////无参构造
//	//Student()
//	//{
//	//	cout << "无参构造调用" << endl;
//	//}
//
//	//构造参数带默认值
//	Student(int i = 0, string n = "未知")
//	{
//		id = i;
//		name = n;
//	}
//	void show()
//	{
//		cout << id << " " << name << endl;
//	}
//};
//
//int main()
//{
//	Student s1;//当函数重载与默认参数同时发生，可能会出现编译器分不清对象用哪一个构造函数初始化
//	Student s2(2001);
//	Student s3(2002, "王");
//	s1.show();
//	s2.show();
//	s3.show();
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
//	//无参构造
//	Student()
//	{
//		id = 0;
//		name = "无名";
//	}
//	//有参构造1.只传学号
//	Student(int i)
//	{
//		id = i;
//		name = "无名";
//	}
//	//有参构造2.学号+姓名
//	Student(int i, string s)
//	{
//		id = i;
//		name = s;
//	}
//	void show()
//	{
//		cout << "学号： " << id << " 姓名： " << name << endl;
//	}
//};
//
//int main()
//{
//	Student s1;//无参构造
//	Student s2(1001);//单参数构造
//	Student s3(1002, "zhang");//双参数构造
//	s1.show();
//	s2.show();
//	s3.show();
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
//	//无参构造(自己不写，编译器提供默认无参构造）
//	Student()
//	{
//		cout << "无参构造函数自动调用" << endl;
//		id = 0;
//		name = "未知";
//	}
//	void show()
//	{
//		cout << id << " " << name << endl;
//	}
//};
//
//int main()
//{
//	Student s;//创建对象时，自动调用无参构造
//	s.show();
//	system("pause");
//	return 0;
//}