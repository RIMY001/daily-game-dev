#include <iostream>
#include <map>
#include <string>
#include <vector>
using namespace std;

//pair前置知识
void test01()
{
	pair<int, string> p = make_pair(1001, "张三");
	cout << p.first;//key:1001
	cout << p.second;//value:张三
	cout << endl;
}

//构造与4种插入方法
void test02()
{
	map<int, string> stuMap;
	//方式1：[]下标插入（最常用）
	stuMap[1001] = "张三";
	stuMap[1002] = "李四";
	//方式2：insert + make_pair
	stuMap.insert(make_pair(1003, "王五"));
	//方式3：insert 直接大括号
	stuMap.insert({ 1004,"赵六" });
	//方式4：初始化列表创建map
	map<int, string> m2{ {1,"A"},{2,"B"} };
}

//遍历map
void test03()
{
	map<int, string> m3{ {1001,"张三"},{1002,"李四"} };
	//迭代器遍历
	for (map<int, string>::iterator it = m3.begin();it != m3.end();it++)
	{
		cout << "学号： " << it->first << "姓名： " << it->second << endl;
	}
	//范围for循环
	for (auto& item : m3)
		cout << item.first << " " << item.second << endl;
}

//查找find
void test04()
{
	map<int, string> m3{ {1001,"张三"},{1002,"李四"} };
	auto it = m3.find(1002);
	if (it != m3.end())
		cout << "找到： " << it->second << endl;
	else
		cout << "无此学号" << endl;
}

//删除erase
void test05()
{
	map<int, string> m3{ {1001,"张三"},{1002,"李四"} ,{1003,"王五"}};
	m3.erase(1001);
	auto it = m3.find(1002);
	if (it != m3.end()) m3.erase(it);
	m3.clear();
}

//辅助接口
void test06()
{
	map<int, string> m3{ {1001,"张三"},{1002,"李四"} ,{1003,"王五"} };
	cout << m3.size() << endl;
	m3.empty();
	cout << m3.count(1001) <<endl;
}

//单词计数
void test07()
{
	vector<string> words = { "apple","banana","apple","orange","banana","apple" };
	map<string, int> wordCount;
	for (auto& word : words)
		wordCount[word]++;
	for (auto& p : wordCount)
		cout << "单词： " << p.first << "出现次数： " << p.second << endl;
}

int main()
{
	test01();
	cout << "---------------------" << endl;
	test02();
	cout << "---------------------" << endl;
	test03();
	cout << "---------------------" << endl;
	test04();
	cout << "---------------------" << endl;
	test05();
	cout << "---------------------" << endl;
	test06();
	cout << "---------------------" << endl;
	test07();

	system("pause");
	return 0;
}