#include <iostream>
#include <vector>

// 1. 链表节点定义（LeetCode 标准结构）
struct ListNode {
    int val;
    ListNode* next;
    ListNode() : val(0), next(nullptr) {}
    ListNode(int x) : val(x), next(nullptr) {}
    ListNode(int x, ListNode* next) : val(x), next(next) {}
};

// 2. 核心解法：迭代反转链表
class Solution {
public:
    ListNode* reverseList(ListNode* head) {
        ListNode* prev = nullptr;     // 前驱指针初始化为空（新尾节点）
        ListNode* curr = head;        // 当前指针从头开始

        while (curr != nullptr) {
            ListNode* next = curr->next; // 【关键】暂存后继节点，防止断链
            curr->next = prev;           // 翻转当前节点的指向
            prev = curr;                 // prev 前进一步
            curr = next;                 // curr 前进一步（用暂存的值）
        }

        return prev; // prev 即为反转后的新头节点
    }
};

// ==================== 辅助工具函数 ====================

// 从 vector 构建链表
ListNode* buildList(const std::vector<int>& nums) {
    if (nums.empty()) return nullptr;
    ListNode* head = new ListNode(nums[0]);
    ListNode* curr = head;
    for (size_t i = 1; i < nums.size(); ++i) {
        curr->next = new ListNode(nums[i]);
        curr = curr->next;
    }
    return head;
}

// 打印链表
void printList(ListNode* head) {
    std::cout << "[";
    while (head) {
        std::cout << head->val;
        if (head->next) std::cout << ",";
        head = head->next;
    }
    std::cout << "]" << std::endl;
}

// 释放链表内存
void freeList(ListNode* head) {
    while (head) {
        ListNode* temp = head;
        head = head->next;
        delete temp;
    }
}

// ==================== 主函数：测试验证 ====================
int main() {
    Solution sol;

    // 测试用例 1: [1,2,3,4,5] -> [5,4,3,2,1]
    ListNode* head1 = buildList({ 1, 2, 3, 4, 5 });
    std::cout << "输入: "; printList(head1);
    ListNode* result1 = sol.reverseList(head1);
    std::cout << "输出: "; printList(result1);
    freeList(result1);
    std::cout << std::endl;

    // 测试用例 2: [1,2] -> [2,1]
    ListNode* head2 = buildList({ 1, 2 });
    std::cout << "输入: "; printList(head2);
    ListNode* result2 = sol.reverseList(head2);
    std::cout << "输出: "; printList(result2);
    freeList(result2);
    std::cout << std::endl;

    // 测试用例 3: [] -> []
    ListNode* head3 = buildList({});
    std::cout << "输入: "; printList(head3);
    ListNode* result3 = sol.reverseList(head3);
    std::cout << "输出: "; printList(result3);
    // head3 为 nullptr，无需释放

    return 0;
}