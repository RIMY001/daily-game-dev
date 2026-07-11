#include <iostream>
#include <queue>
#include <string>
using namespace std;

void test01()
{
	queue<int> q;
	//入队
	q.push(1);
	q.push(2);
	q.push(3);
	cout << "队首：" << q.front() << endl;
	cout << "队尾：" << q.back() << endl;
	cout << "总数量：" << q.size() << endl;
	//全部出队
	while (!q.empty())
	{
		cout << q.front() << " ";
		q.pop();
	}
	cout << endl;
}

//模拟银行排队
void test02()
{
	queue<string> bankQueue;
	//顾客排队
	bankQueue.push("顾客A");
	bankQueue.push("顾客B");
	bankQueue.push("顾客C");
	cout << "=======依次办理业务========" << endl;
	while (!bankQueue.empty())
	{
		cout << "正在办理：" << bankQueue.front() << endl;
		bankQueue.pop();
	}
	cout << "全部顾客办理完成" << endl;
}

int main()
{
	test01();
	cout << "==================" << endl;
	test02();

	system("pause");
	return 0;
}