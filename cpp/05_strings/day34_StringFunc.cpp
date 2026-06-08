#include <iostream>
#include <string>
using namespace std;

int main()
{
	string s = "asdfghj";
	//1.substr(起始下标，截取长度),字符串截取
	string sub = s.substr(2, 4);
	cout << "截取： " << sub << endl;
	//2.insert(位置，“内容”）,指定位置插入
	s.insert(2, "123");
	cout << "插入后： " << s << endl;
	//3.erase（起始，长度），删除字符
	s.erase(2, 3);
	cout << "删除后： " << s << endl;
	//4.find("要找的串")返回下标，找不到返回string::npos
	int pos = s.find("123");
	if (pos != string::npos)
	{
		cout << "123下标： " << pos << endl;
	}
	else
		cout << "未找到" << endl;
	//5.replace(起始，长度，“新内容”),替换
	s.replace(2, 3, "xxxx");
	cout << "替换后： " << s << endl;

	system("pause");
	return 0;
}