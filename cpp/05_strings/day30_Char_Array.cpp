#include <iostream>
using namespace std;
int main()
{
	//字符数组初始化方式一：逐个字符赋值
	char str1[20] = { 'H','e','l','l','o','\0' };
	//字符数组初始化方式二：直接写字符串
	char str2[20] = "Hello";
	//输出字符数组
	cout << "str1: " << str1 << endl;
	cout << "str2: " << str2 << endl;
	//单个下标访问字符
	cout << "str2[0] = " << str2[0] << endl;
	cout << "str2[1] = " << str2[1] << endl;
	//键盘输入字符串（不含空格）
	char str3[20]; 
	cout << "请输入字符串（不含空格）: " << endl;
	cin >> str3;
	cout << "你输入的内容： " << str3 <<endl;
	// ✅ 关键修复：清除输入缓冲区中的残留换行符
	cin.ignore(10000, '\n');  // 丢弃直到换行符的所有字符
	/*cin.ignore() 的作用
		cpp
		cin.ignore(10000, '\n');
	参数 1：最多忽略多少个字符（10000 是个足够大的数）
	参数 2：直到遇到什么字符停止（这里是换行符 \n）
	作用： 清除 cin >> 留下的换行符，确保后续的 getline() 能正常等待用户输入。
*/
	//循环遍历字符数组
	cout << "逐个字符遍历： " << endl;
	for (int i = 0;str3[i] != '\0';i++)
	{

		cout << str3[i] << " ";
	}
	cout << endl;
	//使用cin.getline()，可以接受整行内容（包括空格）
	char str4[50];
	cout << "请输入一句话（可带空格）： " << endl;
	cin.getline(str4, 50);
	cout << "内容： " << str4 << endl;
	system ("pause");
	return 0;
}