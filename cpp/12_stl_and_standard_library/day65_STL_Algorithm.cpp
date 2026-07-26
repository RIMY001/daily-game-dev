#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
using namespace std;

void print(int val)
{
	cout << val << " ";
}

struct Person
{
	string name;
	int age;
};

//for_each遍历算法
void test01()
{
	vector<int> v = { 10,20,30,40 };
	//参数：起始迭代器，结束迭代器，回调函数
	for_each(v.begin(), v.end(), print);
	cout << endl;
	cout << "<<Lambda写法>> : " << endl;
	for_each(v.begin(), v.end(), [](int val) {cout << val << " ";});
	cout << endl;
}

//sort排序
void test02()
{
	vector<int> v = { 5,2,9,1,5,6 };
	//升序
	sort(v.begin(), v.end());
	for_each(v.begin(), v.end(), [](int val) {cout << val << " ";});
	cout << endl;
	//降序
	sort(v.begin(), v.end(), greater<int>());
	for_each(v.begin(), v.end(), [](int val) {cout << val << " ";});
	cout << endl;
	//自定义排序规则
	vector<Person> p{ {"a",18},{"b",16},{"c",20} };
	sort(p.begin(), p.end(), [](const Person& x, const Person& y) {return x.age < y.age;});
	for_each(p.begin(), p.end(), [](const Person& x) {cout << x.name << " " << x.age << endl;});
}

//find查找元素
void test03()
{
	vector<int> v{ 10,20,30 };
	for_each(v.begin(), v.end(), [](int val) {cout << val << " ";});
	cout << endl;
	auto it = find(v.begin(), v.end(), 20);
	if (it != v.end())
		cout << "找到： " << *it << endl;
}

//find_if条件查找
void test04()
{
	vector<int> v{ 10,20,30,40 };
	for_each(v.begin(), v.end(), [](int val) {cout << val << " ";});
	cout << endl;
	//查找第一个大于25的数字
	auto it = find_if(v.begin(), v.end(), [](int x) {return x > 25;});
	if (it != v.end())
		cout << "找到： " << *it << endl;
}

//count/count_if计数
void test05()
{
	vector<int> v{ 1,2,2,2,3 };
	for_each(v.begin(), v.end(), [](int val) {cout << val << " ";});
	cout << endl;
	//统计等于2的个数
	int num1 = count(v.begin(), v.end(), 2);
	cout << "统计等于2的个数: " << num1 << endl;
	//统计大于2的个数
	int num2 = count_if(v.begin(), v.end(), [](int x) {return x > 2;});
	cout << "统计大于2的个数: " << num2 << endl;
}

//copy拷贝容器元素  copy不会自动扩容，它只做赋值。目标必须有足够的预分配空间。
void test06()
{
	vector<int> src = { 1,2,3,4 };
	for_each(src.begin(), src.end(), [](int val) {cout << val << " ";});
	cout << endl;
	vector<int> dst(4);//这里的 (4) 调用的是 vector 的填充构造函数
	for_each(dst.begin(), dst.end(), [](int val) {cout << val << " ";});
	cout << endl;
	copy(src.begin(), src.end(), dst.begin());
	for_each(src.begin(), src.end(), [](int val) {cout << val << " ";});
	cout << endl;
	for_each(dst.begin(), dst.end(), [](int val) {cout << val << " ";});
	cout << endl;
}

//replace/replace_if 替换
void test07()
{
	vector<int> v{ 1,2,2,3 };
	for_each(v.begin(), v.end(), [](int val) {cout << val << " ";});
	cout << endl;
	//把所有的2替换成99
	replace(v.begin(), v.end(), 2, 99);
	for_each(v.begin(), v.end(), [](int val) {cout << val << " ";});
	cout << endl;
	//条件替换：大于3的全部替换成0
	replace_if(v.begin(), v.end(), [](int val) {return val > 3;}, 0);
	for_each(v.begin(), v.end(), [](int val) {cout << val << " ";});
	cout << endl;
}

//remove删除
void test08()
{
	//remove删除
	vector<int> v = { 1,2,3,2,5 };
	//删除所有值为2的元素
	auto newEnd = remove(v.begin(), v.end(), 2);
	v.erase(newEnd, v.end());
	for_each(v.begin(), v.end(), [](int val) {cout << val << " ";});
	cout << endl;
	//remove_if条件删除
	//删除所有大于3的数
	v.erase(remove_if(v.begin(), v.end(), [](int x) {return x > 3;}), v.end());
	for_each(v.begin(), v.end(), [](int val) {cout << val << " ";});
	cout << endl;
}

int main()
{
	test01();
	cout << "----------------------------------------------------------------" << endl;
	test02();
	cout << "----------------------------------------------------------------" << endl;
	test03();
	cout << "----------------------------------------------------------------" << endl;
	test04();
	cout << "----------------------------------------------------------------" << endl;
	test05();
	cout << "----------------------------------------------------------------" << endl;
	test06();
	cout << "----------------------------------------------------------------" << endl;
	test07();
	cout << "----------------------------------------------------------------" << endl;
	test08();
	cout << "----------------------------------------------------------------" << endl;

	system("pause");
	return 0;
}