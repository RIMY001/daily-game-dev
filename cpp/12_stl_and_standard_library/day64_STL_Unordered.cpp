#include <iostream>
#include <unordered_set>
#include <unordered_map>
using namespace std;

void test01()
{
	unordered_set<int> us = { 3,1,4,1,5 };
	us.insert(9);
	us.erase(3);
	//遍历顺序不确定
	for (unordered_set<int>::iterator it = us.begin();it != us.end();it++)
	{
		cout << *it << " ";
	}
	cout << endl;
	//查找
	if (us.find(5) != us.end())
		cout << "\n找到5" << endl;
}

void test02()
{
	unordered_map<string, int> mp;
	//下标插入/修改
	mp["张三"] = 20;
	mp["李四"] = 18;
	mp["王五"] = 22;//覆盖
	//insert插入
	mp.insert({ "王五",21 });
	//遍历，顺序随机
	for (auto& item : mp)
		cout << item.first << " : " << item.second << endl;
	//查找（推荐find,不会新增key)
	auto it = mp.find("李四");
	if (it != mp.end())
		cout << it->second;
	mp.erase("王五");
	cout << endl;
 }

void test03()
{
	vector<string> words = { "apple","banana","apple" };
	unordered_map<string, int> cnt;
	for (auto& w : words) cnt[w]++;
	for (const auto& [word, count] : cnt)  // C++17 结构化绑定
		cout << word << ": " << count << endl;
}



int main()
{
	test01();
	cout << "========================= " << endl;
	test02();
	cout << "========================= " << endl;
	test03();
	cout << "========================= " << endl;

	system("pause");
	return 0;
}