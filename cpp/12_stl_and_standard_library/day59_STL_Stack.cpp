#include <iostream>
#include <stack>
#include <string>
using namespace std;

void test01()
{
	stack<int> st;
	//入栈
	st.push(10);
	st.push(20);
	st.push(30);
	cout << "栈的大小：" << st.size() << endl;
	//循环出栈，直到栈空
	while (!st.empty())
	{
		cout << "栈顶:" << st.top() << endl;
		st.pop();
	}
	cout << "出栈完毕，当前栈大小：" << st.size() << endl;
}

//括号匹配校验
bool checkBracket(const string& s)
{
	stack<char> st;
	for (char ch : s)
	{
		if (ch == '(')
		{
			st.push(ch);
		}
		else if (ch == ')')
		{
			//右括号但栈空，缺少左括号
			if (st.empty())
				return false;
			st.pop();
		}
	}
	//遍历结束栈必须为空，否则左括号多余
	return st.empty();
}

void test02()
{
	string s1 = "()234(12())";
	string s2 = "((123)";
	cout << boolalpha;
	cout << s1 << "是否合法： " << checkBracket(s1) << endl;
	cout << s2 << "是否合法： " << checkBracket(s2) << endl;
}

int main()
{
	test01();
	test02();

	system("pause");
	return 0;
}