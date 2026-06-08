#include <iostream>
using namespace std;
int factorial(int a)
{
	if (a==0 ||a == 1)//0!和1!的阶乘都为1
	{
		return 1;
	}
	if (a < 0)
	{
		return -1;

	}
	return a * factorial(a - 1);
}
int main()
{
	int a = 0;
	cout << "Enter a number: " << endl;
	cin >> a;
	int b = factorial(a);
	if (b == -1)
	{
		cout << "Invalid input!" << endl;
	}
	else
	{
		cout << "Factorial result: " << b << endl;
	}
	
	system("pause");
	return 0;
}
/*
#include <iostream>
using namespace std;

int factorial(int a)
{
    if (a < 0) return -1;
    int res = 1;
    for (int i = 2; i <= a; ++i)
    {
        res *= i;
    }
    return res;
}

int main()
{
    int a;
    cout << "Enter a number: " << endl;
    cin >> a;
    int ans = factorial(a);
    ans == -1 ? cout<<"Invalid input!" : cout<<"Factorial result: "<<ans;
    system("pause");
    return 0;
}
写一个计算阶乘的程序，输入数字，合法就算出阶乘结果，负数就提示输入无效。
函数部分拆解
cpp
运行
int factorial(int a)
定义名叫factorial的函数，接收一个整数，最后也要返回整数结果。
cpp
运行
if (a < 0) return -1;
小于 0 是非法数字，直接返回 - 1 当作错误标记。
return：立刻结束函数，把后面数值送回调用处。
cpp
运行
int res = 1;
定义变量存最终乘积，阶乘从 1 开始相乘，初始值固定设为 1。
cpp
运行
for (int i = 2; i <= a; ++i)
for循环：从数字 2 开始，一直循环到输入的数字 a 为止。
i <= a：循环继续的条件；++i：每次循环 i 自增 1。
cpp
运行
res *= i;
*=复合赋值运算符，等价res = res * i
不断把数字累乘，算出阶乘数值。
cpp
运行
return res;
把算好的阶乘结果返回出去。
主函数部分拆解
cpp
运行
int a;
cout << "Enter a number: " << endl;
cin >> a;
定义变量，控制台提示文字，读取键盘输入的数字。
cpp
运行
int ans = factorial(a);
调用阶乘函数，把输入的数字传进去，接收函数返回的结果存到 ans 里。
cpp
运行
ans == -1 ? cout<<"Invalid input!" : cout<<"Factorial result: "<<ans;
==相等判断运算符，判断结果是不是错误标记 - 1
? :三目运算符，简写的判断分支
条件成立执行问号后语句，不成立执行冒号后语句
cpp
运行
system("pause");
return 0;
暂停窗口方便查看结果，程序正常结束。
用到的运算符用法汇总
算术运算符
*：乘法运算
++：变量自身加 1
赋值运算符
*=：乘后赋值，累计相乘更新数值
关系运算符
<小于、<=小于等于、==判断是否相等
三目运算符
条件 ? 成立执行 : 不成立执行，精简 if 判断写法
运行逻辑举例
输入 5
循环：1×2×3×4×5，最终算出 120；
输入 - 3，直接返回 - 1，弹窗提示输入无效。
*/












