#include <iostream>
#include <initializer_list>
#include <cassert>

//	定义链表节点
struct ListNode {
	int val;
	ListNode* next;
	ListNode() : val(0), next(nullptr) {}
	ListNode(int x) : val(x), next(nullptr) {}
	ListNode(int x, ListNode* next) : val(x), next(next) {}
};

//	删除第n个节点
class Solution {
public:
	ListNode* removeNthFromEnd(ListNode* head, int n) {
		//	虚拟头节点
		ListNode dummy(0, head);

		ListNode* fast = &dummy;
		ListNode* slow = &dummy;

		//	快指针提前走n步
		for (int i = 0; i < n; ++i) {
			fast = fast->next;
		}

		//	同步走，快指针走到最后的结点时，慢指针在目标指针前驱
		while (fast->next) {
			fast = fast->next;
			slow = slow->next;
		}

		//	执行删除
		ListNode* toDelete = slow->next;
		assert(slow != nullptr && slow->next != nullptr);
		slow->next = slow->next->next;

		return dummy.next;
	}
};

//	测试辅助函数
ListNode* buildList(std::initializer_list<int> vals) {
	ListNode dummy;
	ListNode* curr = &dummy;
	for (int v : vals) {
		curr->next = new ListNode(v);
		curr = curr->next;
	}
	return dummy.next;
}

void printList(ListNode* head) {
	std::cout << "[";
	while (head) {
		std::cout << head->val;
		if (head->next) std::cout << ",";
		head = head->next;
	}
	std::cout << "]" << std::endl;
}

void freeList(ListNode* head) {
	while (head) {
		ListNode* tmp = head->next;
		head = head->next;
		delete tmp;
	}
}

int main() {
	Solution sol;

	// 测试用例1: 普通删除
	auto l1 = buildList({ 1, 2, 3, 4, 5 });
	printList(sol.removeNthFromEnd(l1, 2));  // [1,2,3,5]

	// 测试用例2: 删除唯一节点 ← 原来代码会崩的地方
	auto l2 = buildList({ 1 });
	printList(sol.removeNthFromEnd(l2, 1));  // []

	// 测试用例3: 删除尾节点
	auto l3 = buildList({ 1, 2 });
	printList(sol.removeNthFromEnd(l3, 1));  // [1]

	// 测试用例4: 删除头节点 ← 另一个容易崩的场景
	auto l4 = buildList({ 1, 2, 3 });
	printList(sol.removeNthFromEnd(l4, 3));  // [2,3]

	return 0;
}