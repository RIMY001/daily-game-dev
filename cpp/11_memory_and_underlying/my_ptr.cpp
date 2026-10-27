#include <cstdio>
#include <utility>
#include "my_ptr.h"

static int g_fail = 0;

#define CHECK(cond) do { \
	if (!(cond)) { \
		std::printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #cond); \
		++g_fail; \
	} \
} while (0)

// --------------------- 被管理对象：用存活计数验证析构时机 ---------------------
struct Tracked {
	static int alive;
	int value;
	explicit Tracked(int v) : value(v) { ++alive; }
	~Tracked() { --alive; }
};
int Tracked::alive = 0;

// -------------------- 用例1：拷贝与引用计数 --------------------
static void test_copy_and_count() {
	std::printf("[1] copy / use_count\n");
	{
		my::shared_ptr<Tracked> p = my::make_shared<Tracked>(7);
		CHECK(p.use_count() == 1);
		CHECK(p->value == 7);
		CHECK(Tracked::alive == 1);

		{
			my::shared_ptr<Tracked> q = p;	//	拷贝 +1
			CHECK(p.use_count() == 2);
			CHECK(q.use_count() == 2);
			CHECK(q.get() == p.get());
		}									//	q 析构 -1
		CHECK(p.use_count() == 1);
		CHECK(Tracked::alive == 1);
	}
	CHECK(Tracked::alive == 0);				//	最后一个释放后对象销毁
}

// -------------------- 用例2：移动不改变计数 --------------------
static void test_move() {
	std::printf("[2] move\n");
	my::shared_ptr<Tracked> a = my::make_shared<Tracked>(1);
	my::shared_ptr<Tracked> b = std::move(a);
	CHECK(b.use_count() == 1);
	CHECK(a.get() == nullptr);
	CHECK(a.use_count() == 0);
	CHECK(static_cast<bool>(b));
	CHECK(!static_cast<bool>(a));
	CHECK(Tracked::alive == 1);
}

// -------------------- 用例3：reset --------------------
static void test_reset() {
	std::printf("[3] reset\n");
	my::shared_ptr<Tracked> p = my::make_shared<Tracked>(2);
	CHECK(Tracked::alive == 1);
	p.reset();
	CHECK(p.get() == nullptr);
	CHECK(p.use_count() == 0);
	CHECK(Tracked::alive == 0);
}

// -------------------- 用例4：weak_ptr 提升 --------------------
static void test_weak_lock() {
	std::printf("[4] weak_ptr::lock\n");
	my::weak_ptr<Tracked> w;
	{
		my::shared_ptr<Tracked> p = my::make_shared<Tracked>(3);
		w = my::weak_ptr<Tracked>(p);
		CHECK(w.use_count() == 1);	//	弱引用不影响强计数
		CHECK(!w.expired());

		my::shared_ptr<Tracked> locked = w.lock();
		CHECK(locked.get() == p.get());
		CHECK(p.use_count() == 2);	//	lock 成功期间 +1
	}
	CHECK(w.expired());				//	对象已销毁
	CHECK(w.lock().get() == nullptr);	//	提升失败返回空
	CHECK(Tracked::alive == 0);
}

// -------------------- 用例5：循环引用 (用 weak 打破） --------------------
struct Node {
	my::shared_ptr<Node> next;
	my::weak_ptr<Node> back;	//	反向用弱引用，环不该泄露
	int id = 0;
};

static void test_cycle() {
	std::printf("[5] circular reference\n");
	{
		my::shared_ptr<Node> a = my::make_shared<Node>();
		my::shared_ptr<Node> b = my::make_shared<Node>();
		a->id = 1; b->id = 2;
		a->next = b;	//	强
		b->back = a;	//	弱，不形成强环
	}
	//	强引用都退出作用域后应全部释放；若实现有泄露，Asan 会报告
	std::printf("	(对象是否释放由 Asan/泄露检查确认)\n");
}

// -------------------- 用例6：空指针调用 --------------------
static void test_default() {
	std::printf("[6] default construct\n");
	my::shared_ptr<Tracked> p;
	CHECK(p.get() == nullptr);
	CHECK(p.use_count() == 0);
	CHECK(!p);
	p.reset();	//	空指针 reset 不能崩
	CHECK(p.get() == nullptr);
}

int main() {
	test_copy_and_count();
	test_move();
	test_reset();
	test_weak_lock();
	test_cycle();
	test_default();

	if (g_fail == 0) {
		std::printf("\nALL PASS\n");
		return 0;
	}
	std::printf("\n%d CHECK FAILED\n", g_fail);
	return 1;
}
