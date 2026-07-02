#include <iostream>
using namespace std;

class Base
{
public:
    virtual void fun() const { cout << "父类" << endl; }//不加const,test02()报错，原因是fun()可能会修改只读
};
class Son :public Base
{
public:
    void fun() const override { cout << "子类" << endl; }//父类加const,子类不加，多态失效，如果父类和子类的函数签名有一丁点不匹配（比如参数不同、或者父类没加 virtual），多态就会失效。
};
void test(Base b)
{
    b.fun();

}
void test02(const Base &b)
{
    b.fun();

}

int main() 
{
    Son s;
    test(s);
    test02(s);

    system("pause");
    return 0;

    
}

//int main() {
//    const char* str1 = "hello";
//    const char* str2 = "hello";  // 相同的字符串
//
//    cout << (void*)str1 << endl;  // 输出：0x7ff... 
//    cout << (void*)str2 << endl;  // 输出：0x7ff... （相同地址！）
//
//    system("pause");
//    return 0;
//
//    // str1 和 str2 指向同一个地址
//    // 证明编译器优化：相同的字符串字面量只存储一份
//}

////全局变量：全局静态区
//int g_num = 10;
//static int s_num = 20;
//const double PI = 3.14;//常量区
//
//
//int main()
//{
//	//栈区
//	int a = 1;
//	int arr[10];
//	//堆区
//	int* p = new int(100);
//	//字符串：常量区
//	const char* str = "hello";
//	delete p;
//
//	system("pause");
//	return 0;
//}