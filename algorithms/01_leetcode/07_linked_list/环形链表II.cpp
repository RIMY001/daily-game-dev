#include <iostream>
#include <vector>
#include <unordered_set>

// 链表节点定义（与 LeetCode 一致）
struct ListNode {
    int val;
    ListNode* next;
    ListNode(int x) : val(x), next(nullptr) {}
};

class Solution {
public:
    // Floyd 快慢指针法 - O(1) 空间
    ListNode* detectCycle(ListNode* head) {
        if (!head || !head->next) return nullptr;

        ListNode* slow = head;
        ListNode* fast = head;

        // 第一阶段：检测环
        while (fast && fast->next) {
            slow = slow->next;
            fast = fast->next->next;

            if (slow == fast) {
                // 第二阶段：找入环点
                ListNode* ptr = head;
                while (ptr != slow) {
                    ptr = ptr->next;
                    slow = slow->next;
                }
                return ptr;
            }
        }
        return nullptr;
    }
};

// ==================== 测试辅助函数 ====================

// 根据数组和 pos 构建带环链表
ListNode* buildList(const std::vector<int>& vals, int pos) {
    if (vals.empty()) return nullptr;

    std::vector<ListNode*> nodes;
    for (int v : vals) {
        nodes.push_back(new ListNode(v));
    }
    for (size_t i = 0; i + 1 < nodes.size(); ++i) {
        nodes[i]->next = nodes[i + 1];
    }
    // 如果 pos 有效，尾节点指向 pos 处节点形成环
    if (pos >= 0 && pos < static_cast<int>(nodes.size())) {
        nodes.back()->next = nodes[pos];
    }
    return nodes[0];
}

// 安全释放带环链表（避免内存泄漏）
void freeList(ListNode* head) {
    std::unordered_set<ListNode*> visited;
    ListNode* p = head;
    while (p && visited.insert(p).second) {  // 你之前学的 .second 用法！
        ListNode* next = p->next;
        delete p;
        p = next;
    }
}

// 获取节点在链表中的索引（用于验证结果）
int getNodeIndex(ListNode* head, ListNode* target) {
    if (!target) return -1;
    std::unordered_set<ListNode*> visited;
    int idx = 0;
    while (head && visited.insert(head).second) {
        if (head == target) return idx;
        head = head->next;
        ++idx;
    }
    return -1;
}

// ==================== 主函数 ====================

int main() {
    Solution sol;

    // 测试用例：{值数组, pos, 期望输出索引}
    struct TestCase {
        std::vector<int> vals;
        int pos;
        int expected;
    };

    TestCase tests[] = {
        {{3, 2, 0, -4}, 1, 1},   // 环入口在索引1
        {{1, 2},        0, 0},   // 环入口在索引0
        {{1},          -1, -1},  // 无环
        {{1, 2, 3},    -1, -1},  // 无环
        {{1, 2, 3, 4}, 2, 2},   // 环入口在索引2
    };

    bool allPassed = true;
    for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
        ListNode* head = buildList(tests[i].vals, tests[i].pos);
        ListNode* result = sol.detectCycle(head);
        int actualIdx = getNodeIndex(head, result);

        bool pass = (actualIdx == tests[i].expected);
        std::cout << "Test " << i + 1 << ": "
            << (pass ? "PASS " : "FAIL ")
            << " | Expected: " << tests[i].expected
            << ", Got: " << actualIdx << std::endl;

        if (!pass) allPassed = false;
        freeList(head);  // 安全释放，防止内存泄漏
    }

    std::cout << "\n" << (allPassed ? "All tests passed!" : "Some tests failed!") << std::endl;
    return 0;
}