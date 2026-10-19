#include <iostream>
#include <vector>
#include <numeric>

//	创建vecetor
void test01() {
	std::vector<int> vec01;
	std::vector<int> vec02(5);
	std::vector<int> vec03(5, 10);
	std::vector<int> vec04 = { 1, 2, 3, 4 };
}

//	添加元素
void test02() {
	std::vector<int> vec;
	vec.push_back(100);
}

//	访问元素
void test03() {
	std::vector<int> vec = { 1,2,3,4 };
	//	使用下标访问
	int x = vec[0];	// 不检查越界，速度更快
	//	使用at()
	int y = vec.at(1);	//	at()会检查越界，更安全
}

//	获取大小
void test04() {
	std::vector<int> vec = { 1,2,3,4 };
	std::cout << vec.size();

	//	size和capacity的区别
	std::vector<int> vec01;
	vec.push_back(1);
	vec.push_back(2);
	std::cout << vec01.size() << std::endl;	//	当前元素数量。
	std::cout << vec01.capacity() << std::endl;	//	当前已分配的内存容量
}

//	vector扩容机制 与 reserve()预分配空间
void test05() {
	//	可能发生多次内存分配
	std::vector<int> vec;
	for (int i = 0; i < 1000000; ++i) {
		vec.push_back(i);
	}

	//	提前分配内存
	std::vector<int> vec01;
	vec01.reserve(1000000);
}

//	遍历vector
void test06() {
	std::vector<int> vec = { 1,2,3,4 };
	//	使用下标访问
	for (size_t i = 0; i < vec.size(); ++i) {
		std::cout << vec[i] << std::endl;
	}
	//	使用迭代器
	for (auto it = vec.begin(); it != vec.end(); ++it) {
		std::cout << *it << std::endl;
	}
	//	范围for循环
	for (int element : vec) {
		std::cout << element << std::endl;
	}
}

//	删除元素 与 插入
void test07() {
	std::vector<int> vec = { 1,2,3 };
	vec.erase(vec.begin() + 2);	//	vector删除中间元素时，后面的元素会整体前移。
	vec.insert(vec.begin(), 100);
}

//	迭代器失效
void test08() {
	std::vector<int> vec = { 1,2,3 };
	auto it = vec.begin();
	vec.push_back(4);
	//	std::cout << *it;	//	这里的it可能已经失效。因为push_back()可能导致重新分配内存。
}

//	clear()的注意事项
void test09() {
	std::vector vec = { 1,2,3 };
	vec.clear();	//	claer只会清空元素：size变为0,capacity可能仍然保留
	//	如果希望释放内存
	std::vector<int>().swap(vec);	//swap 这个技巧本质交换内部指针，不是拷贝元素，所以效率很高，O (1)。
	std::vector vec01 = { 2,3,4 };

	//	或者
	vec.shrink_to_fit();	//	向标准库请求释放多余容量，不是强制，库可以不执行
	//	注意：C++11 之后 shrink_to_fit 是推荐写法，但它只是请求；如果需要可靠强制释放，历史上常用 swap 临时对象方案
}

//	push_back 和 emplace_back
void test10() {
	std::cout << "现代C++更推荐使用：emplace_back()" << std::endl;
/*例如：
vec.push_back(Person("Tom", 20));可能产生临时对象。而：
vec.emplace_back("Tom", 20);会直接在 vector内部原地构造对象。通常效率更高。*/	
}

int main() {
	std::vector<int> scores = { 85,90,78,92 };
	scores.push_back(88);
	std::cout << "所有成绩：";
	for (int score : scores) {
		std::cout << score << " ";
	}

	std::cout << std::endl;
	int total = std::accumulate(scores.begin(), scores.end(), 0);
	double average = total * 1.0 / scores.size();
	std::cout << "平均成绩：" << average << std::endl;
	scores.erase(scores.begin() + 2);
	std::cout << "删除后成绩: ";
	for (int score : scores) {
		std::cout << score << " ";
	}

	std::cout << std::endl;

	return 0;
}