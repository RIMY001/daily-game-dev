/*待改进代码示例
#include <iostream>
using namespace std;
int main()
{
	int arr[7] = { 1,5,12,4,6,98,14 };
	int target;//定义用户输入的要查找的数
	for (int i = 0; i < 7; i++)//遍历一遍数组
	{
		cout << arr[i] << " ";
	}
	cout << endl;
	for ( int j = 0 ; j < j + 1 ; j++)//让循环一直进行下去直到找到一个存在于数组中的值
	{ 
		cout << "Please enter a number that you want to find" << endl;//让用户输入
		cin >> target;
		//判断输入的数字是否存在于数组中
		for (int i = 0; i < 7; i++)
		{
			if (target == arr[i])
			{
				cout << "The number you entered is finded" << endl;
				cout << "Index: " << i << endl;
				target = -1;
				break;
			}
		}
		if (target != -1)
		{
			cout << "You entered number is not in array." << endl;
		}
		else
		{
			break;
		}
	}
	system("pause");
	return 0;
}*/

/*
问题所在： 
主要问题
1. 外层循环条件错误（最严重）
cpp
for (int j = 0; j < j + 1；j++)  // ❌ 危险！
这个条件 j < j + 1 永远为真（对于正常整数），因为任何数都小于它加1。这会导致：
✅ 理论上会一直循环
⚠️ 实际上当 j 增加到 INT_MAX 时会溢出，变成负数，可能导致未定义行为
2. 使用 target 作为控制变量不合理
cpp
target = -1;  // 用目标值作为标志位
这种做法混淆了"用户输入的值"和"程序状态标志"，代码可读性差。
3. 找到元素后只退出内层循环
虽然你用了 break，但逻辑不够清晰。
*/
#include <iostream>
using namespace std;

int main()
{
    int arr[7] = { 1, 5, 12, 4, 6, 98, 14 };

    // 显示数组内容
    cout << "Array elements: ";
    for (int i = 0; i < 7; i++)
    {
        cout << arr[i] << " ";
    }
    cout << endl;

    // ✅ 使用 while 循环持续接收用户输入
    while (true)
    {
        int target;
        cout << "\nPlease enter a number to find (or -1 to exit): ";
        cin >> target;

        // ✅ 提供退出机制
        if (target == -1)
        {
            cout << "Exiting program." << endl;
            break;
        }

        // ✅ 使用布尔标志位，逻辑更清晰
        bool found = false;
        int index = -1;

        for (int i = 0; i < 7; i++)
        {
            if (target == arr[i])
            {
                found = true;
                index = i;
                break;  // 找到后立即退出
            }
        }

        // ✅ 根据标志位输出结果
        if (found)
        {
            cout << "Number " << target << " found at index: " << index << endl;
        }
        else
        {
            cout << "Number " << target << " is not in the array." << endl;
        }
    }

    system("pause");
    return 0;
}

/*一些游戏开发场景
结合前文讨论的线性查找（Linear Search）代码，请详细解释这种基础查找算法在游戏开发中的具体应用场景。例如：在玩家背包中查找特定物品ID、在NPC列表中查找目标角色、或检测碰撞体数组中的特定对象时，线性查找有哪些优缺点？在什么数据规模或场景下，游戏开发者会优先选择线性查找而不是二分查找或哈希表？请给出具体的游戏逻辑案例。结合前文讨论的线性查找（Linear Search）代码，请详细解释这种基础查找算法在游戏开发中的具体应用场景。例如：在玩家背包中查找特定物品ID、在NPC列表中查找目标角色、或检测碰撞体数组中的特定对象时，线性查找有哪些优缺点？在什么数据规模或场景下，游戏开发者会优先选择线性查找而不是二分查找或哈希表？请给出具体的游戏逻辑案例。这是一个非常实际的问题！线性查找在游戏开发中有着广泛的应用场景。让我结合游戏开发的实际需求详细分析：🎮 线性查找在游戏开发中的应用场景1. 玩家背包系统cpp// 游戏中的背包物品结构
struct Item {
    int id;
    string name;
    int quantity;
    bool isEquipped;
};

class PlayerInventory {
private:
    Item items[50];  // 背包通常容量有限
    int itemCount;

public:
    // ✅ 线性查找：在背包中查找特定物品
    int findItem(int targetId) {
        for (int i = 0; i < itemCount; i++) {
            if (items[i].id == targetId) {
                return i;  // 返回物品位置
            }
        }
        return -1;  // 未找到
    }

    // 使用示例
    void useItem(int itemId) {
        int index = findItem(itemId);
        if (index != -1) {
            items[index].quantity--;
            cout << "使用了: " << items[index].name << endl;
        } else {
            cout << "背包中没有该物品！" << endl;
        }
    }
};为什么用线性查找？✅ 背包容量小（通常 20-100 个物品）✅ 玩家操作频率低（不会每帧都查找）✅ 实现简单，易于调试✅ 数据经常变化（添加/删除物品），维护排序或哈希成本高2. NPC 列表管理cppstruct NPC {
    int id;
    string name;
    float x, y;  // 位置
    bool isActive;
    string currentDialogue;
};

class GameManager {
private:
    NPC npcs[100];  // 当前场景的 NPC 数量有限
    int npcCount;

public:
    // ✅ 线性查找：找到特定 NPC
    NPC* findNPC(int targetId) {
        for (int i = 0; i < npcCount; i++) {
            if (npcs[i].id == targetId && npcs[i].isActive) {
                return &npcs[i];
            }
        }
        return nullptr;
    }

    // ✅ 线性查找：找到最近的 NPC
    NPC* findNearestNPC(float playerX, float playerY) {
        NPC* nearest = nullptr;
        float minDistance = FLT_MAX;

        for (int i = 0; i < npcCount; i++) {
            if (!npcs[i].isActive) continue;

            float dist = sqrt(
                pow(npcs[i].x - playerX, 2) +
                pow(npcs[i].y - playerY, 2)
            );

            if (dist < minDistance) {
                minDistance = dist;
                nearest = &npcs[i];
            }
        }
        return nearest;
    }
};实际应用案例：plaintext【对话系统】
玩家按下 E 键 → 线性查找范围内可交互的 NPC → 触发对话

【任务系统】
任务要求"与村长对话" → 线性查找 ID=1001 的 NPC → 更新任务状态3. 碰撞检测cppstruct GameObject {
    int id;
    string tag;  // "Enemy", "Player", "Obstacle"
    float x, y, width, height;
    bool isActive;
};

class CollisionSystem {
private:
    GameObject objects[200];
    int objectCount;

public:
    // ✅ 线性查找：检测玩家与所有敌人的碰撞
    vector<GameObject*> checkCollisions(GameObject& player) {
        vector<GameObject*> collidedObjects;

        for (int i = 0; i < objectCount; i++) {
            if (!objects[i].isActive) continue;
            if (objects[i].tag != "Enemy") continue;

            // AABB 碰撞检测
            if (isColliding(player, objects[i])) {
                collidedObjects.push_back(&objects[i]);
            }
        }
        return collidedObjects;
    }

private:
    bool isColliding(GameObject& a, GameObject& b) {
        return !(a.x + a.width < b.x ||
                 b.x + b.width < a.x ||
                 a.y + a.height < b.y ||
                 b.y + b.height < a.y);
    }
};性能考虑：每帧执行 60 次（60 FPS）对象数量 < 200 时，线性查找完全可接受如果对象过多，会使用空间分区（四叉树、网格）优化4. 技能/特效系统cppstruct Buff {
    int id;
    string name;
    float duration;
    float remainingTime;
    bool isActive;
};

class Character {
private:
    Buff buffs[20];  // 角色身上的增益/减益效果
    int buffCount;

public:
    // ✅ 线性查找：检查是否有特定 Buff
    bool hasBuff(int buffId) {
        for (int i = 0; i < buffCount; i++) {
            if (buffs[i].id == buffId && buffs[i].isActive) {
                return true;
            }
        }
        return false;
    }

    // ✅ 线性查找：移除过期 Buff
    void updateBuffs(float deltaTime) {
        for (int i = 0; i < buffCount; i++) {
            if (!buffs[i].isActive) continue;

            buffs[i].remainingTime -= deltaTime;
            if (buffs[i].remainingTime <= 0) {
                buffs[i].isActive = false;
                cout << "Buff 过期: " << buffs[i].name << endl;
            }
        }
    }
};📊 线性查找 vs 二分查找 vs 哈希表性能对比表 算法  时间复杂度  前提条件  适用场景  线性查找  O(n)  无  小数据集、无序数据  二分查找  O(log n)  必须排序  大数据集、静态数据  哈希表  O(1) 平均  需要哈希函数  频繁查找、大数据集 🎯 游戏开发者选择线性查找的场景✅ 优先选择线性查找的情况1. 数据规模小（n < 100）cpp// 示例：玩家的装备栏（最多 10 个槽位）
Item equipment[10];

// 线性查找 10 个元素：最多 10 次比较
// 二分查找需要先排序，反而更慢！原因：常数因子小，实际速度快无需维护排序状态代码简单，不易出错2. 数据频繁变化cpp// 示例：动态生成的敌人列表
vector<Enemy> activeEnemies;

// 每帧都有敌人死亡、生成
// 如果使用二分查找，每次插入都要重新排序 O(n log n)
// 线性查找只需 O(n)，总体更快3. 只需要查找一次或少数几次cpp// 示例：加载关卡时查找初始玩家位置
SpawnPoint findPlayerSpawn() {
    for (auto& point : spawnPoints) {
        if (point.type == PLAYER_SPAWN) {
            return point;
        }
    }
}
// 只执行一次，不需要优化4. 需要遍历所有元素做额外处理cpp// 示例：更新所有激活的技能冷却
for (auto& skill : skills) {
    if (skill.isActive) {
        skill.cooldown -= deltaTime;
        if (skill.cooldown <= 0) {
            skill.isReady = true;
        }
    }
}
// 反正要遍历，顺便查找，无需额外数据结构5. 内存受限的平台（移动端、主机）cpp// 线性查找：只需数组，额外内存 O(1)
// 哈希表：需要额外存储哈希桶、指针等，内存开销大❌ 不应该使用线性查找的情况1. 大数据集（n > 1000）且频繁查找cpp// ❌ 糟糕的设计：每帧线性查找 10000 个物体
GameObject allObjects[10000];
for (int i = 0; i < 10000; i++) {
    if (allObjects[i].id == targetId) { ... }
}

// ✅ 应该使用哈希表
unordered_map<int, GameObject*> objectMap;
auto it = objectMap.find(targetId);  // O(1)2. 数据静态不变且需要快速查找cpp// ❌ 物品数据库（1000+ 物品，几乎不变）
Item itemDatabase[1500];
linearSearch(itemDatabase, 1500, itemId);

// ✅ 应该排序后用二分查找，或用哈希表
sort(itemDatabase, itemDatabase + 1500);
binary_search(itemDatabase, 1500, itemId);  // O(log n)3. 实时性要求极高的核心循环cpp// ❌ 每帧执行的网络消息处理（可能有数千条消息）
for (auto& msg : pendingMessages) {
    if (msg.playerId == currentPlayerId) { ... }
}

// ✅ 应该用哈希表按玩家 ID 索引
unordered_map<int, vector<Message>> messagesByPlayer;🎮 真实游戏案例分析案例 1：《Minecraft》的物品栏plaintext数据结构：9 个快捷栏槽位 + 27 个主背包槽位
查找频率：玩家按键时（低频）
选择：线性查找
原因：
- 数据量极小（36 个物品）
- 频繁增删物品
- 不需要复杂索引案例 2：《英雄联盟》的 Buff 系统plaintext数据结构：每个英雄最多 20-30 个 Buff
查找频率：每帧更新（高频）
选择：线性查找 + 数组
原因：
- Buff 数量少
- 需要遍历所有 Buff 更新时间
- 查找和更新可以合并案例 3：《魔兽世界》的拍卖行plaintext数据结构：数十万条拍卖记录
查找频率：玩家搜索时（中频）
选择：数据库索引（B+树/哈希）
原因：
- 数据量巨大
- 需要复杂查询（价格区间、品质等）
- 线性查找完全不可行💡 实际开发建议决策流程图plaintext需要查找数据？
    │
    ├─ 数据量 < 50？
    │   └─ ✅ 线性查找（最简单）
    │
    ├─ 数据量 50-500？
    │   ├─ 频繁查找？
    │   │   ├─ 数据有序？ → ✅ 二分查找
    │   │   └─ 数据无序？ → ✅ 哈希表
    │   └─ 偶尔查找？
    │       └─ ✅ 线性查找
    │
    └─ 数据量 > 500？
        ├─ 每帧查找？
        │   └─ ✅ 哈希表或空间分区
        └─ 偶尔查找？
            ├─ 数据静态？ → ✅ 二分查找
            └─ 数据动态？ → ✅ 哈希表代码模板cpp// ✅ 通用线性查找模板（游戏开发常用）
template<typename T>
int linearSearch(T arr[], int size, function<bool(const T&)> predicate) {
    for (int i = 0; i < size; i++) {
        if (predicate(arr[i])) {
            return i;
        }
    }
    return -1;
}

// 使用示例
// 查找第一个血量低于 30% 的敌人
int weakEnemyIndex = linearSearch(enemies, enemyCount,
    [](const Enemy& e) {
        return e.hp < e.maxHp * 0.3f;
    });📝 总结线性查找在游戏开发中的核心价值：
    ✅ 简单可靠：不容易出错，易于调试
    ✅ 小数据高效：n < 100 时实际性能优秀
    ✅ 灵活性强：支持任意查找条件（不只是相等比较）
    ✅ 内存友好：无需额外数据结构
    ✅ 适合动态数据：不需要维护排序或哈希
    记住这个原则：在游戏开发中，过早优化是万恶之源。先用线性查找实现功能， profiler 发现瓶颈后再优化。大多数情况下，你永远不会需要优化它！
*/