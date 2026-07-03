#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <algorithm>
using namespace std;

//创建选手类
class Person
{
public:
	string m_Name;
	int m_Score;

	Person(string n, int s) :m_Name(n), m_Score(s) {};
};

//创建具体选手
void createPerson(vector<Person>& v)
{
	string nameSeed = "ABCDE";
	for (int i = 0;i < 5;i++)
	{
		string name = "选手";
		name += nameSeed[i];

		int score = 0;
		Person p(name, score);
		//将创建的Person对象放入容器中
		v.push_back(p);
	}
}

//打分然后取平均值
void setScore(vector<Person> &v)
{
	for (vector<Person>::iterator it = v.begin();it != v.end();it++)
	{
		deque<int>d;//v里每有一个对象，就创一个装打分的容器
		for (int i = 0;i < 10;i++)
		{
			int score = rand() % 40 + 61;//用随机数模拟打分
			d.push_back(score);//将打分放到容器中
		}

		//排序
		sort(d.begin(), d.end());
		//去除最高分和最低分
		d.pop_back();
		d.pop_front();
		//取平均分
		int sum = 0;
		for (deque<int>::iterator it = d.begin();it != d.end();it++)
		{
			sum += *it;
		}
		int avg = sum / d.size();
		//将平均分赋给对象的成员属性m_Score上
		it->m_Score = avg;
	}
}

//显示分数
void showScore(vector<Person> &v)
{
	for (vector<Person>::iterator it = v.begin();it != v.end();it++)
	{
		cout << "姓名：" << it->m_Name << " "
			<< "分数：" << it->m_Score << endl;
	}
	cout << endl;
}

int main()
{
	//随机数种子
	srand((unsigned)time(NULL));
	//创建5名选手
	vector<Person>v;//存放选手容器
	createPerson(v);

	//打分
	setScore(v);

	//显示分数
	showScore(v);

	system("pause");
	return 0;
}