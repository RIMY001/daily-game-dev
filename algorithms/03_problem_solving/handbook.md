# 数据结构与算法高频技巧汇总手册

> **面向**：游戏客户端开发 + 力扣面试
> **标签说明**：
> - 【力扣高频】= 面试刷题必须掌握
> - 【工程实践】= 游戏客户端开发高频使用
> - 双标签 = 两者都适用

---

# 一、链表技巧

---

## 1.1 虚拟头节点（Dummy Node）【力扣高频】

### ① 使用目的 & 适用场景

**解决问题**：链表头部可能被修改时（删除、插入），避免对头节点的特殊处理。

**游戏场景**：
- 对象池 free-list 管理（头节点可能被回收）
- 事件监听器链表（需要频繁增删）

### ② 写法 & 使用步骤

```cpp
// 删除链表中所有值为 val 的节点
ListNode* removeElements(ListNode* head, int val) {
    ListNode dummy(0);      // 虚拟头节点
    dummy.next = head;
    
    ListNode* cur = &dummy;
    while (cur->next) {
        if (cur->next->val == val) {
            ListNode* toDelete = cur->next;
            cur->next = toDelete->next;
            delete toDelete;
        } else {
            cur = cur->next;
        }
    }
    return dummy.next;
}
```

**STL 对应**：`std::list` 内部使用哨兵节点（类似 dummy）。

### ③ 底层原理

- **时间复杂度**：O(n)
- **空间复杂度**：O(1)
- **优点**：统一头节点和中间节点的处理逻辑
- **缺点**：多一个临时节点的空间（可忽略）

**常见坑**：
- 忘记返回 `dummy.next` 而是返回 `head`
- 虚拟节点的生命周期（栈上 vs 堆上）

### 【选型提醒】

**不适合**：
- 不需要修改头节点的操作（遍历、查找）
- 数组结构（用索引即可）

---

## 1.2 快慢指针（Floyd 判圈）【力扣高频】【工程实践】

### ① 使用目的 & 适用场景

**解决问题**：
1. 检测链表是否有环
2. 找到环的入口
3. 找到链表中点

**游戏场景**：
- 循环动画检测（动画状态机是否有死循环）
- 任务依赖链检测（是否有循环依赖）

### ② 写法 & 使用步骤

```cpp
// 检测环并找到入口
ListNode* detectCycle(ListNode* head) {
    ListNode *slow = head, *fast = head;
    
    // 第一阶段：检测是否有环
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
        if (slow == fast) {  // 相遇，有环
            // 第二阶段：找入口
            ListNode* ptr = head;
            while (ptr != slow) {
                ptr = ptr->next;
                slow = slow->next;
            }
            return ptr;  // 环入口
        }
    }
    return nullptr;  // 无环
}

// 找链表中点
ListNode* middleNode(ListNode* head) {
    ListNode *slow = head, *fast = head;
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    return slow;
}
```

### ③ 底层原理

- **时间复杂度**：O(n)
- **空间复杂度**：O(1)
- **原理**：快指针速度是慢指针 2 倍，如果有环必然相遇
- **数学证明**：设环长为 C，入口距离为 a，相遇点距离入口 b，则 2(a+b) = a + b + kC → a = kC - b

**常见坑**：
- 快指针判断条件用 `fast && fast->next`（不是 `fast->next && fast->next->next`）
- 找入口时从 head 和相遇点同时出发

### 【选型提醒】

**不适合**：
- 需要修改链表结构的操作
- 数组（直接用索引计算中点）

---

## 1.3 反转链表（递归 & 迭代）【力扣高频】

### ① 使用目的 & 适用场景

**解决问题**：反转链表方向。

**游戏场景**：
- 回放系统（反向播放操作序列）
- 撤销系统（反向遍历操作栈）

### ② 写法 & 使用步骤

```cpp
// 迭代法（推荐）
ListNode* reverseList(ListNode* head) {
    ListNode *prev = nullptr, *cur = head;
    while (cur) {
        ListNode* next = cur->next;  // 保存下一个
        cur->next = prev;            // 反转指针
        prev = cur;                  // 前进
        cur = next;
    }
    return prev;
}

// 递归法
ListNode* reverseList(ListNode* head) {
    if (!head || !head->next) return head;
    ListNode* newHead = reverseList(head->next);
    head->next->next = head;
    head->next = nullptr;
    return newHead;
}

// 反转部分链表（区间 [left, right]）
ListNode* reverseBetween(ListNode* head, int left, int right) {
    ListNode dummy(0);
    dummy.next = head;
    ListNode* prev = &dummy;
    
    for (int i = 0; i < left - 1; ++i)
        prev = prev->next;
    
    ListNode* cur = prev->next;
    for (int i = 0; i < right - left; ++i) {
        ListNode* next = cur->next;
        cur->next = next->next;
        next->next = prev->next;
        prev->next = next;
    }
    return dummy.next;
}
```

### ③ 底层原理

- **迭代法**：时间 O(n)，空间 O(1)
- **递归法**：时间 O(n)，空间 O(n)（调用栈）
- **优点**：迭代法空间更优
- **缺点**：递归法可能栈溢出

**常见坑**：
- 忘记保存 `next` 节点
- 反转后头尾节点的处理

### 【选型提醒】

**不适合**：
- 双向链表（需要同时修改 prev 和 next）
- 需要保持原始顺序的场景

---

## 1.4 合并 K 个有序链表【力扣高频】【工程实践】

### ① 使用目的 & 适用场景

**解决问题**：合并多个有序链表为一个有序链表。

**游戏场景**：
- 多路归并排序（外部排序）
- 合并多个渲染队列（按 depth 排序）
- 合并多个时间轴动画

### ② 写法 & 使用步骤

```cpp
// 方法一：优先队列（最小堆）
struct Compare {
    bool operator()(ListNode* a, ListNode* b) {
        return a->val > b->val;
    }
};

ListNode* mergeKLists(vector<ListNode*>& lists) {
    priority_queue<ListNode*, vector<ListNode*>, Compare> pq;
    
    for (auto list : lists) {
        if (list) pq.push(list);
    }
    
    ListNode dummy(0);
    ListNode* tail = &dummy;
    
    while (!pq.empty()) {
        ListNode* node = pq.top();
        pq.pop();
        tail->next = node;
        tail = tail->next;
        
        if (node->next) pq.push(node->next);
    }
    return dummy.next;
}

// 方法二：分治合并（类似归并排序）
ListNode* mergeTwoLists(ListNode* l1, ListNode* l2) {
    if (!l1) return l2;
    if (!l2) return l1;
    if (l1->val < l2->val) {
        l1->next = mergeTwoLists(l1->next, l2);
        return l1;
    } else {
        l2->next = mergeTwoLists(l1, l2->next);
        return l2;
    }
}

ListNode* mergeKLists(vector<ListNode*>& lists) {
    if (lists.empty()) return nullptr;
    return mergeRange(lists, 0, lists.size() - 1);
}

ListNode* mergeRange(vector<ListNode*>& lists, int left, int right) {
    if (left == right) return lists[left];
    int mid = left + (right - left) / 2;
    auto l1 = mergeRange(lists, left, mid);
    auto l2 = mergeRange(lists, mid + 1, right);
    return mergeTwoLists(l1, l2);
}
```

**STL 对应**：`std::priority_queue`（最小堆）。

### ③ 底层原理

- **优先队列法**：时间 O(N log k)，空间 O(k)，N 为总节点数，k 为链表数
- **分治法**：时间 O(N log k)，空间 O(log k)（递归栈）
- **优点**：优先队列法空间更优
- **缺点**：需要额外的堆空间

**常见坑**：
- 优先队列的比较函数方向（`>` 是最小堆）
- 空链表的处理

### 【选型提醒】

**不适合**：
- 链表数量很少（< 3，直接两两合并）
- 不需要全局有序（分别处理即可）

---

# 二、数组 & 双指针技巧

---

## 2.1 双指针（对撞 & 快慢）【力扣高频】【工程实践】

### ① 使用目的 & 适用场景

**解决问题**：
1. **对撞指针**：有序数组两数之和、容器盛水
2. **快慢指针**：移除元素、原地修改数组

**游戏场景**：
- 装备属性配对查找（两数之和）
- 帧数据压缩（移除冗余帧）

### ② 写法 & 使用步骤

```cpp
// 对撞指针：有序数组两数之和
vector<int> twoSum(vector<int>& nums, int target) {
    int left = 0, right = nums.size() - 1;
    while (left < right) {
        int sum = nums[left] + nums[right];
        if (sum == target) return {left, right};
        else if (sum < target) left++;
        else right--;
    }
    return {};
}

// 快慢指针：移除指定元素（原地）
int removeElement(vector<int>& nums, int val) {
    int slow = 0;
    for (int fast = 0; fast < nums.size(); ++fast) {
        if (nums[fast] != val) {
            nums[slow++] = nums[fast];
        }
    }
    return slow;
}

// 快慢指针：移除重复元素（保持一个）
int removeDuplicates(vector<int>& nums) {
    if (nums.empty()) return 0;
    int slow = 0;
    for (int fast = 1; fast < nums.size(); ++fast) {
        if (nums[fast] != nums[slow]) {
            nums[++slow] = nums[fast];
        }
    }
    return slow + 1;
}
```

### ③ 底层原理

- **时间复杂度**：O(n)
- **空间复杂度**：O(1)
- **优点**：原地操作，无需额外空间
- **缺点**：只适用于特定问题

**常见坑**：
- 对撞指针要求数组有序
- 快慢指针的边界条件（空数组、单元素）

### 【选型提醒】

**不适合**：
- 无序数组的两数之和（用哈希表）
- 需要保留原始顺序的原地修改

---

## 2.2 滑动窗口【力扣高频】【工程实践】

### ① 使用目的 & 适用场景

**解决问题**：找到满足条件的连续子数组/子串。

**游戏场景**：
- 帧率统计（最近 N 帧平均帧时间）
- 网络延迟平滑（滑动窗口平均）
- 连续伤害检测（3 秒内伤害超过阈值）

### ② 写法 & 使用步骤

```cpp
// 最小覆盖子串（hard）
string minWindow(string s, string t) {
    unordered_map<char, int> need, window;
    for (char c : t) need[c]++;
    
    int left = 0, right = 0;
    int valid = 0;  // 满足条件的字符数
    int start = 0, minLen = INT_MAX;
    
    while (right < s.size()) {
        char c = s[right];
        right++;
        
        if (need.count(c)) {
            window[c]++;
            if (window[c] == need[c]) valid++;
        }
        
        while (valid == need.size()) {
            if (right - left < minLen) {
                start = left;
                minLen = right - left;
            }
            
            char d = s[left];
            left++;
            
            if (need.count(d)) {
                if (window[d] == need[d]) valid--;
                window[d]--;
            }
        }
    }
    return minLen == INT_MAX ? "" : s.substr(start, minLen);
}

// 长度最小的子数组（和 ≥ target）
int minSubArrayLen(int target, vector<int>& nums) {
    int left = 0, sum = 0;
    int minLen = INT_MAX;
    
    for (int right = 0; right < nums.size(); ++right) {
        sum += nums[right];
        
        while (sum >= target) {
            minLen = min(minLen, right - left + 1);
            sum -= nums[left++];
        }
    }
    return minLen == INT_MAX ? 0 : minLen;
}
```

### ③ 底层原理

- **时间复杂度**：O(n)（每个元素最多访问 2 次）
- **空间复杂度**：O(k)（k 为字符集大小，或 O(1)）
- **优点**：将 O(n²) 暴力枚举优化为 O(n)
- **缺点**：只适用于连续子数组/子串

**常见坑**：
- 窗口收缩条件（while vs if）
- 窗口状态的维护（增加/移除元素时更新）

### 【选型提醒】

**不适合**：
- 非连续子序列
- 需要全局信息的问题

---

## 2.3 前缀和 & 差分数组【力扣高频】【工程实践】

### ① 使用目的 & 适用场景

**解决问题**：
1. **前缀和**：快速查询区间和
2. **差分**：区间批量修改

**游戏场景**：
- 伤害统计（查询某段时间内的总伤害）
- 地图区域修改（批量增加/减少属性）

### ② 写法 & 使用步骤

```cpp
// 前缀和
class NumArray {
    vector<int> prefix;
public:
    NumArray(vector<int>& nums) {
        prefix.resize(nums.size() + 1);
        for (int i = 0; i < nums.size(); ++i) {
            prefix[i + 1] = prefix[i] + nums[i];
        }
    }
    
    int sumRange(int left, int right) {
        return prefix[right + 1] - prefix[left];
    }
};

// 差分数组（区间批量修改）
class DifferenceArray {
    vector<int> diff;
public:
    DifferenceArray(int n) {
        diff.resize(n);
    }
    
    void increment(int i, int j, int val) {
        diff[i] += val;
        if (j + 1 < diff.size()) diff[j + 1] -= val;
    }
    
    vector<int> getResult() {
        vector<int> res(diff.size());
        res[0] = diff[0];
        for (int i = 1; i < diff.size(); ++i) {
            res[i] = res[i - 1] + diff[i];
        }
        return res;
    }
};

// 应用：拼车问题
bool carPooling(vector<vector<int>>& trips, int capacity) {
    DifferenceArray da(1001);
    for (auto& trip : trips) {
        da.increment(trip[1], trip[2] - 1, trip[0]);
    }
    auto res = da.getResult();
    for (int num : res) {
        if (num > capacity) return false;
    }
    return true;
}
```

### ③ 底层原理

- **前缀和**：预处理 O(n)，查询 O(1)
- **差分**：修改 O(1)，还原 O(n)
- **优点**：将区间操作优化为 O(1)
- **缺点**：需要额外空间

**常见坑**：
- 前缀和数组大小为 n+1（方便计算）
- 差分的边界处理（j+1 是否越界）

### 【选型提醒】

**不适合**：
- 单点修改频繁（用线段树）
- 非区间操作

---

# 三、树技巧

---

## 3.1 二叉树遍历（递归 & 迭代）【力扣高频】【工程实践】

### ① 使用目的 & 适用场景

**解决问题**：前序/中序/后序/层序遍历二叉树。

**游戏场景**：
- 场景图遍历（渲染场景树）
- UI 树遍历（渲染 UI 层级）
- 骨骼动画遍历（更新骨骼层级）

### ② 写法 & 使用步骤

```cpp
struct TreeNode {
    int val;
    TreeNode* left;
    TreeNode* right;
};

// 前序遍历（递归）
void preorder(TreeNode* root, vector<int>& res) {
    if (!root) return;
    res.push_back(root->val);
    preorder(root->left, res);
    preorder(root->right, res);
}

// 前序遍历（迭代）
vector<int> preorderTraversal(TreeNode* root) {
    vector<int> res;
    stack<TreeNode*> stk;
    TreeNode* cur = root;
    
    while (cur || !stk.empty()) {
        while (cur) {
            res.push_back(cur->val);
            stk.push(cur);
            cur = cur->left;
        }
        cur = stk.top();
        stk.pop();
        cur = cur->right;
    }
    return res;
}

// 层序遍历（BFS）
vector<vector<int>> levelOrder(TreeNode* root) {
    vector<vector<int>> res;
    if (!root) return res;
    
    queue<TreeNode*> q;
    q.push(root);
    
    while (!q.empty()) {
        int size = q.size();
        vector<int> level;
        
        for (int i = 0; i < size; ++i) {
            TreeNode* node = q.front();
            q.pop();
            level.push_back(node->val);
            
            if (node->left) q.push(node->left);
            if (node->right) q.push(node->right);
        }
        res.push_back(level);
    }
    return res;
}
```

**STL 对应**：`std::stack`（迭代遍历）、`std::queue`（层序遍历）。

### ③ 底层原理

- **递归法**：时间 O(n)，空间 O(h)（h 为树高）
- **迭代法**：时间 O(n)，空间 O(h)
- **层序遍历**：时间 O(n)，空间 O(n)（最宽层）
- **优点**：递归法简洁，迭代法避免栈溢出
- **缺点**：递归可能栈溢出（深树）

**常见坑**：
- 迭代法的栈操作顺序
- 层序遍历的层数控制（`q.size()` 在循环中变化）

### 【选型提醒】

**不适合**：
- 需要频繁修改树结构（用平衡树）
- 树非常深（用迭代法或 Morris 遍历）

---

## 3.2 二叉搜索树（BST）操作【力扣高频】

### ① 使用目的 & 适用场景

**解决问题**：BST 的查找、插入、删除、验证。

**游戏场景**：
- 排行榜系统（有序数据）
- 时间轴管理（按时间排序的事件）

### ② 写法 & 使用步骤

```cpp
// BST 查找
TreeNode* searchBST(TreeNode* root, int val) {
    if (!root) return nullptr;
    if (root->val == val) return root;
    if (val < root->val) return searchBST(root->left, val);
    return searchBST(root->right, val);
}

// BST 插入
TreeNode* insertIntoBST(TreeNode* root, int val) {
    if (!root) return new TreeNode(val);
    if (val < root->val) root->left = insertIntoBST(root->left, val);
    else root->right = insertIntoBST(root->right, val);
    return root;
}

// BST 删除（三种情况）
TreeNode* deleteNode(TreeNode* root, int key) {
    if (!root) return nullptr;
    
    if (key < root->val) {
        root->left = deleteNode(root->left, key);
    } else if (key > root->val) {
        root->right = deleteNode(root->right, key);
    } else {
        // 情况 1 & 2：无子节点或只有一个子节点
        if (!root->left) return root->right;
        if (!root->right) return root->left;
        
        // 情况 3：有两个子节点，找右子树最小值
        TreeNode* minNode = getMin(root->right);
        root->val = minNode->val;
        root->right = deleteNode(root->right, minNode->val);
    }
    return root;
}

TreeNode* getMin(TreeNode* node) {
    while (node->left) node = node->left;
    return node;
}

// 验证 BST（中序遍历应为递增序列）
bool isValidBST(TreeNode* root) {
    stack<TreeNode*> stk;
    TreeNode* prev = nullptr;
    TreeNode* cur = root;
    
    while (cur || !stk.empty()) {
        while (cur) {
            stk.push(cur);
            cur = cur->left;
        }
        cur = stk.top();
        stk.pop();
        
        if (prev && prev->val >= cur->val) return false;
        prev = cur;
        cur = cur->right;
    }
    return true;
}
```

### ③ 底层原理

- **查找/插入/删除**：平均 O(log n)，最坏 O(n)（退化为链表）
- **验证**：O(n)
- **优点**：有序数据结构，查找高效
- **缺点**：可能退化（用 AVL/红黑树）

**常见坑**：
- 删除有两个子节点的节点（用后继节点替换）
- 验证 BST 时注意 `prev` 的更新

### 【选型提醒】

**不适合**：
- 需要严格 O(log n)（用平衡树）
- 不需要有序（用哈希表）

---

## 3.3 最近公共祖先（LCA）【力扣高频】

### ① 使用目的 & 适用场景

**解决问题**：找到二叉树中两个节点的最近公共祖先。

**游戏场景**：
- 骨骼层级查询（两个骨骼的公共父骨骼）
- UI 层级查询（两个 UI 元素的公共父节点）

### ② 写法 & 使用步骤

```cpp
// 二叉树的 LCA
TreeNode* lowestCommonAncestor(TreeNode* root, TreeNode* p, TreeNode* q) {
    if (!root || root == p || root == q) return root;
    
    auto left = lowestCommonAncestor(root->left, p, q);
    auto right = lowestCommonAncestor(root->right, p, q);
    
    if (left && right) return root;  // p 和 q 在两侧
    return left ? left : right;      // 都在同一侧
}

// BST 的 LCA（利用 BST 性质）
TreeNode* lowestCommonAncestorBST(TreeNode* root, TreeNode* p, TreeNode* q) {
    while (root) {
        if (p->val < root->val && q->val < root->val) {
            root = root->left;
        } else if (p->val > root->val && q->val > root->val) {
            root = root->right;
        } else {
            return root;
        }
    }
    return nullptr;
}
```

### ③ 底层原理

- **二叉树**：时间 O(n)，空间 O(h)
- **BST**：时间 O(h)，空间 O(1)（迭代）
- **优点**：BST 利用性质更高效
- **缺点**：普通树需要遍历

**常见坑**：
- 递归返回条件（`!root || root == p || root == q`）
- BST 的比较逻辑

### 【选型提醒】

**不适合**：
- 需要频繁查询（预处理 + RMQ）
- 树结构频繁变化

---

# 四、图搜索技巧

---

## 4.1 BFS（广度优先搜索）【力扣高频】【工程实践】

### ① 使用目的 & 适用场景

**解决问题**：无权图最短路径、层序遍历、连通性检测。

**游戏场景**：
- 网格地图最短路径（战棋、塔防）
- 洪水填充（战争迷雾）
- 社交网络 N 度关系

### ② 写法 & 使用步骤

```cpp
// 网格 BFS（最短路径）
int bfs(vector<vector<int>>& grid, pair<int,int> start, pair<int,int> end) {
    int rows = grid.size(), cols = grid[0].size();
    vector<vector<bool>> visited(rows, vector<bool>(cols, false));
    queue<pair<int,int>> q;
    
    q.push(start);
    visited[start.first][start.second] = true;
    
    int dist = 0;
    int dx[] = {-1, 1, 0, 0};
    int dy[] = {0, 0, -1, 1};
    
    while (!q.empty()) {
        int size = q.size();
        for (int i = 0; i < size; ++i) {
            auto [x, y] = q.front();
            q.pop();
            
            if (x == end.first && y == end.second) return dist;
            
            for (int d = 0; d < 4; ++d) {
                int nx = x + dx[d], ny = y + dy[d];
                if (nx >= 0 && nx < rows && ny >= 0 && ny < cols &&
                    !visited[nx][ny] && grid[nx][ny] == 0) {
                    visited[nx][ny] = true;
                    q.push({nx, ny});
                }
            }
        }
        dist++;
    }
    return -1;  // 不可达
}

// 层序遍历（树的 BFS）
vector<vector<int>> levelOrder(TreeNode* root) {
    vector<vector<int>> res;
    if (!root) return res;
    
    queue<TreeNode*> q;
    q.push(root);
    
    while (!q.empty()) {
        int size = q.size();
        vector<int> level;
        
        for (int i = 0; i < size; ++i) {
            TreeNode* node = q.front();
            q.pop();
            level.push_back(node->val);
            
            if (node->left) q.push(node->left);
            if (node->right) q.push(node->right);
        }
        res.push_back(level);
    }
    return res;
}
```

**STL 对应**：`std::queue`。

### ③ 底层原理

- **时间复杂度**：O(V + E)
- **空间复杂度**：O(V)（队列 + visited）
- **优点**：保证最短路径（无权图）
- **缺点**：空间开销大

**常见坑**：
- 队列大小在循环中变化（用 `q.size()` 控制层数）
- visited 数组的初始化

### 【选型提醒】

**不适合**：
- 有权图最短路径（用 Dijkstra/A*）
- 需要找所有路径（用 DFS）

---

## 4.2 DFS（深度优先搜索）【力扣高频】【工程实践】

### ① 使用目的 & 适用场景

**解决问题**：遍历图/树、连通性检测、拓扑排序、路径搜索。

**游戏场景**：
- 迷宫生成（DFS 回溯）
- 任务依赖拓扑排序
- 场景图遍历

### ② 写法 & 使用步骤

```cpp
// DFS 递归
void dfs(int node, vector<vector<int>>& adj, vector<bool>& visited) {
    visited[node] = true;
    // process(node);
    
    for (int neighbor : adj[node]) {
        if (!visited[neighbor]) {
            dfs(neighbor, adj, visited);
        }
    }
}

// DFS 迭代（显式栈）
void dfsIterative(int start, vector<vector<int>>& adj) {
    stack<int> stk;
    vector<bool> visited(adj.size(), false);
    
    stk.push(start);
    
    while (!stk.empty()) {
        int node = stk.top();
        stk.pop();
        
        if (visited[node]) continue;
        visited[node] = true;
        // process(node);
        
        for (int neighbor : adj[node]) {
            if (!visited[neighbor]) {
                stk.push(neighbor);
            }
        }
    }
}

// 拓扑排序（DFS）
vector<int> topoSort(int numCourses, vector<vector<int>>& prerequisites) {
    vector<vector<int>> adj(numCourses);
    for (auto& pre : prerequisites) {
        adj[pre[1]].push_back(pre[0]);
    }
    
    vector<int> res;
    vector<int> state(numCourses, 0);  // 0=未访问, 1=访问中, 2=已访问
    
    for (int i = 0; i < numCourses; ++i) {
        if (state[i] == 0 && !dfs(i, adj, state, res)) {
            return {};  // 有环
        }
    }
    return res;
}

bool dfs(int node, vector<vector<int>>& adj, vector<int>& state, vector<int>& res) {
    state[node] = 1;
    
    for (int neighbor : adj[node]) {
        if (state[neighbor] == 1) return false;  // 环
        if (state[neighbor] == 0 && !dfs(neighbor, adj, state, res)) {
            return false;
        }
    }
    
    state[node] = 2;
    res.push_back(node);
    return true;
}
```

**STL 对应**：`std::stack`（迭代 DFS）。

### ③ 底层原理

- **时间复杂度**：O(V + E)
- **空间复杂度**：O(V)（栈 + visited）
- **优点**：空间比 BFS 小（最坏 O(V) vs O(V)）
- **缺点**：不保证最短路径

**常见坑**：
- 递归深度（可能栈溢出）
- 环检测（用 state 数组）

### 【选型提醒】

**不适合**：
- 最短路径（用 BFS）
- 层序遍历（用 BFS）

---

## 4.3 Dijkstra & A* 【工程实践】

### ① 使用目的 & 适用场景

**解决问题**：
- **Dijkstra**：单源最短路径（有权图）
- **A\***：单源单目标最短路径（启发式搜索）

**游戏场景**：
- AI 寻路（A* 是核心算法）
- 技能传导（Dijkstra）
- 可达性分析

### ② 写法 & 使用步骤

```cpp
// Dijkstra（优先队列）
vector<int> dijkstra(int n, vector<vector<pair<int,int>>>& adj, int start) {
    vector<int> dist(n, INT_MAX);
    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<>> pq;
    
    dist[start] = 0;
    pq.push({0, start});
    
    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        
        if (d > dist[u]) continue;  // 懒惰删除
        
        for (auto [v, weight] : adj[u]) {
            if (dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                pq.push({dist[v], v});
            }
        }
    }
    return dist;
}

// A* 寻路（网格）
struct Node {
    int x, y;
    float g, h;
    float f() const { return g + h; }
};

vector<pair<int,int>> aStar(vector<vector<int>>& grid, 
                            pair<int,int> start, pair<int,int> end) {
    int rows = grid.size(), cols = grid[0].size();
    
    auto heuristic = [&](int x, int y) {
        return abs(x - end.first) + abs(y - end.second);
    };
    
    priority_queue<Node, vector<Node>, function<bool(Node,Node)>> pq(
        [](const Node& a, const Node& b) { return a.f() > b.f(); }
    );
    
    vector<vector<float>> gScore(rows, vector<float>(cols, INFINITY));
    vector<vector<pair<int,int>>> parent(rows, vector<pair<int,int>>(cols, {-1,-1}));
    
    gScore[start.first][start.second] = 0;
    pq.push({start.first, start.second, 0, heuristic(start.first, start.second)});
    
    int dx[] = {-1, 1, 0, 0};
    int dy[] = {0, 0, -1, 1};
    
    while (!pq.empty()) {
        auto [x, y, g, h] = pq.top();
        pq.pop();
        
        if (x == end.first && y == end.second) {
            // 回溯路径
            vector<pair<int,int>> path;
            for (auto p = end; p != start; p = parent[p.first][p.second]) {
                path.push_back(p);
            }
            path.push_back(start);
            reverse(path.begin(), path.end());
            return path;
        }
        
        for (int d = 0; d < 4; ++d) {
            int nx = x + dx[d], ny = y + dy[d];
            if (nx >= 0 && nx < rows && ny >= 0 && ny < cols && grid[nx][ny] == 0) {
                float newG = g + 1;
                if (newG < gScore[nx][ny]) {
                    gScore[nx][ny] = newG;
                    parent[nx][ny] = {x, y};
                    pq.push({nx, ny, newG, heuristic(nx, ny)});
                }
            }
        }
    }
    return {};
}
```

**STL 对应**：`std::priority_queue`。

### ③ 底层原理

- **Dijkstra**：时间 O((V+E) log V)，空间 O(V)
- **A\***：时间 O(E log V)（取决于启发函数），空间 O(V)
- **优点**：A* 比 Dijkstra 更快（有启发引导）
- **缺点**：A* 需要好的启发函数

**常见坑**：
- A* 的启发函数必须可接受（admissible）
- 优先队列的懒惰删除

### 【选型提醒】

**不适合**：
- 无权图（用 BFS）
- 有负权边（用 Bellman-Ford）
- 多源多目标（用 Floyd-Warshall）

---

# 五、排序技巧

---

## 5.1 快速排序【力扣高频】【工程实践】

### ① 使用目的 & 适用场景

**解决问题**：通用排序，平均性能最优。

**游戏场景**：
- 渲染队列排序（按 depth）
- 实体排序（按距离）

### ② 写法 & 使用步骤

```cpp
// 三路快排（处理大量重复元素）
void quickSort(vector<int>& arr, int left, int right) {
    if (left >= right) return;
    
    int pivot = arr[left + (right - left) / 2];
    int lt = left, gt = right, i = left;
    
    while (i <= gt) {
        if (arr[i] < pivot) {
            swap(arr[lt++], arr[i++]);
        } else if (arr[i] > pivot) {
            swap(arr[i], arr[gt--]);
        } else {
            i++;
        }
    }
    
    quickSort(arr, left, lt - 1);
    quickSort(arr, gt + 1, right);
}

// 随机化快排（避免最坏情况）
int partition(vector<int>& arr, int left, int right) {
    int pivotIdx = left + rand() % (right - left + 1);
    swap(arr[pivotIdx], arr[right]);
    
    int pivot = arr[right];
    int i = left;
    
    for (int j = left; j < right; ++j) {
        if (arr[j] < pivot) {
            swap(arr[i++], arr[j]);
        }
    }
    swap(arr[i], arr[right]);
    return i;
}

void quickSortRandom(vector<int>& arr, int left, int right) {
    if (left < right) {
        int p = partition(arr, left, right);
        quickSortRandom(arr, left, p - 1);
        quickSortRandom(arr, p + 1, right);
    }
}
```

**STL 对应**：`std::sort`（Introsort = 快排 + 堆排 + 插入排序）。

### ③ 底层原理

- **时间复杂度**：平均 O(n log n)，最坏 O(n²)
- **空间复杂度**：O(log n)（递归栈）
- **优点**：缓存友好，平均性能最优
- **缺点**：不稳定，最坏情况差

**常见坑**：
- pivot 选择（三数取中或随机）
- 大量重复元素（用三路划分）

### 【选型提醒】

**不适合**：
- 需要稳定排序（用归并）
- 小数据量（< 16，用插入排序）

---

## 5.2 归并排序【力扣高频】

### ① 使用目的 & 适用场景

**解决问题**：稳定排序，适合外部排序。

**游戏场景**：
- 渲染队列稳定排序（同 depth 保持顺序）

### ② 写法 & 使用步骤

```cpp
void merge(vector<int>& arr, vector<int>& temp, int left, int mid, int right) {
    int i = left, j = mid + 1, k = left;
    
    while (i <= mid && j <= right) {
        if (arr[i] <= arr[j]) temp[k++] = arr[i++];
        else temp[k++] = arr[j++];
    }
    
    while (i <= mid) temp[k++] = arr[i++];
    while (j <= right) temp[k++] = arr[j++];
    
    for (int p = left; p <= right; ++p) arr[p] = temp[p];
}

void mergeSort(vector<int>& arr, vector<int>& temp, int left, int right) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    mergeSort(arr, temp, left, mid);
    mergeSort(arr, temp, mid + 1, right);
    merge(arr, temp, left, mid, right);
}

void mergeSort(vector<int>& arr) {
    vector<int> temp(arr.size());
    mergeSort(arr, temp, 0, arr.size() - 1);
}
```

**STL 对应**：`std::stable_sort`。

### ③ 底层原理

- **时间复杂度**：O(n log n)（稳定）
- **空间复杂度**：O(n)
- **优点**：稳定排序
- **缺点**：需要额外空间

**常见坑**：
- 临时数组的分配（可以预分配）
- 合并时的边界条件

### 【选型提醒】

**不适合**：
- 内存敏感场景
- 不需要稳定排序（快排更快）

---

# 六、哈希技巧

---

## 6.1 哈希表（unordered_map）【力扣高频】【工程实践】

### ① 使用目的 & 适用场景

**解决问题**：O(1) 查找、插入、删除。

**游戏场景**：
- 资源管理器（ResourceID → Resource*）
- 实体管理器（EntityID → Entity*）
- 配置表查询

### ② 写法 & 使用步骤

```cpp
// 两数之和
vector<int> twoSum(vector<int>& nums, int target) {
    unordered_map<int, int> map;  // value → index
    
    for (int i = 0; i < nums.size(); ++i) {
        int complement = target - nums[i];
        if (map.count(complement)) {
            return {map[complement], i};
        }
        map[nums[i]] = i;
    }
    return {};
}

// 字母异位词分组
vector<vector<string>> groupAnagrams(vector<string>& strs) {
    unordered_map<string, vector<string>> map;
    
    for (auto& str : strs) {
        string key = str;
        sort(key.begin(), key.end());
        map[key].push_back(str);
    }
    
    vector<vector<string>> res;
    for (auto& [k, v] : map) {
        res.push_back(v);
    }
    return res;
}
```

**STL 对应**：`std::unordered_map`、`std::unordered_set`。

### ③ 底层原理

- **时间复杂度**：平均 O(1)，最坏 O(n)（哈希冲突）
- **空间复杂度**：O(n)
- **优点**：查找极快
- **缺点**：内存碎片，不支持有序遍历

**常见坑**：
- 自定义类型的哈希函数
- rehash 开销（预分配 `reserve()`）

### 【选型提醒】

**不适合**：
- 需要有序遍历（用 `std::map`）
- 内存敏感场景（碎片严重）

---

# 七、堆技巧

---

## 7.1 优先队列（Top-K 问题）【力扣高频】【工程实践】

### ① 使用目的 & 适用场景

**解决问题**：找到最大/最小的 K 个元素。

**游戏场景**：
- 排行榜（Top 100 玩家）
- 最近的 N 个敌人
- A* 寻路的 open list

### ② 写法 & 使用步骤

```cpp
// 前 K 个高频元素
vector<int> topKFrequent(vector<int>& nums, int k) {
    unordered_map<int, int> count;
    for (int num : nums) count[num]++;
    
    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<>> pq;
    
    for (auto& [num, freq] : count) {
        pq.push({freq, num});
        if (pq.size() > k) pq.pop();
    }
    
    vector<int> res;
    while (!pq.empty()) {
        res.push_back(pq.top().second);
        pq.pop();
    }
    return res;
}

// 数据流中第 K 大的元素
class KthLargest {
    priority_queue<int, vector<int>, greater<int>> pq;
    int k;
public:
    KthLargest(int k, vector<int>& nums) : k(k) {
        for (int num : nums) add(num);
    }
    
    int add(int val) {
        pq.push(val);
        if (pq.size() > k) pq.pop();
        return pq.top();
    }
};
```

**STL 对应**：`std::priority_queue`。

### ③ 底层原理

- **时间复杂度**：O(n log k)
- **空间复杂度**：O(k)
- **优点**：空间最优（只需维护 K 个元素）
- **缺点**：不支持随机访问

**常见坑**：
- 最小堆 vs 最大堆（`greater<>` 是最小堆）
- 堆大小控制（`pq.size() > k` 时 pop）

### 【选型提醒】

**不适合**：
- 需要完整排序（直接 sort）
- K 很大（接近 n，用快排 partition）

---

# 八、动态规划技巧

---

## 8.1 背包问题【力扣高频】

### ① 使用目的 & 适用场景

**解决问题**：0-1 背包、完全背包、分组背包。

**游戏场景**：
- 装备选择优化（最大价值）
- 技能点分配

### ② 写法 & 使用步骤

```cpp
// 0-1 背包（一维优化）
int knapsack01(int W, vector<int>& weights, vector<int>& values) {
    int n = weights.size();
    vector<int> dp(W + 1, 0);
    
    for (int i = 0; i < n; ++i) {
        for (int w = W; w >= weights[i]; --w) {  // 逆序！
            dp[w] = max(dp[w], dp[w - weights[i]] + values[i]);
        }
    }
    return dp[W];
}

// 完全背包（物品可重复使用）
int knapsackComplete(int W, vector<int>& weights, vector<int>& values) {
    int n = weights.size();
    vector<int> dp(W + 1, 0);
    
    for (int i = 0; i < n; ++i) {
        for (int w = weights[i]; w <= W; ++w) {  // 正序！
            dp[w] = max(dp[w], dp[w - weights[i]] + values[i]);
        }
    }
    return dp[W];
}
```

### ③ 底层原理

- **时间复杂度**：O(nW)
- **空间复杂度**：O(W)（一维优化）
- **优点**：空间优化后很高效
- **缺点**：W 很大时性能差

**常见坑**：
- 0-1 背包逆序遍历（保证每个物品只用一次）
- 完全背包正序遍历（物品可重复）

### 【选型提醒】

**不适合**：
- W 很大（用记忆化搜索或贪心近似）
- 物品数量很多（状态压缩 DP）

---

## 8.2 最长递增子序列（LIS）【力扣高频】

### ① 使用目的 & 适用场景

**解决问题**：找到最长递增子序列的长度。

**游戏场景**：
- 动画压缩（找最长单调序列）

### ② 写法 & 使用步骤

```cpp
// O(n log n) 解法（贪心 + 二分）
int lengthOfLIS(vector<int>& nums) {
    vector<int> tails;
    
    for (int num : nums) {
        auto it = lower_bound(tails.begin(), tails.end(), num);
        if (it == tails.end()) tails.push_back(num);
        else *it = num;
    }
    return tails.size();
}
```

### ③ 底层原理

- **时间复杂度**：O(n log n)
- **空间复杂度**：O(n)
- **优点**：比 O(n²) DP 更快
- **缺点**：不容易还原具体序列

**常见坑**：
- `lower_bound` 的使用（找第一个 ≥ num 的位置）
- tails 数组的含义（长度为 i+1 的 LIS 的最小末尾）

### 【选型提醒】

**不适合**：
- 需要具体序列（用 O(n²) DP）
- 非递增子序列（修改比较逻辑）

---

# 九、性能优化技巧

---

## 9.1 位运算优化【工程实践】

### ① 使用目的 & 适用场景

**解决问题**：用位运算替代算术运算，提升性能。

**游戏场景**：
- 标志位管理（状态标志）
- 快速乘除 2 的幂

### ② 写法 & 使用步骤

```cpp
// 快速判断 2 的幂
bool isPowerOfTwo(int n) {
    return n > 0 && (n & (n - 1)) == 0;
}

// 统计二进制中 1 的个数
int hammingWeight(uint32_t n) {
    int count = 0;
    while (n) {
        n &= (n - 1);  // 消除最低位的 1
        count++;
    }
    return count;
}

// 交换两个数（不用临时变量）
void swap(int& a, int& b) {
    a ^= b;
    b ^= a;
    a ^= b;
}

// 标志位操作
enum Flags {
    FLAG_VISIBLE = 1 << 0,
    FLAG_ACTIVE  = 1 << 1,
    FLAG_SELECTED = 1 << 2,
};

uint32_t flags = 0;
flags |= FLAG_VISIBLE;      // 设置标志
flags &= ~FLAG_ACTIVE;      // 清除标志
bool isSelected = flags & FLAG_SELECTED;  // 检查标志
```

### ③ 底层原理

- **时间复杂度**：O(1)
- **空间复杂度**：O(1)
- **优点**：极快（单条指令）
- **缺点**：可读性差

**常见坑**：
- 运算符优先级（`&` 优先级低于 `==`）
- 符号位处理

### 【选型提醒】

**不适合**：
- 非 2 的幂运算
- 需要高可读性的代码

---

## 9.2 内存池 & 对象池【工程实践】

### ① 使用目的 & 适用场景

**解决问题**：避免频繁 new/delete，减少内存碎片。

**游戏场景**：
- 子弹池、粒子池
- 实体对象池

### ② 写法 & 使用步骤

```cpp
template<typename T>
class ObjectPool {
    vector<T*> pool;
    vector<T*> active;
public:
    ObjectPool(size_t size) {
        for (size_t i = 0; i < size; ++i) {
            pool.push_back(new T());
        }
    }
    
    T* acquire() {
        if (pool.empty()) return new T();
        T* obj = pool.back();
        pool.pop_back();
        active.push_back(obj);
        return obj;
    }
    
    void release(T* obj) {
        pool.push_back(obj);
        active.erase(find(active.begin(), active.end(), obj));
    }
};
```

### ③ 底层原理

- **时间复杂度**：acquire/release O(1)
- **空间复杂度**：O(n)
- **优点**：避免内存分配开销
- **缺点**：预分配内存

**常见坑**：
- 池大小不当（浪费或扩容）
- 对象状态重置

### 【选型提醒】

**不适合**：
- 对象生命周期很长
- 对象种类很多

---

# 十、面试一句话速记清单

## 链表

1. **虚拟头节点**：头节点可能被修改时，用 dummy 统一处理
2. **快慢指针**：检测环用快慢指针，找入口时从 head 和相遇点同时出发
3. **反转链表**：迭代法保存 next，递归法注意返回值
4. **合并 K 个链表**：优先队列 O(N log k)，分治法 O(N log k)

## 数组 & 双指针

5. **对撞指针**：有序数组两数之和，左小右大
6. **快慢指针**：原地移除元素，slow 指向下一个写入位置
7. **滑动窗口**：右指针扩展，左指针收缩，维护窗口状态
8. **前缀和**：区间和查询 O(1)，差分数组区间修改 O(1)

## 树

9. **二叉树遍历**：递归简洁，迭代避免栈溢出
10. **BST 操作**：查找 O(log n)，删除时找后继节点
11. **LCA**：普通树递归，BST 利用性质迭代

## 图搜索

12. **BFS**：无权图最短路径，队列控制层数
13. **DFS**：连通性检测，递归注意栈溢出
14. **Dijkstra**：优先队列，懒惰删除
15. **A\***：启发式搜索，h(n) 必须可接受

## 排序

16. **快排**：三路划分处理重复元素，随机 pivot 避免最坏
17. **归并**：稳定排序，需要 O(n) 额外空间

## 哈希

18. **unordered_map**：O(1) 查找，预分配 reserve()
19. **自定义哈希**：实现 hash 函数和 operator==

## 堆

20. **Top-K**：最小堆维护 K 个元素，O(n log k)
21. **优先队列**：`greater<>` 是最小堆

## 动态规划

22. **0-1 背包**：逆序遍历，保证每个物品只用一次
23. **完全背包**：正序遍历，物品可重复
24. **LIS**：贪心 + 二分 O(n log n)

## 性能优化

25. **位运算**：`n & (n-1)` 消除最低位的 1
26. **对象池**：acquire/release O(1)，避免 new/delete

---

> **手册结语**：数据结构和算法的选择本质上是在**时间、空间、实现复杂度**之间做权衡。游戏客户端开发的特殊之处在于：**性能是可预测的**。始终从实际场景出发，考虑帧率预算、内存限制、平台特性。面试时先问清约束，再选择最优方案。
