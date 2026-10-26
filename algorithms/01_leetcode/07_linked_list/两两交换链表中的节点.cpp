#include <iostream>
#include <vector>

struct ListNode {
	int val;
	ListNode* next;
	ListNode() : val(0), next(nullptr) {}
	ListNode(int x) : val(x), next(nullptr) {}
	ListNode(int x, ListNode* next) : val(x), next(next) {}
};

class Solution {
public:
	ListNode* swapPairs(ListNode* head) {
		//	虚拟头节点
		ListNode dummy(0, head);
		ListNode* prev = &dummy;	//	前驱指针：始终保持在要交换的两节点前

		//	在链表中访问 node->next->xxx 之前，永远先确认 node->next != nullptr。
		while (prev->next && prev->next->next) {
			ListNode* first = prev->next;
			ListNode* second = prev->next->next;

			//	执行交换
			first->next = second->next;
			prev->next = second;
			second->next = first;

			//	前驱移动两个节点
			prev = first;
		}

		return dummy.next;
	}
};

//	================== 测试辅助函数 ==================
//	从 vector 构建链表
ListNode* buildList(const std::vector<int>& nums) {
	ListNode dummy(0);
	ListNode* tail = &dummy;
	for (int num : nums) {
		tail->next = new ListNode(num);
		tail = tail->next;
	}
	return dummy.next;
}

//	打印链表
void printList(ListNode* head) {
	while (head) {
		std::cout << head->val;
		if (head->next) std::cout << "->";
		head = head->next;
	}
	std::cout << "-> nullptr" << std::endl;
}

//	释放链表
void freeList(ListNode* head) {
	while (head) {
		ListNode* tmp = head;
		head = head->next;
		delete tmp;
	}
}

//	================== 测试入口 ==================
int main() {
	Solution sol;

	// 测试用例1：偶数个节点
	std::cout << "=== Test 1:[1,2,3,4] ===" << std::endl;
	ListNode* list1 = buildList({ 1,2,3,4 });
	std::cout << "Before: ";printList(list1);
	ListNode* res1 = sol.swapPairs(list1);
	std::cout << "After: "; printList(res1);
	freeList(res1);

	// 测试用例2: 奇数个节点
	std::cout << "\n=== Test 2: [1,2,3] ===" << std::endl;
	ListNode* list2 = buildList({ 1, 2, 3 });
	std::cout << "Before: "; printList(list2);
	ListNode* res2 = sol.swapPairs(list2);
	std::cout << "After:  "; printList(res2);
	freeList(res2);

	// 测试用例3: 空链表
	std::cout << "\n=== Test 3: [] ===" << std::endl;
	ListNode* list3 = buildList({});
	std::cout << "Before: "; printList(list3);
	ListNode* res3 = sol.swapPairs(list3);
	std::cout << "After:  "; printList(res3);
	freeList(res3);

	// 测试用例4: 单节点
	std::cout << "\n=== Test 4: [1] ===" << std::endl;
	ListNode* list4 = buildList({ 1 });
	std::cout << "Before: "; printList(list4);
	ListNode* res4 = sol.swapPairs(list4);
	std::cout << "After:  "; printList(res4);
	freeList(res4);

	return 0;
}