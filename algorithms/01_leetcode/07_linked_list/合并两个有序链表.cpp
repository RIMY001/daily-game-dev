#include <iostream>
#include <vector>
using namespace std;

struct ListNode {
	int val;
	ListNode* next;
	ListNode() : val(0), next(nullptr) {}
	ListNode(int x) : val(x), next(nullptr) {}
	ListNode(int x, ListNode* next) : val(x), next(next) {}
};

class Solution {
public:
	ListNode* mergeTwoLists(ListNode* list1, ListNode* list2) {
		//	哨兵节点
		ListNode dummy(0);
		ListNode* curr = &dummy;

		//	双指针逐一比较
		while (list1 && list2) {
			if (list1->val <= list2->val) {
				curr->next = list1;
				list1 = list1->next;
			}
			else {
				curr->next = list2;
				list2 = list2->next;
			}
			curr = curr->next;
		}

		//	拼接剩余非空链表
		curr->next = list1 ? list1 : list2;

		return dummy.next;	//	返回真正头节点
	}
};

// ============================== 测试辅助函数 ==============================
//	根据数组构建单向链表
ListNode* buildList(const vector<int>& vals) {
	if (vals.empty()) return nullptr;
	ListNode dummy(0);
	ListNode* curr = &dummy;
	for (int v : vals) {
		curr->next = new ListNode(v);
		curr = curr->next;
	}
	return dummy.next;
}

//	将链表转换回vector 以便比对结果
vector<int> listToVector(ListNode* head) {
	vector<int> res;
	while (head) {
		res.push_back(head->val);
		head = head->next;
	}
	return res;
}

//	打印vector
void printVector(const vector<int>& v) {
	cout << "[";
	for (size_t i = 0; i < v.size(); ++i) {
		cout << v[i] << (i + 1 < v.size() ? "," : "");
	}
	cout << "]";
}

//	安全释放链表
void freeList(ListNode* head) {
	while (head) {
		ListNode* next = head->next;
		delete head;
		head = next;
	}
}

int main() {
	Solution sol;

	// 测试用例：{l1数组, l2数组, 期望输出数组}
	struct TestCase {
		std::vector<int> l1;
		std::vector<int> l2;
		std::vector<int> expected;
	};

	TestCase tests[] = {
		{{1, 2, 4}, {1, 3, 4}, {1, 1, 2, 3, 4, 4}}, // 示例 1：常规交替合并
		{{},        {},        {}},                 // 示例 2：双空链表
		{{},        {0},       {0}},                // 示例 3：一空一非空
		{{1, 3, 5}, {2, 4, 6}, {1, 2, 3, 4, 5, 6}}, // 补充：完美交错
		{{5, 6, 7}, {1, 2, 3}, {1, 2, 3, 5, 6, 7}}  // 补充：整体拼接（测试 curr->next 直接拼接逻辑）
	};

	bool allPassed = true;
	for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
		// 1. 构建原始链表
		ListNode* l1 = buildList(tests[i].l1);
		ListNode* l2 = buildList(tests[i].l2);

		// 2. 执行合并
		ListNode* merged = sol.mergeTwoLists(l1, l2);

		// 3. 验证结果
		std::vector<int> actual = listToVector(merged);
		bool pass = (actual == tests[i].expected);

		std::cout << "Test " << i + 1 << ": " << (pass ? "PASS " : "FAIL ") << " | ";
		std::cout << "Expected: "; printVector(tests[i].expected);
		std::cout << ", Got: ";      printVector(actual);
		std::cout << std::endl;

		if (!pass) allPassed = false;

		// 4. 释放内存
		// 核心注意：merged 包含了 l1 和 l2 的所有节点，只能释放 merged！
		// 如果这里再去 freeList(l1) 或 freeList(l2) 会导致 Double Free 崩溃。
		freeList(merged);
	}

	std::cout << "\n" << (allPassed ? "All tests passed!" : "Some tests failed!") << std::endl;
	return 0;
}