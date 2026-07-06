#include <iostream>
#include <vector>
using namespace std;

//四种常用构造：
void test01()
{
	//1.空容器
	vector<int> v1;
	//2.n个初始值为val的元素
	vector<int> v2(5, 10);
	//3.拷贝构造
	vector<int> v3(v2);
	//4.初始化列表
	vector<int> v4{ 1,2,3,4,5 };
}

//赋值操作
void test02()
{
	//赋值操作
	test01();
	vector<int> v1 = { 1,2,3 };
	vector<int> v2;
	v2 = v1;
	//assign批量赋值
	v2.assign(4, 66);//4个66

}

//尾部增删
void test03()
{
	vector<int> v;
	//尾部添加元素
	v.push_back(10);
	v.push_back(20);

	//删除最后一个元素
	v.pop_back();
}

//容量和大小
void test04()
{
	vector<int> v;
	v.reserve(100);//提前开100个空间，避免频繁扩容
	cout << v.size() << endl;
	cout << v.capacity() << endl;
	cout << "=====================================================" << endl;
}

//元素访问
void test05()
{
	vector<int> v{ 1,2,3,4 };
	cout << v[1] << endl;//越界不报错，不安全
	cout << v.at(1) << endl;//越界抛出异常，安全
	cout << v.front() << endl;
	cout << v.back() << endl;
	cout << "=====================================================" << endl;
}

//迭代器遍历
void test06()
{
	vector<int> v{ 10,20,30 };
	for (vector<int>::iterator it = v.begin();it != v.end();it++)
	{
		cout << *it << " ";
	}
	cout << endl;
	cout << "=====================================================" << endl;
}

//简化auto写法
void test07()
{
	vector<int> v{ 10,20,30,40,50 };
	for (auto it = v.begin();it != v.end();it++)
	{
		cout << *it << " ";
	}
	cout << endl;
	cout << "=====================================================" << endl;
}

//范围for循环
void test08()
{
	vector<int> v{ 10,20,30,40,50,60,70 };
	for (int x : v)
	{
		cout << x << " ";
	}
	cout << endl;
	cout << "=====================================================" << endl;
}

//插入与删除
void test09()
{
	//在头部插入100
	vector<int> v{ 30,40,50,60 };
	for (auto it = v.begin();it != v.end();it++)
	{
		cout << *it << " ";
	}
	cout << endl;
	v.insert(v.begin(), 100);
	v.erase(--v.end());//erase() 的参数必须是指向有效元素的迭代器。v.end() 指向的是最后一个元素之后的位置（哨兵位），它不是一个有效的元素。删除它会导致未定义行为，通常直接程序崩溃。
	for (auto it = v.begin();it != v.end();it++)
	{
		cout << *it << " ";
	}
	cout << endl;
	v.clear();
	cout << "=====================================================" << endl;
}

//综合实战案例
void test10()
{
	vector<int> score{ 88,92,75,96 };
	//尾部追加
	score.push_back(100);
	//遍历输出
	for (vector<int>::iterator it = score.begin();it != score.end();it++)
	{
		cout << *it << " ";
	}
	cout << "第一次遍历结束" << endl;
	for (int s : score)
	{
		cout << s << " ";
	}
	cout << "第二次遍历结束" << endl;
	cout << "元素数量size: " << score.size() << endl;
	cout << "内存容量capacity: " << score.capacity() << endl;
	//删除末尾
	score.pop_back();
	score.clear();
	cout << "清空后size:" << score.size() << endl;
	cout << "=====================================================" << endl;
}

int main()
{
	test04();
	test05();
	test04();//看一看同名变量会怎么样,test本来就是一个作用域，调用完就释放了
	test06();
	test07();
	test08();
	test09();
	test10();

	system("pause");
}