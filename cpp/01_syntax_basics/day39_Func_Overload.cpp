//疑问1：我没有设置变量直接传实参没有问题，应该是编译器临时开辟了内存，函数调用完就释放。询问猜测是否正确，以及这种现象叫什么
//疑问2：实例用有返回值类型的函数，我没采用，因为感觉返回值也可以做到这个功能。思考两种的不同，询问return返回的值其内存什么时候释放？以及什么时候用有返回值的好，什么时候无返回值的好？


#include <iostream>
using namespace std;

////1.参数类型不同
//void add(int a, int b)
//{
//	cout  << "参数为整型： " << a + b << endl;
//}
//void add(double a, double b)
//{
//	cout << "参数类型为浮点型： " << a + b << endl;
//}

////2.参数个数不同
//void print(string c1)
//{
//	cout << "打印一个角色： " << c1 << endl;
//}
//void print(string a,string  b)
//{
//	cout << "打印两个角色： " << a << " " << b <<endl;
//}

//3.参数顺序不同
void showCharacter(string name, int level)
{
	cout << "角色姓名： " << name << "	" << "角色等级： " << level << endl;
}
void showCharacter(int level,string name)
{
	cout << "角色姓名： " << name << "	" << "角色等级： " << level << endl;
}


int main()
{
	////add(3, 4);			
	////add(3.14, 6.28);

	//print("雅");
	//print("仪玄");
	//print("雅", "仪玄");

	showCharacter("雅", 80);
	showCharacter(80, "雅");

	system("pause");
	return 0;
}
/*疑问 1：临时内存与“右值”
你的猜测完全正确！当你写 showPerson(book, 3) 时，如果函数定义是 void showPerson(Person* p, int n)，这里的 3 是一个字面量（Literal）。
现象名称：这在 C++ 中被称为 “临时对象” 或更专业的术语 “右值（Rvalue）”。
底层发生了什么：
编译器确实会在栈（Stack）上为这个 3 开辟一个极小的空间（通常是 4 字节）。
它把这个值传给函数的形参 n。
当 showPerson 函数执行完毕，栈帧销毁，这块存放 3 的内存也就随之释放了。
为什么没问题：因为这种临时变量的生命周期虽然短，但足以覆盖整个函数的执行过程。这是 C++ 处理常量传递的标准方式，非常高效。
疑问 2：返回值 vs 无返回值（void）
这是一个关于“数据流向”和“职责分离”的设计问题。
1. return 返回的值，内存什么时候释放？
这取决于返回的是什么类型：
基本类型（如 int, float）：返回值通常存放在 CPU 寄存器（如 EAX/RAX）中。它不属于堆也不属于栈，随着指令执行瞬间就完成了传递，不存在“释放”的概念，因为它本来就没占大块内存。
复杂对象（如 string, vector）：现代 C++ 有返回值优化（RVO/NRVO）。编译器会直接在调用者的内存空间里构造对象，避免了拷贝。如果没优化，它会先在函数栈上创建，拷贝给调用者后，函数栈销毁时释放。
2. 什么时候用“有返回值”，什么时候用“无返回值”？
我们可以通过“你想让函数做什么”来决定：
场景	推荐写法	理由
纯计算/查询	✅ 有返回值	例如 int getSum(), bool isAlive()。函数的目的是“产出”一个结果，不改变外部状态。
修改状态/执行动作	✅ 无返回值 (void)	例如 printInfo(), sortArray(), takeDamage()。函数的目的是“产生副作用”（改数据、打印、联网）。
既要修改又要反馈	⚠️ 混合使用	比如 bool saveFile()。用 void 做保存动作，用 bool 返回是否成功。
💡 深度思考：为什么有时候必须用 return？
假设你要写一个函数判断玩家是否死亡：
写法 A（无返回值，靠引用修改）：
cpp
  void checkDeath(Player& p, bool& isDead) {
      if (p.hp <= 0) isDead = true;
  }
  // 调用：checkDeath(p, result); 比较啰嗦
写法 B（有返回值）：
cpp
  bool checkDeath(const Player& p) {
      return p.hp <= 0;
  }
  // 调用：if (checkDeath(p)) { ... } 逻辑非常清晰
总结建议：
如果函数是为了“得到一个答案”，请用 return。
如果函数是为了“干一件活”（如排序、显示、修改数组内容），请用 void。
在你的通讯录案例中，showPerson 只是负责显示，不产生新数据，所以用 void 是最合适的。
🚀 进阶小贴士
在 UE5 或大型引擎中，如果一个函数既想修改对象又想返回状态，通常会结合使用：
cpp
// 返回布尔值表示操作是否成功，同时通过引用修改了 player 的血量
bool applyDamage(Player& player, int amount) {
    if (player.isInvincible) return false;
    player.hp -= amount;
    return true;
}
这种设计能让代码的逻辑链条（Chain of Logic）非常顺畅！*/