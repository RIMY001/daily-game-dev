//1.定义初始化
//2.赋值
//3.字符串拼接
//4.获取长度
//5.下标访问单个字符
//6.字符串比较
//7.整行输入带空格
#include <iostream>
#include <string>
using namespace std;

int main()
{
	//定义初始化
	string s1 = "Hello";
	string s2("World");//这个初始化为什么可以用括号？
	string s3;
	/*为什么初始化可以用括号 string s2("World");？
	在 C++ 中，string 不是一个简单的数据类型，而是一个类（Class）。
	本质：当你写 string s2("World"); 时，你实际上是在调用这个类的构造函数（Constructor）。
	类比：就像你创建一个角色对象 Player p("Lingma"); 一样，括号里传的是初始化的参数。
	结论：这是 C++ 对象初始化的标准语法之一，和 string s2 = "World"; 效果完全一样，但后者更常用且直观。*/

	//赋值
	s3 = s1;

	//字符串拼接
	s3 = s1 + s2;

	//获取长度
	cout << "s3长度: " << s3.size() << endl;//为什么.size()这么用，（）里面不写任何东西，什么情况下用？
	/*.size() 为什么括号里不写东西？什么情况下用？
	为什么没参数：.size() 是一个成员函数。它的作用是“报告”这个字符串对象自己有多长。因为它只需要读取对象内部已经记录好的长度信息，不需要你额外提供任何数据，所以括号是空的。
	对比 C 风格：C 风格的 strlen(str) 需要你传入字符串的地址，因为它是个外部函数；而 .size() 是字符串“自带”的功能。
	什么时候用：只要你想知道字符串里有多少个字符，就用它。比如遍历字符串时：for(int i=0; i<s.size(); i++)。
	小知识：std::string 还有一个 .length()，功能和 .size() 一模一样，看个人喜好。*/

	//下标访问单个字符
	cout << "s3第一个字母是： " << s3[0] << endl;

	//字符串比较
	string a = "abs";//我看到可以这么用： string a = "abs" , b = "abn";c风格字符串也能吗？
	string b = "abn";
	if (a > b)
		cout << "a > b" << endl;
	else if (a < b)
		cout << "a < b" << endl;
	else
		cout << "a = b" << endl;

	//整行输入带空格
	string input;
	//cin.ignore();ai给出的错误 。 这个有什么用？在这里有什么意义？我记得之前有一个清除缓冲区的函数，是不是这个？
	/* cin.ignore() 有什么用？它是清除缓冲区的吗？
	没错，你的记忆非常准确！ cin.ignore() 就是我们之前讨论过的“清道夫”。
	意义：在你的代码中，前面有 cin >> ... 之类的操作吗？如果没有，其实这行 cin.ignore() 在这里不是必须的。
	潜在风险：如果这段代码前面还有别的输入（比如先输入了一个数字），缓冲区里会残留一个换行符 \n。如果不加 ignore()，下面的 getline 读到这个 \n 就会直接结束，导致你没法输入句子。
	建议：养成好习惯，在从 cin >> 切换到 getline 之前，务必加上它。
	*/
	cout << "输入一句话（带空格）： " << endl;
	getline(cin, input);//我记得有一次打过cin.getline()?为什么我这里用的时候编译器报错？这两个有什么区别？
	/*特性	cin.getline()	getline(cin, str)
	适用对象	C 风格字符串 (char str[50])	C++ 字符串 (std::string str)
	调用方式	cin.getline(数组名, 长度)	getline(输入流, 字符串变量)
	头文件	<iostream>	<string>
	安全性	⚠️ 需要手动指定长度，易溢出	✅ 自动管理内存，无限长（直到内存耗尽）
	为什么你报错了？
	因为你定义的是 string input;（C++ 风格），但你如果尝试用 cin.getline(input, 50)，编译器会懵掉，因为 cin.getline 只认识 char 数组，不认识 string 对象。正确做法：
	如果是 char str[50]：用 cin.getline(str, 50);
	如果是 string str：用 getline(cin, str); ✅ (推荐)*/
	cout << "你输入的是： " << input << endl;
	system("pause");
	return 0;

}
/*cin.ignore() 的作用是：“从缓冲区里扔掉一个字符，如果缓冲区是空的，那就停下来等着用户按一个键再扔掉。”
执行流程：
程序运行到 cin.ignore()。
此时缓冲区是空的（因为前面没有 cin >>）。
cin.ignore() 立刻阻塞，等待你输入一个字符并按回车。
关键点：这时候 cout 的那句提示语可能还在缓冲区里没来得及显示，或者因为你正在键盘上敲字，你的注意力全在光标闪烁上，导致你觉得“还没提示就让我输入”。
当你按了回车，ignore 完成任务，程序继续往下走，cout 才把提示语打印出来，然后 getline 又开始等你输入第二遍。
结论：cin.ignore() 在这里多余且有害。它强行让你多按了一次回车，打乱了程序的节奏。
2. 为什么汉字会乱码？*/