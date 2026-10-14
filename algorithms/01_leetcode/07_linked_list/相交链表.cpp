#include <iostream>
using namespace std;

//	1.链表节点定义
struct ListNode {
	int val;
	ListNode* next;
	ListNode(int x) : val(x), next(nullptr) {}
};

class Solution {
public:
	ListNode* getIntersectionNode(ListNode* headA, ListNode* headB) {
		if (!headA || !headB) return nullptr;

		ListNode* pA = headA;
		ListNode* pB = headB;

		// 当 pA == pB 时退出：要么在交点相遇，要么同时为 nullptr
		while (pA != pB) {
			//	走到末尾则切换到另一条链表的头部
			pA = (pA == nullptr) ? headB : pA->next;
			pB = (pB == nullptr) ? headA : pB->next;
		}
		return pA;	// 相交节点或 nullptr
	}
};

void printResult(ListNode* node) {
	if (node) {
		cout << "相交节点值为：" << node->val << endl;
	}
	else {
		cout << "无相交节点" << endl;
	}
}

int main() {
    // === 测试用例 1: 存在交点 ===
    // 公共部分: 8 -> 4 -> 5
    ListNode* common1 = new ListNode(8);
    ListNode* common2 = new ListNode(4);
    ListNode* common3 = new ListNode(5);
    common1->next = common2;
    common2->next = common3;

    // 链表 A: 4 -> 1 -> [公共部分]
    ListNode* headA = new ListNode(4);
    ListNode* a2 = new ListNode(1);
    headA->next = a2;
    a2->next = common1;

    // 链表 B: 5 -> 6 -> 1 -> [公共部分]
    ListNode* headB = new ListNode(5);
    ListNode* b2 = new ListNode(6);
    ListNode* b3 = new ListNode(1);
    headB->next = b2;
    b2->next = b3;
    b3->next = common1; // 指向同一个内存地址，形成真实相交

    Solution sol;
    std::cout << "测试1: ";
    printResult(sol.getIntersectionNode(headA, headB));

    // === 测试用例 2: 无交点 ===
    ListNode* noIntersectA = new ListNode(1);
    ListNode* noIntersectB = new ListNode(2);

    std::cout << "测试2: ";
    printResult(sol.getIntersectionNode(noIntersectA, noIntersectB));

    // 注意：实际工程中应释放所有 new 出来的节点
    // 为避免重复释放公共节点，此处省略清理代码
    // 在 LeetCode 提交时不需要写 main 和内存释放

    return 0;
}