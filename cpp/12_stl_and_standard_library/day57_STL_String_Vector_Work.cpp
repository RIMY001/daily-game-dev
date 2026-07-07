#include <iostream>
#include <string>
#include <vector>
using namespace std;

//学生结构体
struct Student
{
	int id;
	string name;
};

//打印全部学生
void showAll(const vector<Student> &stuList)
{
	cout << "========全部学生信息=========" << endl;
	if (stuList.empty())
	{
		cout << "暂无学生数据" << endl;
		return;
	}
	for (auto& s : stuList)
	{
		cout << "学号：" << s.id << " 姓名：" << s.name << endl;
	}
	cout << "当前总人数： " << stuList.size() << "\n\n";
}

//根据姓名查找，返回对应迭代器，找不到返回end()
vector<Student>::iterator
findByName(vector<Student>& stuList, const string& name)
{
	for (auto it = stuList.begin();it != stuList.end();it++)
	{
		//string相等匹配
		if (it->name == name)
		{
			return it;
		}
	}
	return stuList.end();
}

int main()
{
	vector<Student> stuList;
	//1.新增学生
	stuList.push_back({ 1001,"张三" });
	stuList.push_back({ 1002,"李四" });
	stuList.push_back({ 1003,"王五" });
	stuList.push_back({ 1004,"张三" });
	showAll(stuList);
	//2.查找学生
	string target = "张三";
	// auto findIt = findByName(stuList, target);
	// if (findIt != stuList.end())
	// {
	// 	cout << "找到目标学生： 学号： " << findIt->id << " 姓名： " << findIt->name << "\n\n";
	// }
	// else
	// {
	// 	cout << "未找到学生： " << target << "\n\n";
	// }
	cout << "======查找结果======" << endl;
	int count = 0;
	//从开头开始找
	auto it = stuList.begin();
	while((it = find_if(it,stuList.end(),[&](const Student& s){
		return s.name == target;
	}
	)) != stuList.end())
	{
		cout << "找到目标学生： 学号： " << it->id << " 姓名： " << it->name << "\n\n";
		it++;
		count++;
	}
	//3.删除找到的学生
	// stuList.erase(findIt);
	 auto firstIt = findByName(stuList, target);
    if (firstIt != stuList.end()) {
        stuList.erase(firstIt);
        cout << "已删除第一个找到的" << target << "\n";
    }
	cout << "删除" << target << "后：\n";
	showAll(stuList);
	//4.string拓展操作：拼接姓名+学号字符串
	Student temp = { 1005,"赵六" };
	string info;
	info += "学号：";
	info += to_string(temp.id);//数字转字符串
	info += " 姓名：";
	info.append(temp.name);
	cout << "拼接字符串:" << info << endl;
	system("pause");
	return 0;
}