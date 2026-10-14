#include <iostream>
#include <vector>
#include <unordered_set>
using namespace std;

struct ListNode {
	int val;
	ListNode* next;
	ListNode(int x) : val(x), next(nullptr) {}
};

class Solution {
public:
	bool isPalindrome(ListNode* head) {
		if (!head || !head->next) return true;	//	天然是回文

		//	快慢指针找中点
		ListNode* slow = head;
		ListNode* fast = head;
		while (fast->next && fast->next->next) {
			slow = slow->next;
			fast = fast->next->next;
		}

		//	反转后半段链表
		ListNode* prev = nullptr;
		ListNode* curr = slow->next;
		while (curr) {
			ListNode* next = curr->next;
			curr->next = prev;
			prev = curr;
			curr = next;
		}

		//	双指针比较
		ListNode* p1 = head;
		ListNode* p2 = prev;
		bool result = true;
		while (p2) {
			if (p1->val != p2->val) {
				result = false;
				break;
			}
			p1 = p1->next;
			p2 = p2->next;
		}

		//	恢复链表
		curr = prev;
		prev = nullptr;
		while (curr) {
			ListNode* next = curr->next;
			curr->next = prev;
			prev = curr;
			curr = next;
		}
		slow->next = prev;

		return result;
	}
};

//	========================= 测试辅助函数 =========================
//	构建链表
ListNode* createList(const vector<int>& nums) {
	if (nums.empty()) return nullptr;
	ListNode* head = new ListNode(nums[0]);
	ListNode* curr = head;
	for (size_t i = 1; i < nums.size(); ++i) {
		curr->next = new ListNode(nums[i]);
		curr = curr->next;
	}
	//  防御性验证：确认链表完整性
	if (!head || !head->next && nums.size() > 1) {
		cerr << "[FATAL] createList 返回了非法链表!" << endl;
		abort();
	}
	return head;
}

//	打印链表
void printList(ListNode* head, const string& label) {
	cout << label << ": ";
	for (ListNode* p = head; p; p = p->next) {
		cout << p->val;
		if (p->next) cout << " -> ";
	}
	cout << endl;
}


// 安全释放链表
void freeList(ListNode* head) {
    unordered_set<ListNode*> visited;
    ListNode* p = head;
    while (p && visited.insert(p).second) {
        ListNode* next = p->next; // 先保存下一节点
        delete p;                // 再释放当前节点
        p = next;                // 使用保存的指针继续
    }
}	

int main() {
	Solution sol;

	//	测试用例设计
	struct TestCase {
		vector<int> data;
		bool expected;
		string desc;
	};

	vector<TestCase> tests = {
		 {{1, 2, 3, 2, 1},       true,  "奇数长度回文"},
		{{1, 2, 2, 1},          true,  "偶数长度回文"},
		{{1, 2, 3, 4, 5},       false, "非回文"},
		{{1},                   true,  "单节点"},
		{{},                    true,  "空链表"},
		{{1, 2},                false, "两节点非回文"},
		{{1, 1},                true,  "两节点回文"},
	};

	
	for (auto& t : tests) {
		std::cout << "=== " << t.desc << " ===" << std::endl;
		ListNode* head = createList(t.data);

		// 验证创建
		printList(head, "创建后 ");

		// 只调用一次，只定义一次 res
		bool res = sol.isPalindrome(head);

		// 验证恢复
		printList(head, "恢复后 ");

		std::cout << "结果: " << (res == t.expected ? "PASS" : "FAIL")
			<< " (期望:" << t.expected << " 实际:" << res << ")" << std::endl;

		freeList(head);
		std::cout << std::endl;
		}
	

	std::cout << "按回车键退出...";
	std::cin.get();
	return 0;
}