#include <iostream>
#include <string>
using namespace std;

int main()
{
	//1.空字符串
	string s1;
	//2.用字符串常量初始化
	string s2("hello");
	//3.拷贝构造
	string s3(s2);
	//4.n个相同字符
	string s4(5, 'a');
	cout << s1 << " " << s2 << " " << s3 << " " << s4 << endl;
	//赋值操作 =/assign
	string str;
	str = "test";
	cout << str << endl;
	str.assign("abcde");
	cout << str << endl;
	str.assign(3, 'x');
	cout << str << endl;
	//字符串拼接
	string s = "abc";
	s += "123";
	cout << s << endl;
	s.append("xyz");
	cout << s << endl;
	s.append(2, 'k');
	cout << s << endl;
	//查找
	string s5 = "abc123abc";
	int pos = s5.find("abc");
	if (pos != string::npos)
	{
		cout << "找到，下标： " << pos << endl;
	}
	string s6 = "hello world hello C++";
	string target = "hello";
	int pos1 = 0;
	int count = 0;
	while ((pos1 = s6.find(target, pos1)) != string::npos)
	{
		cout << "找到位置： " << pos1 << endl;
		count++;
		pos1 += target.length();
	}
	cout << "共找到" << count << "次" << endl;
	//截取字串substr
	string s7 = "0123456";
	string sub = s7.substr(1, 3);
	cout << sub << endl;
	//替换replace
	string s8 = "aaaaaa";
	cout << s8.replace(2, 3, "bb") << endl;
	//插入insert&删除erase
	string s9 = "12345";
	cout << s9.insert(2, "***") << " " << s9.erase(0, 3) << endl;
	//访问单个字符
	string s10 = "1234";
	cout << s10[2] << " " << s10.at(2) << endl;
	//获取长度
	cout << s10.size() << " " << s10.length() << endl;

	system("pause");
	return 0;
}