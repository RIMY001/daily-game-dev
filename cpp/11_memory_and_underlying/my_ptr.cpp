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

//// ========== 新增 用例7：传入裸空指针给 shared_ptr(T*) 构造 ==========
//static void test_null_rawptr()
//{
//	std::printf("[7] raw nullptr constructor\n");
//	Tracked* raw = nullptr;
//	my::shared_ptr<Tracked> sp(raw);
//	CHECK(sp.get() == nullptr);
//	CHECK(sp.use_count() == 0);
//	CHECK(!sp);
//	sp.reset();
//	CHECK(sp.get() == nullptr);
//	CHECK(Tracked::alive == 0);
//}

// ========== 新增 用例8：weak_ptr 拷贝构造 / 拷贝赋值，weak计数增加 ==========
static void test_weak_copy()
{
	std::printf("[8] weak_ptr copy\n");
	my::shared_ptr<Tracked> sp = my::make_shared<Tracked>(100);
	my::weak_ptr<Tracked> w1(sp);
	CHECK(!w1.expired());
	CHECK(w1.use_count() == 1);

	my::weak_ptr<Tracked> w2 = w1; // weak拷贝，weak_cnt++，强计数不变
	CHECK(w1.use_count() == 1);
	CHECK(w2.use_count() == 1);

	{
		my::weak_ptr<Tracked> w3;
		w3 = w2;
		CHECK(!w3.expired());
		CHECK(w3.lock()->value == 100);
	}
	CHECK(sp.use_count() == 1);
}

// ========== 新增 用例9：strong归零、weak还在：对象销毁，控制块存活 ==========
static void test_cb_survive_after_object_destroy()
{
	std::printf("[9] control block survives weak alive\n");
	my::weak_ptr<Tracked> w;
	{
		my::shared_ptr<Tracked> sp = my::make_shared<Tracked>(200);
		w = sp;
		CHECK(Tracked::alive == 1);
	}
	// shared全部销毁：strong=0 → Tracked析构；weak>0，控制块保留
	CHECK(Tracked::alive == 0);
	CHECK(w.expired() == true);
	CHECK(w.lock().get() == nullptr);
	// 这里不能直接检查控制块是否在内存（用户层看不到）
	// 但语义上：weak依然可以安全调用expired/lock，不会野指针崩溃
}

int main() {
	test_copy_and_count();
	test_move();
	test_reset();
	test_weak_lock();
	test_cycle();
	test_default();

	//	test_null_rawptr();
	test_weak_copy();
	test_cb_survive_after_object_destroy();

	if (g_fail == 0) {
		std::printf("\nALL PASS\n");
		return 0;
	}
	std::printf("\n%d CHECK FAILED\n", g_fail);
	return 1;
}
