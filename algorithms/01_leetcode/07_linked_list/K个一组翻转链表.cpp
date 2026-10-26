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
	ListNode* reverseKGroup(ListNode* head, int k) {
		ListNode dummy(0, head);
		ListNode* prev = &dummy;	//	前驱指针，指向要反转的局部链表的前一个节点

		//	局部反转：1.检查剩余节点是否有K个；2.有则局部反转；3.将局部链表接回原链表中
		while (true) {
			ListNode* check = prev->next;
			for (int i = 0; i < k; ++i) {
				if (!check) return dummy.next;
				check = check->next;
			}

			//	反转局部链表
			ListNode* prevLoc = nullptr;
			ListNode* curr = prev->next;
			ListNode* first = prev->next;	//	记录反转后尾节点
			for (int j = 0; j < k; ++j) {
				ListNode* nxt = curr->next;
				curr->next = prevLoc;
				prevLoc = curr;
				curr = nxt;
			}

			//	连接成完整链表
			prev->next = prevLoc;;
			first->next = curr;
			//	移动前驱节点
			prev = first;
		}

		return dummy.next;
	}
};

//	------------------ 测试辅助函数 ------------------
ListNode* buildList(const std::vector<int>& nums) {
	if (nums.empty()) return nullptr;
	ListNode dummy(0);
	ListNode* tail = &dummy;
	for (int num : nums) {
		tail->next = new ListNode(num);
		tail = tail->next;
	}
	return dummy.next;
}

void freeList(ListNode* head) {
	while (head) {
		ListNode* temp = head;
		head = head->next;
		delete temp;
	}
}

void printList(ListNode* head) {
	while (head) {
		std::cout << head->val;
		if (head->next) std::cout << " -> ";
		head = head->next;
	}
	std::cout << " -> nullptr" << std::endl;
}

// ---------- 测试 ----------
int main() {
	Solution sol;

	// 测试用例 1: [1,2,3,4,5], k=2 → [2,1,4,3,5]
	{
		ListNode* head = buildList({ 1, 2, 3, 4, 5 });
		std::cout << "原链表:   ";
		printList(head);
		ListNode* res = sol.reverseKGroup(head, 2);
		std::cout << "k=2 结果: ";
		printList(res);
		freeList(res);
	}

	// 测试用例 2: [1,2,3,4,5], k=3 → [3,2,1,4,5]
	{
		ListNode* head = buildList({ 1, 2, 3, 4, 5 });
		std::cout << "原链表:   ";
		printList(head);
		ListNode* res = sol.reverseKGroup(head, 3);
		std::cout << "k=3 结果: ";
		printList(res);
		freeList(res);
	}

	// 测试用例 3: k=1（不翻转）
	{
		ListNode* head = buildList({ 1, 2, 3 });
		std::cout << "原链表:   ";
		printList(head);
		ListNode* res = sol.reverseKGroup(head, 1);
		std::cout << "k=1 结果: ";
		printList(res);
		freeList(res);
	}

	// 测试用例 4: 节点数不足 k
	{
		ListNode* head = buildList({ 1, 2 });
		std::cout << "原链表:   ";
		printList(head);
		ListNode* res = sol.reverseKGroup(head, 3);
		std::cout << "k=3 结果: ";
		printList(res);
		freeList(res);
	}

	return 0;
}