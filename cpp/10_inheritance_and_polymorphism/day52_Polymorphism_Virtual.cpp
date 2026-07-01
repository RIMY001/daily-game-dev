#include <iostream>
using namespace std;

//之所以用父类指针或引用，是因为我们想要达到多态的效果，传入子类对象，就调用该子类的成员函数

class Shape
{
public:
	virtual void getArea() = 0;//纯虚函数
};

class Circle :public Shape
{
private:
	int r;
public:
	Circle(int x):r(x){}
	void getArea()
	{
		cout << 3.14 * r * r << endl;
	}
	
};

int main()
{
	//Shape s;抽象类不能创建对象
	Circle c(2);
	c.getArea();

	system("pause");
	return 0;
}

//class Animal
//{
//public:
//	virtual void speak()//虚函数
//	{
//		cout << "动物在叫" << endl;
//	}
//};
//
//class Dog :public Animal
//{
//public:
//	void speak()
//	{
//		cout << "狗在叫" << endl;
//	}
//};
//
//class Cat :public Animal
//{
//public:
//	void speak()
//	{
//		cout << "猫在叫" << endl;
//	}
//};
//
//
////统一接口，多态体现
//void doSpeak(Animal& a)
//{
//	a.speak();
//}
//
//int main()
//{
//	Dog d;
//	Cat c;
//	doSpeak(d);
//	doSpeak(c);
//
//	system("pause");
//	return 0;
//}

//class Animal
//{
//public:
//	void speak()
//	{
//		cout << "动物在叫" << endl;
//	}
//};
//class Dog :public Animal
//{
//public:
//	void speak()
//	{
//		cout << "狗在叫" << endl;
//	}
//};
//
//int main()
//{
//	Animal* p = new Dog;
//	p->speak();//动物在叫。依旧执行父类方法，不会触发子类
//	Dog* p1 = new Dog;
//	p1->speak();//狗在叫。
//	delete p;
//
//	system("pause");
//	return 0;
//}