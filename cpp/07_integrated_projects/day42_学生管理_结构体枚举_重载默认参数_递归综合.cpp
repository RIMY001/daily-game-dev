#include <iostream>
#include <string>
using namespace std;

//1.枚举性别
enum Sex
{
	BOY,
	GIRL,
	UNKNOWN
};

//2.构建学生结构体
struct Student
{
	int id;
	string name;
	Sex sex;
	int score;
};
const int MAX_STU = 100;

//3.函数重载：打印信息
//重载1.打印单个数字
void print(int num)
{
	cout << "数字： " << num << endl;
}
//重载2.打印字符串
void print(string s)
{
	cout << "文本： " << s << endl;
}
//重载3.打印单个学生
void print(Student st, bool showTitle = true)
{
	if (showTitle)
		cout << "===== 学生信息 =====" << endl;
	cout << "学号： " << st.id << "\n"
		<< "姓名： " << st.name << "\n";
	//枚举判断输出文字
	switch (st.sex)
	{
	case BOY:cout << "性别： 男" << endl;
	case GIRL:cout << "性别： 女" << endl;
	case UNKNOWN:cout << "性别： 未知" << endl;
	}
	cout << "分数： " << st.score << endl;
}
//重载4.打印全部学生
void print(Student arr[], int n)
{
	for (int i = 0;i < n;i++)
	{
		print(arr[i], false);//关闭表头，不重复打印
		cout << endl;
	}
}

//4.冒泡排序
void sortByScore(Student* st, int n)
{
	for (int i = 0;i < n - 1;i++)
	{
		for (int j = 0;j < n - 1 - i;j++)
		{
			if ((st + j)->score > (st + j + 1)->score)
			{
				Student temp = *(st + j);//会改变实参
				*(st + j) = *(st + j + 1);
				*(st + j + 1) = temp;
			}
		}
	}
}

//5.递归求总分
int getTotalScore(Student arr[], int n)
{
	//递归出口
	if (n == 0)
		return 0;
	//递推：最后一名分数+前面所有人总分
	return arr[n - 1].score + getTotalScore(arr, n - 1);
}

int main()
{
	//初始化结构体数组
	Student stu[MAX_STU] = {
		{1001,"张",BOY,88},
		{1002,"刘",GIRL,95},
		{1003,"王",GIRL,90},
		{1004,"王",BOY,90}
	};
	int counst = 4;
	//测试函数重载
	print(1);
	print("学生管理系统");
	cout << endl;
	//打印单个学生
	print(stu[0]);
	cout << endl;
	//打印全部学生
	cout << "排序前全部学生： " << endl;
	print(stu, counst);
	cout << endl;
	//分数排序
	sortByScore(stu, counst);
	cout << "排序后全部学生： " << endl;
	print(stu, counst);
	cout << endl;
	//递归计算总分
	int total = getTotalScore(stu, counst);
	cout << "全班总分： " << total << endl;

	system("pause");
	return 0;
}