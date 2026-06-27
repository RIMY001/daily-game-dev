#include "workerManager.h"

//构造函数
WorkerManager::WorkerManager()
{
	//初始化属性
	this->m_EmpNum = 0;
	this->m_EmpArray = NULL;
}

//展示菜单
void WorkerManager::Show_Menu() 
{
	cout << "************************************************" << endl;
	cout << "************* 欢迎使用职工管理系统！ ***********" << endl;
	cout << "**************** 0.退出管理程序 ****************" << endl;
	cout << "**************** 1.增加职工信息 ****************" << endl;
	cout << "**************** 2.显示职工信息 ****************" << endl;
	cout << "**************** 3.删除离职职工 ****************" << endl;
	cout << "**************** 4.修改职工信息 ****************" << endl;
	cout << "**************** 5.查找职工信息 ****************" << endl;
	cout << "**************** 6.按编号排序   ****************" << endl;
	cout << "**************** 7.清空所有文档 ****************" << endl;
	cout << "************************************************" << endl;
	cout << endl;
}

//退出系统
void WorkerManager::ExitSystem()
{
	cout << "欢迎下次使用" << endl;
	system("pause");
	exit(0);//退出程序
}

//添加员工
void WorkerManager::AddEmp()
{
	cout << "请输入添加职工数量： " << endl;
	int addNum = 0;//保存用户的输入数量
	cin >> addNum;
	if (addNum > 0)
	{
		//添加
		//计算添加新空间大小
		int newSize = this->m_EmpNum + addNum;
		//开辟新空间
		Worker** newSpace = new Worker * [newSize];
		//将原来空间下数据，拷贝到新空间
		if (this->m_EmpArray != NULL)
		{
			for (int i = 0; i < this->m_EmpNum;i++)
			{
				newSpace[i] = this->m_EmpArray[i];
			}
		}
		//批量添加新数据
		for (int i = 0;i < addNum;i++)
		{

		}
	}
	else cout << "输入有误" << endl;
}

//析构函数
WorkerManager::~WorkerManager()
{

}