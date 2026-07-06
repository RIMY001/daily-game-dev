#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <ctime>
using namespace std;

#define CEHUA  0
#define MEISHU 1
#define YANFA  2

class Worker
{
public:
	string m_Name;
	int m_Salary;
};

void createWorker(vector<Worker> &v)
{
	string nameSeed = "ABCDEFGHIJ";
	for (int i = 0;i < 10;i++)
	{
		Worker worker;
		worker.m_Name = "员工";
		worker.m_Name += nameSeed[i];
		worker.m_Salary = rand()%1000 + 1000;
		v.push_back(worker);
	}
}

void setGroup(vector<Worker>&v,multimap<int,Worker>& m)
{
	for (vector<Worker>::iterator it = v.begin();it != v.end();it++)
	{
		//随机产生部门编号
		int deptId = rand() % 3;
		//将员工插入到分组中
		m.insert(make_pair(deptId, *it));
		//key部门编号，value具体员工
	}
}

void showWorkerByGroup(multimap<int,Worker>&m)
{
	cout << "策划部门" << endl;
	multimap<int, Worker>::iterator pos = m.find(CEHUA);
	int count = m.count(CEHUA);//统计具体人数
	int index = 0;
	for (;pos != m.end() && index < count;pos++,index++)
	{
		cout << "姓名： " << pos->second.m_Name << "工资： " << pos->second.m_Salary << endl;
	}
	cout << "=========================================" << endl;
	cout << "美术部门" << endl;
	pos = m.find(MEISHU);
	count = m.count(MEISHU);//统计具体人数
	index = 0;
	for (;pos != m.end() && index < count;pos++, index++)
	{
		cout << "姓名： " << pos->second.m_Name << "工资： " << pos->second.m_Salary << endl;
	}
	cout << "=========================================" << endl;
	cout << "研发部门" << endl;
	pos = m.find(YANFA);
	count = m.count(YANFA);//统计具体人数
	index = 0;
	for (;pos != m.end() && index < count;pos++, index++)
	{
		cout << "姓名： " << pos->second.m_Name << "工资： " << pos->second.m_Salary << endl;
	}
	cout << "=========================================" << endl;

}

int main()
{
	srand((unsigned)time(NULL));
	//1.创建10名员工，放到vector中
	vector<Worker> vWorker;
	createWorker(vWorker);

	//2.遍历vector容器，取出每个员工，进行随机分组
	multimap<int, Worker> mWorker;
	setGroup(vWorker, mWorker);

	//3.分组后，将员工编号作为key,具体员工作为value，放到multimap容器中
	showWorkerByGroup(mWorker);

	//4.分部门显示员工信息

	system("pause");
	return 0;
}