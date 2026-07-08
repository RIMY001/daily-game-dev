#include <iostream>
#include <deque>
#include <string>
using namespace std;

//常用构造方式
void test01()
{
	//1.空容器
	deque<int> d1;
	//2.n个相同元素
	deque<int> d2(10, 2);
	//3.拷贝构造
	deque<int> d3(d2);
	//4.初始化列表
	deque<int> d4{ 1,2,3,4 };
}

//首尾增删（deque核心优势接口）
void test02()
{
	deque<int> d;
	//尾部操作 (同vector)
	d.push_back(1);
	d.push_back(2);
	d.push_back(3);
	d.pop_back();
	//头部操作（vector没有高效实现）
	d.push_front(4);
	d.push_front(5);
	d.pop_front();
}

//元素访问
void test03()
{
	deque<int> d{ 2,3,4,5,6 };
	cout << d[0] << endl;
	cout << d.at(1) << endl;
	cout << d.front() << endl;
	cout << d.back() << endl;
}

//插入、删除、清空
void test04()
{
	deque<int> d{ 3,4,5,6,7 };
	//指定迭代器位置插入
	d.insert(d.begin(), 5);
	d.insert(d.begin(), 5);
	void test05(deque<int> &d);
	test05(d);
	//删除单个元素
	d.erase(d.begin());
	//删除区间
	d.erase(++d.begin(), --d.end());
	test05(d);
	//清空
	d.clear();
}

//遍历方式
void test05(deque<int> &d)
{
	for (auto it = d.begin();it != d.end();it++)
	{
		cout << *it << " ";
	}
	cout << endl;
}

//容量相关接口
void test06()
{
	deque<int> d{ 4,5,6,7 };
	cout << d.size() << endl;
	if (d.empty()) { cout << "双端队列为空" << endl; }
	else cout << "双端队列不为空" << endl;
	//deque没有capacity和reserve,底层分段内存，无需预分配整块空间
}

//模拟排队系统
template <typename Container>
void test08(const Container& c);
void test07()
{
	deque<string> d;
	d.push_back("顾客A");
	d.push_back("顾客B");
	d.push_back("顾客C");
	cout << "当前队列： " << endl;
	test08(d);
	//插队（头部加入）
	d.push_front("VIP顾客D");
	cout << "VIP插队后：" << endl;
	test08(d);
	//队首办理完成后离开
	d.pop_front();
	cout << "VIP办理完毕，剩余队列:" << endl;
	test08(d);
}

//定义一个通用的打印模板
template <typename Container>
void test08(const Container& c)
{
	for (const auto& elem : c) { cout << elem << " "; }
	cout << endl;
}

int main()
{
	test03();
	test04();
	test06();
	test07();

	system("pause");
	return 0;
}