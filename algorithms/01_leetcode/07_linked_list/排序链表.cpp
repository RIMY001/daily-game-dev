#include <iostream>
#include <vector>
#include <string>

// 链表节点定义
struct ListNode {
    int val;
    ListNode* next;
    ListNode() : val(0), next(nullptr) {}
    explicit ListNode(int x) : val(x), next(nullptr) {}
    ListNode(int x, ListNode* next) : val(x), next(next) {}
};

class Solution {
public:
    ListNode* sortList(ListNode* head) {
        // 基准情况：空链表或只有一个节点
        if (!head || !head->next) {
            return head;
        }

        // 1. 快慢指针找中点，slow 停在左半部分末尾
        ListNode* slow = head;
        ListNode* fast = head->next;   // 快指针先走一步，避免偶数长度时死循环
        while (fast && fast->next) {
            slow = slow->next;
            fast = fast->next->next;
        }

        ListNode* mid = slow->next;    // 右半部分头结点
        slow->next = nullptr;          // 断开链表

        // 2. 递归排序左右两部分
        ListNode* left = sortList(head);
        ListNode* right = sortList(mid);

        // 3. 合并两个有序链表
        return merge(left, right);
    }

private:
    ListNode* merge(ListNode* l1, ListNode* l2) {
        ListNode dummy(0);             // 虚拟头结点，简化边界处理
        ListNode* tail = &dummy;

        while (l1 && l2) {
            if (l1->val < l2->val) {
                tail->next = l1;
                l1 = l1->next;
            }
            else {
                tail->next = l2;
                l2 = l2->next;
            }
            tail = tail->next;
        }

        tail->next = l1 ? l1 : l2;     // 接上剩余部分
        return dummy.next;
    }
};

// ---------- 以下为测试辅助函数 ----------

// 由 vector 构建链表
ListNode* buildList(const std::vector<int>& nums) {
    ListNode dummy(0);
    ListNode* tail = &dummy;
    for (int x : nums) {
        tail->next = new ListNode(x);
        tail = tail->next;
    }
    return dummy.next;
}

// 打印链表
void printList(ListNode* head) {
    std::cout << "[";
    bool first = true;
    for (ListNode* p = head; p; p = p->next) {
        if (!first) std::cout << ",";
        std::cout << p->val;
        first = false;
    }
    std::cout << "]" << std::endl;
}

// 释放链表内存
void freeList(ListNode* head) {
    while (head) {
        ListNode* nxt = head->next;
        delete head;
        head = nxt;
    }
}

int main() {
    Solution solution;

    std::vector<std::vector<int>> tests = {
        { 4, 2, 1, 3 },
        { -1, 5, 3, 4, 0 },
        {},
        { 1 },
        { 5, 5, 3, 1, 2, 5 }
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        ListNode* head = buildList(tests[i]);

        std::cout << "Test Case " << (i + 1) << " 输入: ";
        printList(head);

        head = solution.sortList(head);

        std::cout << "Test Case " << (i + 1) << " 输出: ";
        printList(head);
        std::cout << std::endl;

        freeList(head);   // 避免内存泄漏
    }

    return 0;
}