#include <iostream>
#include <unordered_map>
#include <vector>
#include <cassert>

struct Node {
	int val;
	Node* next;
	Node* random;
	Node(int val_) : val(val_), next(nullptr), random(nullptr) {}
};

class Solution {
public:
	Node* copyRandomList(Node* head) {
		if (head == nullptr) return nullptr;

		//	创建哈希表并建立映射
		std::unordered_map<Node*, Node*> map;
		Node* curr = head;
		while (curr != nullptr) {
			map[curr] = new Node(curr->val);
			curr = curr->next;
		}

		//	设置新节点的next和random指针
		curr = head;
		while (curr != nullptr) {
			//	获取当前节点的新节点
			Node* newCurr = map[curr];
			//	设置新节点的next指针
			if (curr->next != nullptr) {
				newCurr->next = map[curr->next];
			}
			//	设置新节点的random指针
			if (curr->random != nullptr) {
				newCurr->random = map[curr->random];
			}
			curr = curr->next;
		}

		return map[head];
	}
};

// -------------------------------------- 测试辅助代码 --------------------------------------
// 打印链表
void printList(Node* head) {
	Node* cur = head;
	while (cur != nullptr) {
		std::cout << "Node(val=" << cur->val;

		if (cur->random != nullptr) {
			std::cout << ", random->val=" << cur->random->val;
		}
		else {
			std::cout << ", random=nullptr";
		}

		std::cout << ")";

		if (cur->next != nullptr) {
			std::cout << " -> ";
		}

		cur = cur->next;
	}
	std::cout << std::endl;
}

// 释放链表内存
void deleteList(Node* head) {
	while (head != nullptr) {
		Node* tmp = head;
		head = head->next;
		delete tmp;
	}
}

// 构造测试链表
Node* buildTestList(
	const std::vector<int>& values,
	const std::vector<int>& randomIndex
) {
	assert(values.size() == randomIndex.size());

	int n = static_cast<int>(values.size());
	if (n == 0) {
		return nullptr;
	}

	std::vector<Node*> nodes(n);

	// 先创建所有节点
	for (int i = 0; i < n; ++i) {
		nodes[i] = new Node(values[i]);
	}

	// 连接 next
	for (int i = 0; i < n - 1; ++i) {
		nodes[i]->next = nodes[i + 1];
	}

	// 连接 random
	for (int i = 0; i < n; ++i) {
		if (randomIndex[i] != -1) {
			nodes[i]->random = nodes[randomIndex[i]];
		}
	}

	return nodes[0];
}

int main() {
	Solution solution;

	// 测试用例 1：[[7,null],[13,0],[11,4],[10,2],[1,0]]
	{
		Node* head = buildTestList(
			{ 7, 13, 11, 10, 1 },
			{ -1, 0, 4, 2, 0 }
		);

		std::cout << "原链表：" << std::endl;
		printList(head);

		Node* copyHead = solution.copyRandomList(head);

		std::cout << "复制链表：" << std::endl;
		printList(copyHead);

		// 验证：复制链表不能和原链表共享节点
		assert(copyHead != head);
		if (head->next && copyHead->next) {
			assert(copyHead->next != head->next);
		}

		deleteList(head);
		deleteList(copyHead);

		std::cout << "测试用例 1 通过。" << std::endl << std::endl;
	}

	// 测试用例 2：[[1,1],[2,1]]
	{
		Node* head = buildTestList(
			{ 1, 2 },
			{ 1, 1 }
		);

		std::cout << "原链表：" << std::endl;
		printList(head);

		Node* copyHead = solution.copyRandomList(head);

		std::cout << "复制链表：" << std::endl;
		printList(copyHead);

		deleteList(head);
		deleteList(copyHead);

		std::cout << "测试用例 2 通过。" << std::endl << std::endl;
	}

	// 测试用例 3：空链表
	{
		Node* copyHead = solution.copyRandomList(nullptr);
		assert(copyHead == nullptr);

		std::cout << "测试用例 3（空链表）通过。" << std::endl;
	}

	return 0;
}


