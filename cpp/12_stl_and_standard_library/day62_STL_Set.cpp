#include <iostream>
#include <set>
#include <vector>
using namespace std;

//常用构造
void test01()
{
	//1.空集合
	set<int> s1;
	//2.初始化列表构造
	set<int> s2{ 5,3,6,9,1,3 };
	//3.拷贝构造
	set<int> s3(s2);
}

//增删查核心API
void test02()
{
	//1.插入insert()
	set<int> s;
	s.insert(10);
	s.insert(2);
	s.insert(1);
	//2.查找find()
	auto it = s.find(2);
	if (it != s.end())
	{
		cout << "存在元素" << *it << endl;
	}
	//3.删除erase()
	set<int> s1{ 1,2,3,4,5 };
	s.erase(3);//按值删除
	s.erase(s.begin());//按迭代器删除
	s.erase(s.begin(), s.end());//删除区间，清空容器
	//4.辅助接口
	s.size();
	s.empty();
	s.clear();
}

//迭代器遍历（唯一遍历方式）
void test03()
{
	set<int> s{ 7,2,4,1 };
	//正向遍历
	for (set<int>::iterator it = s.begin();it != s.end();it++)
	{
		cout << *it << " ";
	}
	cout << endl;
	//反向遍历
	for (auto it = s.rbegin();it != s.rend();it++)
	{
		cout << *it << " ";
	}
	cout << endl;
}

//自定义排序规则
void test04()
{
	//降序集合
	set<int, greater<int>> s{ 3,1,8,1 };
	for (auto x : s)
	{
		cout << x << " ";
	}
	cout << endl;
}

//数组去重并排序
void test05()
{
	vector<int> v{ 11,3,5,6,44,11,4,5 };
	set<int> st;
	//将数组全部插入set自动去重排序
	for (int num : v)
	{
		st.insert(num);
	}
	cout << "去重升序结果： " << endl;
	for (auto val : st)
		cout << val << " ";
}

int main()
{
	test03();
	cout << "========================" << endl;
	test04();
	cout << "========================" << endl;
	test05();

	system("pause");
	return 0;
}