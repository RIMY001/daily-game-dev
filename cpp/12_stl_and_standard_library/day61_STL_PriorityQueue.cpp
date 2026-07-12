#include <iostream>
#include <queue>
#include <vector>
#include <string>
using namespace std;

//默认大根堆示例
void test01()
{
	priority_queue<int> maxHeap;
	maxHeap.push(22);
	maxHeap.push(11);
	maxHeap.push(33);
	maxHeap.push(5);
	//依次弹出最大值
	while (!maxHeap.empty())
	{
		cout << maxHeap.top() << " ";
		maxHeap.pop();
	}
	cout << endl;
}

//构建小根堆
void test02()
{
	priority_queue<int, vector<int>, greater<int>> minHeap;
	minHeap.push(22);
	minHeap.push(11);
	minHeap.push(33);
	minHeap.push(5);
	while (!minHeap.empty())
	{
		cout << minHeap.top() << " ";
		minHeap.pop();
	}
	cout << endl;
}

//自定义结构体优先级
struct Student
{
	int id;
	int score;
};
//自定义比较仿函数，大根堆逻辑；返回true则交换
struct Cmp
{
	bool operator()(const Student& a, const Student& b)
	{
		//分数降序，分数相同学号升序
		if (a.score != b.score)
			return a.score < b.score;
		return a.id > b.id;
	}
};
void test03()
{
	priority_queue<Student, vector<Student>, Cmp> stuHeap;
	stuHeap.push({ 1001,88 });
	stuHeap.push({ 1002,98 });
	stuHeap.push({ 1003,98 });
	while (!stuHeap.empty())
	{
		auto s = stuHeap.top();
		cout << "学号： " << s.id << " 分数： " << s.score << endl;
		stuHeap.pop();
	}
	cout << endl;
}

//实战算法：Top3最大数
void test04()
{
	vector<int> data = { 12,45,7,99,23,66,31 };
	priority_queue<int> maxHeap;
	//全部入堆
	for (int num : data)
		maxHeap.push(num);
	cout << "前三最大值： " << endl;
	for (int i = 0; i < 3; i++)
	{
		cout << maxHeap.top() << " ";
		maxHeap.pop();
	}
	cout << endl;
}

int main()
{
	test01();
	test02();
	test03();
	test04();

	system("pause");
	return 0; 
}