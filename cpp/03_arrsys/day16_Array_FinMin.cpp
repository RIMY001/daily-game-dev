#include <iostream>
#include <Windows.h>
using namespace std;

int main()
{
    
    int array[] = { 11, 6, 33, 44, 3, 68 };
    int len = sizeof(array) / sizeof(array[0]);

    // 将 minValue 定义在循环外部,并初始化为第一个元素
    int minValue = array[0];

    for (int i = 1; i < len; i++)
    {
        if (array[i] < minValue)
        {
            minValue = array[i];
        }
    }

    cout << "minvalue: " << minValue << endl;
    system("pause");
    return 0;
}