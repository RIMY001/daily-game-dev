#include <iostream>
#include <string>
using namespace std;

template <typename T>
bool myCompare(const T& a, const T& b) {
    cout << "[通用模板] ";
    return a > b;
}

// 具体化模板：处理 Person
struct Person {
    string name;
    int age;
};

template<>
bool myCompare<Person>(const Person& p1, const Person& p2) {
    cout << "[具体化模板-Person] ";
    return p1.age > p2.age;
}

int main()
{
    int a = 10, b = 20;
    cout << "10 > 20 ? " << myCompare(a, b) << endl;

    Person p1 = { "Zhang", 25 };
    Person p2 = { "Li", 18 };
    cout << "Zhang(25) older than Li(18)? " << myCompare(p1, p2) << endl;

    system("pause");
    return 0;
}

////3.模板函数外部实现
//template<typename T>
//class MyArray
//{
//public:
//	void print(T a);
//};
//
////类外实现
//template<typename T>
//void MyArray<T>::print(T a)
//{
//	cout << a << " " << "类外实现成功" << endl;
//}
//
//int main()
//{
//	MyArray<int> arr1;
//	arr1.print(10);
//
//	system("pause");
//	return 0;
//}

////2.1通用数组容器
//template<typename T>
//class MyArray
//{
//private:
//	T arr[100];
//	int size = 0;//初始化
//public:
//	void push(T val) {
//		if (size < 100) {
//			arr[size++] = val;
//		}
//		else {
//			// 处理溢出，例如抛出异常或扩容
//		}
//	}
//	T get(int index)
//	{
//		return arr[index];
//	}
//};
//
//int main()
//{
//	//int数组
//	MyArray<int> intArr;
//	intArr.push(10);
//	intArr.push(2);
//	intArr.push(2);
//	intArr.push(26);
//	intArr.push(32);
//	cout << intArr.get(2) << endl;
//	//string数组
//	MyArray<string> strArr;
//	strArr.push("Hello");
//	strArr.push("Are");
//	strArr.push("You");
//	strArr.push("Ok");
//	cout << strArr.get(3) << endl;
//
//	system("pause");
//	return 0;
//}

//1.2多个参数类型
//template<typename T1,typename T2>
//void show(T1 a, T2 b)
//{
//	cout << a << " " << b << endl;
//}
//
//int main()
//{
//	show(3.14, "zhang");
//
//	system("pause");
//	return 0;
//}


//1.1通用求最大值
//template<typename T>
//T getMax(T a, T b)
//{
//	return a > b ? a : b;
//}
//
//int main()
//{
//	cout << getMax(10, 20) << endl;//T = int
//	cout << getMax(3.14, 1.59) << endl;//T = double
//	cout << getMax('a', 'b') << endl;//T = char
//
//	system("pause");
//	return 0;
//}