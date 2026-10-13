#include <iostream>
#include <unordered_set>

using namespace std;

//	链表定义
struct ListNode {
	int val;
	ListNode* next;
	ListNode(int x) : val(x), next(nullptr) {}
};

//	快慢指针判环
class Solution {
public:
	bool hasCycle(ListNode* head) {
		if (!head || !head->next) return false;

		ListNode* slow = head;
		ListNode* fast = head;

		while (fast && fast->next) {	//	双重检查，防止偶数长度无环时，发生空指针解引用
			slow = slow->next;
			fast = fast->next->next;

			if (slow == fast) return true;
		}

		return false;
	}
};

//	安全释放链表
void freeList(ListNode* head) {
	unordered_set<ListNode*> visited;
	ListNode* p = head;
	while (p && visited.insert(p).second) {	//	visited.innsert(p).second返回true则表插入成功，是新节点，安全释放；false则是环，停止释放
		ListNode* next = p->next;
		delete p;
		p = next;
	}
}

//	创造测试链表
ListNode* createTestList(bool hasCycle, int cyclePos) {
	//	创造5个节点
	ListNode* nodes[5];
	for (int i = 0; i < 5; ++i) {
		nodes[i] = new ListNode(i);
	}
	for (int i = 0; i < 4; ++i) {
		nodes[i]->next = nodes[i + 1];
	}

	//	根据参数决定是否成环
	if (hasCycle && cyclePos >= 0 && cyclePos < 5) {
		nodes[4]->next = nodes[cyclePos];	//	尾节点指向cyclePos
	}
	return nodes[0];
}

int main() {
	Solution sol;

	// 测试用例 1：有环，尾节点连接到索引 1 的位置
	std::cout << "=== 测试用例 1: 有环 (pos=1) ===" << std::endl;
	ListNode* list1 = createTestList(true, 1);
	std::cout << "hasCycle: " << (sol.hasCycle(list1) ? "true" : "false") << std::endl;
	freeList(list1); // 安全释放带环链表

	// 测试用例 2：无环
	std::cout << "\n=== 测试用例 2: 无环 ===" << std::endl;
	ListNode* list2 = createTestList(false, -1);
	std::cout << "hasCycle: " << (sol.hasCycle(list2) ? "true" : "false") << std::endl;
	freeList(list2);

	// 测试用例 3：空链表
	std::cout << "\n=== 测试用例 3: 空链表 ===" << std::endl;
	std::cout << "hasCycle: " << (sol.hasCycle(nullptr) ? "true" : "false") << std::endl;

	// 测试用例 4：单节点自环
	std::cout << "\n=== 测试用例 4: 单节点自环 ===" << std::endl;
	ListNode* list4 = new ListNode(0);
	list4->next = list4;
	std::cout << "hasCycle: " << (sol.hasCycle(list4) ? "true" : "false") << std::endl;
	freeList(list4);

	return 0;
}
