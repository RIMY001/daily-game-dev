#include <iostream>
#include <memory>
using namespace std;

void demo_unique()
{
	auto p1 = make_unique<int>(99);
	cout << *p1 << endl;
	//unique_ptr<int> p2 = p1;	编译报错，删除了拷贝构造
	auto p2 = move(p1);	//转移所有权，p1置空
	cout << *p2 << endl;
	if (!p1)
	{
		cout << "p1已经为空" << endl;
	}
	//出作用域自动释放堆内存，不用手动delete
	cout << endl;
}

void demo_shared()
{
	auto s1 = make_shared<int>(666);
	cout << s1.use_count() << endl;
	auto s2 = s1;
	cout << s1.use_count() << endl;
	s1.reset();	//s1放弃管理
	cout << s2.use_count() << endl;
	cout << endl;
}

struct Node
{
	int val;
	weak_ptr<Node> next;	//改为weak_ptr打破循环
	Node(int v): val(v){}
};

void fix_circle()
{
	auto n1 = make_shared<Node>(1);
	auto n2 = make_shared<Node>(2);
	n1->next = n2;
	n2->next = n1;
}

int main()
{
	demo_unique();
	demo_shared();
	fix_circle();

	system("pause");
	return 0;
}