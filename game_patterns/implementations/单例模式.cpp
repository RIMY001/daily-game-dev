#include <iostream>
#include <mutex>
#include <cassert>
using namespace std;

//	方案一：Meyers 单例 (推荐，C++11线程安全)
class GameManager {
public:
	static GameManager& getInstance() {
		// 局部静态变量：第一次调用getInstance才构造，懒加载
        // C++11标准规定：局部static变量初始化，多线程自动保护，不用手动加锁
		static GameManager instance;	//	这个变量不属于 “类的成员”，它只是函数里面的变量，自然不需要类外GameManager::xxx定义。
		return instance;
	}

	void update(float deltaTime) { std::cout << "GameManager update, dt = " << deltaTime << std::endl; }
	void pause() { std::cout << "GameManager pause" << std::endl; }

	//	禁止拷贝
	// 删除拷贝构造函数，禁止复制单例对象
	GameManager(const GameManager&) = delete;
	// 删除赋值运算符重载，禁止 a = b 拷贝单例
	GameManager& operator=(const GameManager&) = delete;

private:
	GameManager() {}	//	私有构造函数：外部不能new GameManager()
	~GameManager() {}	// 私有析构：外部不能delete这个实例
};


//返回引用，不会有空指针风险
//生命周期：程序退出时，由 C++ 运行时自动销毁
//优点：代码极简、C++11 起天然线程安全、懒加载、无内存泄漏
//缺点：
//你不能手动控制销毁时机，是 main 结束后才析构；游戏引擎想在关卡结束 / 引擎关闭主动清理资源就不方便
//跨 DLL 边界时，不同 DLL 会各自生成实例（坑！游戏项目容易踩）
//面试一句话：Meyers 单例，局部 static，C++11 保证初始化阶段线程安全；实例在程序退出自动析构。

//	方案二：双重检查锁（懒加载 + 指针）
class ResourceManager {
	static ResourceManager* instance;	// 静态全局指针，存单例地址
	static mutex mtx;	// 互斥锁，保护多线程初始化

public:
	static ResourceManager* getInstance() {
		if (instance == nullptr) {	//	第一次检查（无锁）无锁快速判断。实例已经创建，直接返回，避免每次拿锁（锁开销大）
			//	RAII锁，离开作用域自动解锁
			lock_guard<mutex> lock(mtx);
			if (instance == nullptr) {	//	第二次检查（有锁）防止多个线程同时进入外层if，阻塞后重复new
				instance = new ResourceManager();
			}
		}
		return instance;
	}

	//	禁止拷贝
	ResourceManager(const ResourceManager&) = delete;
	ResourceManager& operator=(const ResourceManager&) = delete;

private:
	ResourceManager() {}
};
// ==========【重点！静态成员定义，不能少！】==========
ResourceManager* ResourceManager::instance = nullptr;
std::mutex ResourceManager::mtx;
//	缺点：代码复杂、容易内存序踩坑；new 出来的对象不会自动 delete，需要手动处理销毁，否则内存泄漏。
//	游戏开发现在很少手写 DCL，基本优先 Meyers 或者引擎手动生命周期单例。

//	方案三：引擎风格（手动生命周期控制）
class AudioSystem {
	static AudioSystem* s_instance;
public:
	// 引擎初始化阶段手动调用一次，创建实例
	static void create() {
		// 断言：开发模式校验，防止重复create，重复创建会崩溃提醒
		assert(s_instance == nullptr);
		s_instance = new AudioSystem();
	}

	// 引擎关闭/关卡卸载手动调用，主动释放资源
	static void destroy() {
		assert(s_instance != nullptr);
		delete s_instance;
		s_instance = nullptr;	// 置空，防止野指针！非常关键
	}

	// 获取实例引用，断言保护，在没create的时候访问直接触发assert报错
	static AudioSystem& getInstance() {
		assert(s_instance != nullptr);
		return *s_instance;
	}

	void playSound(int soundId) { std::cout << "Play sound id:" << soundId << std::endl; }

private:
	AudioSystem() {}
	~AudioSystem() {}
};
// ==========【重点！静态成员定义，不能少！】==========
AudioSystem* AudioSystem::s_instance = nullptr;
/*优点（游戏引擎最爱）：
完全手动控制创建和销毁时机！
音频、渲染、物理系统，引擎要在合适时机释放显存、音频句柄、物理资源。Meyers 单例等到程序结束才析构，资源释放太晚，会造成驱动 / 引擎资源泄漏。
没有线程初始化竞争问题（一般在主线程引擎启动阶段调用 create，不会多线程同时创建）
可在 destroy 里面做资源清理、日志打印、调试校验
缺点：
必须严格遵守调用顺序：先 create，再 getInstance，最后 destroy；调用顺序错直接 assert 崩溃。
如果忘记调用 destroy，造成内存泄漏。
Piccolo、Unity/UE 内部很多管理器都是这套思路。面试重点：游戏引擎为什么不用 Meyers，而用手动生命周期单例？*/

// ============ 主函数 测试入口 ============
int main()
{
	std::cout << "===== 测试 Meyers 单例 =====" << std::endl;
	GameManager::getInstance().update(0.016f);
	GameManager::getInstance().pause();

	std::cout << "\n===== 测试DCL双重检查锁 =====" << std::endl;
	ResourceManager* resMgr = ResourceManager::getInstance();
	std::cout << "ResourceManager ptr: " << resMgr << std::endl;

	std::cout << "\n===== 测试引擎手动生命周期单例 =====" << std::endl;
	AudioSystem::create();
	AudioSystem::getInstance().playSound(1001);
	AudioSystem::destroy();

	// AudioSystem::getInstance(); // 取消注释，Debug模式会触发assert崩溃！

	return 0;
}