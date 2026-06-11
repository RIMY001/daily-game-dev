//我想实现传入人这样的数组（里面有人信息不全），然后达到函数默认值的效果。我只用在结构体定义的时候设置好默认值就行了

#include <iostream>
#include <string>
using namespace std;

//地址默认为空，性别未知
void showPerson(string name, string phone, string add = "", string sex = "未知")
{
	cout << "姓名： " << name << endl;
	cout << "电话： " << phone << endl;
	cout << "地址： " << add << endl;
	cout << "性别； " << sex << endl;
	cout << endl;
}

int main()
{
	//只传必填参数，使用地址、性别默认值
	showPerson("张丹", "155");
	//传入地址，性别使用默认值
	showPerson("王为", "188", "上海");
	//全部参数手动传入，覆盖默认值
	showPerson("杨", "157", "上海", "男");


	system("pause");
	return 0;
}

/*#include <iostream>
#include <string>
using namespace std;

// ✅ 1. 在结构体里直接设好“兜底”的默认值
struct Person {
    string name;
    string phone;
    string address = "未填写"; // <--- 这里设了默认值！
    string sex = "未知";       // <--- 这里也设了！
};

// ✅ 2. 函数现在不需要任何判断逻辑了，直接打印即可
void showPerson(const Person& p) {
    cout << "姓名： " << p.name << endl;
    cout << "电话： " << p.phone << endl;
    cout << "地址： " << p.address << endl; // 直接打，反正不会是空的
    cout << "性别： " << p.sex << endl;     // 直接打，反正不会是空的
    cout << "----------------" << endl;
}

int main() {
    Person p1;
    p1.name = "张丹";
    p1.phone = "155";
    // 注意：我没给 p1.address 赋值
    
    showPerson(p1); 
    // 输出结果：地址： 未填写  （因为结构体初始化时自动给了默认值）
    
    return 0;
}*/