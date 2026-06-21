#include <iostream>
#include <string>
using namespace std;

enum Sex
{
	WOMAN,MAN
};
class Student
{
private:
	int id;
	string name;
	Sex sex;
	int score;
public:
	void setAll(int i, string n, int s, Sex se = WOMAN)
	{
		id = i;
		name = n;
		score = s;
		sex = se;
	}
	void show()
	{
		cout << "学号： " << id
			<< " 姓名： " << name
			<< " 性别： ";
		if (sex == MAN)
			cout << "男"  ;
		else cout << "女" ;
		cout << " 分数： " << score << endl;
	}
	void show(string tip)
	{
		cout << tip << endl;
		show();
	}
};

int main()
{
	Student st1, st2;
	st1.setAll(3001, "wang", 90);
	st1.show();
	st2.setAll(3002, "yang", 90, MAN);
	st2.show("===== 学生信息 =====");

	system("pause");
	return 0;
}

//class Student
//{
//private:
//	int id;
//	string name;
//public:
//	//重载：1个参数
//	void setDate(int i)
//	{
//		id = i;
//	}
//	//重载：2个参数
//	void setDate(int i, string s)
//	{
//		id = i;
//		name = s;
//	}
//	void show()
//	{
//		cout << id << " " << name << endl;
//	}
//};
//
//int main()
//{
//	Student s;
//	s.setDate(2001);
//	s.show();
//	s.setDate(2002, "wang");
//	s.show();
//
//	return 0;
//}

////定义学生类
//class Student
//{
////私有成员（属性，数据）
//private:
//	int id;
//	string name;
//	int score;
////公有成员（行为、函数）
//public:
////设置数据
//	void setInfo(int i, string n, int s)
//	{
//		id = i;
//		name = n;
//		score = s;
//	}
////打印信息
//	void showInfo()
//	{
//		cout << "学号： " << id
//			<< " 姓名： " << name
//			<< " 分数： " << score << endl;
//	}
////获取分数
//	int getScore()
//	{
//		return score;
//	}
//};
//
//
//int main()
//{
//	//创建对象：stu1
//	Student stu1;
//	//通过共有函数操作私有数据
//	stu1.setInfo(1001, "zhang", 88);
//	stu1.showInfo();
//	//创建第二个对象，互相独立
//	Student stu2;
//	stu2.setInfo(1002, "li", 95);
//	stu2.showInfo();
//	cout << "stu1分数： " << stu1.getScore() << endl;
//
//	system("pause");
//	return 0;
//}