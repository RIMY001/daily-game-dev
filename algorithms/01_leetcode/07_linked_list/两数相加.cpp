#include <iostream>
#include <vector>

//	链表节点定义
struct ListNode {
	int val;
	ListNode* next;
	ListNode() : val(0), next(nullptr) {}
	ListNode(int x) : val(x), next(nullptr) {}
	ListNode(int x, ListNode* next) : val(x), next(next) {}
};

class Solution {
public:
	ListNode* addTwoNumbers(ListNode* l1, ListNode* l2) {
		ListNode dummy(0);
		ListNode* curr = &dummy;
		int carry = 0;

		//	carry 也作为循环条件，自动处理最高位进位
		while (l1 || l2 || carry) {
			int v1 = l1 ? l1->val : 0;	//	短链表补零
			int v2 = l2 ? l2->val : 0;

			int sum = v1 + v2 + carry;
			carry = sum / 10;

			curr->next = new ListNode(sum % 10);
			curr = curr->next;

			if (l1) l1 = l1->next;
			if (l2) l2 = l2->next;
		}

		return dummy.next;
	}
};

//	测试辅助函数

//	根据数组构建逆序链表
ListNode* buildList(const std::vector<int>& vals) {
	if (vals.empty()) return nullptr;
	ListNode dummy(0);
	ListNode* curr = &dummy;
	for (int v : vals) {
		curr->next = new ListNode(v);
		curr = curr->next;
	}

	return dummy.next;
}

//	将链表转换回 vector ，对比结果
std::vector<int> listToVector(ListNode* head) {
	std::vector<int> res;
	while (head) {
		res.push_back(head->val);
		head = head->next;
	}
	return res;
}

//	打印 vector
void printVector(const std::vector<int>& v) {
	std::cout << "[";
	for (size_t i = 0; i < v.size(); ++i) {
		std::cout << v[i] << (i + 1 < v.size() ? "," : "");
	}
	std::cout << "]";
}

//	安全释放链表内存
void freeList(ListNode* head) {
	while (head) {
		ListNode* next = head->next;
		delete head;
		head = next;
	}
}

//	主函数
int main() {
	Solution sol;

	//	测试用例:
	struct TestCase {
		std::vector<int> l1;
		std::vector<int> l2;
		std::vector<int> expected;
	};

	TestCase tests[] = {
		{{2, 4, 3},       {5, 6, 4},       {7, 0, 8}},          // 示例1: 342 + 465 = 807
		{{0},             {0},             {0}},                // 示例2: 0 + 0 = 0
		{{9,9,9,9,9,9,9}, {9,9,9,9},       {8,9,9,9,0,0,0,1}}, // 示例3: 最高位进位
		{{9, 9},          {1},             {0, 0, 1}},          // 补充: 短链表+进位穿透
		{{1, 8},          {0},             {1, 8}}              // 补充: 加零不变
	};

	bool allPassed = true;
	for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
		//	构建原始链表
		ListNode* l1 = buildList(tests[i].l1);
		ListNode* l2 = buildList(tests[i].l2);

		//	执行相加
		ListNode* result = sol.addTwoNumbers(l1, l2);

		//	验证结果
		std::vector<int> actual = listToVector(result);
		bool pass = (actual == tests[i].expected);

		std::cout << "Test" << i + 1 << ":" << (pass ? "PASS" : "FAIL") << "|";
		std::cout << "Expected:"; printVector(tests[i].expected);
		std::cout << ", Got:"; printVector(actual);
		std::cout << std::endl;

		if (!pass) allPassed = false;

		//	释放内存
		freeList(result);
		freeList(l1);
		freeList(l2);
	}

	std::cout << "\n" << (allPassed ? "All tests passed!" : "Some tests failed!") << std::endl;
	return 0;
}
