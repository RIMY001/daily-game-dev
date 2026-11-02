#include <cstdio>
#include <utility>
#include "my_vector.h"

static int g_fail = 0;

#define CHECK(cond)																	\
	do {																			\
		if(!(cond)) {																\
			std::printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #cond)				\
			++g_fail;																\
		}																			\
	} while (0)	

// ---------- 用存活计数验证构造/析构/深拷贝 ----------
struct Tracked {
	static int alive;
	int v;
	explicit Tracked(int x = 0) : v(x) { ++alive; }
	Tracked(const Tracked& o) : v(o.v) { ++alive; }
	Tracked(Tracked&& o) noexcept : v(o.v) { o.v = -1; ++alive; }
	Tracked& operator=(const Tracked& o) { v = o.v; return *this; }
	Tracked& operator=(Tracked&& o) noexcept { v = o.v; o.v = -1; return *this; }
	~Tracked() { --alive; }
}; 
int Tracked::alive = 0;

// ---------- 1. push_back 与增长 ----------
static void test_push_growth() {
	std::printf("[1] push_back / growth\n");
	my::vector<int> v;
	CHECK(v.size() == 0);
	CHECK(v.empty());
	for (int i = 0; i < 100; ++i) v.push_back(i);
	CHECK(v.size() == 100);
	CHECK(!v.empty());
	CHECK(v.capacity() >= 100);
	CHECK(v[0] == 0);
	CHECK(v[99] == 99);
	CHECK(v.front() == 0);
	CHECK(v.back() == 99);
}

// ---------- 2. reserve 之后指针稳定（不应偷偷扩容） ----------
static void test_reserve_stable() {
	std::printf("[2] reserve keeps pointer\n");
	my::vector<int> v;
	v.reserve(128);
	CHECK(v.capacity() >= 128);
	const int* p = v.data();
	for (int i = 0; i < 10; ++i) v.push_back(i);
	CHECK(v.data() == p);
}

// ---------- 3. 深拷贝：两个 vector 互相独立 ----------
static void test_copy_independent() {
	std::printf("[3] deep copy\n");
	{
		my::vector<Tracked> a;
		a.push_back(Tracked(1));
		a.push_back(Tracked(2));
		CHECK(Tracked::alive == 2);
		my::vector<Tracked> b = a;          // 深拷贝
		CHECK(a.size() == 2);
		CHECK(b.size() == 2);
		CHECK(Tracked::alive == 4);         // 复制出 2 个新对象
		CHECK(b[0].v == 1);
		b[0].v = 100;
		CHECK(a[0].v == 1);                 // 互不影响
	}
	CHECK(Tracked::alive == 0);
}

// ---------- 4. 移动：偷缓冲区，源清空 ----------
static void test_move_steal() {
	std::printf("[4] move steals buffer\n");
	my::vector<int> a;
	a.push_back(1);
	a.push_back(2);
	int* old = a.data();
	my::vector<int> b = std::move(a);
	CHECK(b.size() == 2);
	CHECK(b.data() == old);                 // 没有拷贝，直接接管
	CHECK(a.size() == 0);                   // 源被置空
	int* old2 = b.data();
	my::vector<int> c;
	c = std::move(b);
	CHECK(c.data() == old2);
	CHECK(b.size() == 0);
}

// ---------- 5. 自赋值 ----------
static void test_self_assign() {
	std::printf("[5] self assignment\n");
	my::vector<int> v;
	v.push_back(7);
	v = v;
	CHECK(v.size() == 1);
	CHECK(v[0] == 7);
}

// ---------- 6. clear 保留容量 ----------
static void test_clear_keeps_capacity() {
	std::printf("[6] clear keeps capacity\n");
	my::vector<Tracked> v;
	v.reserve(16);
	v.push_back(Tracked(1));
	v.push_back(Tracked(2));
	size_t cap = v.capacity();
	v.clear();
	CHECK(v.size() == 0);
	CHECK(v.capacity() == cap);
	CHECK(Tracked::alive == 0);
}

// ---------- 7. pop_back ----------
static void test_pop_back() {
	std::printf("[7] pop_back\n");
	my::vector<Tracked> v;
	v.push_back(Tracked(1));
	v.push_back(Tracked(2));
	v.pop_back();
	CHECK(v.size() == 1);
	CHECK(Tracked::alive == 1);
	v.pop_back();
	CHECK(v.size() == 0);
	CHECK(Tracked::alive == 0);
}

int main() {
	test_push_growth();
	test_reserve_stable();
	test_copy_independent();
	test_move_steal();
	test_self_assign();
	test_clear_keeps_capacity();
	test_pop_back();
	if (Tracked::alive != 0) {
		std::printf("FAIL  leak: Tracked::alive = %d\n", Tracked::alive);
		++g_fail;
	}
	if (g_fail == 0) {
		std::printf("\nALL PASS\n");
		return 0;
	}
	std::printf("\n%d CHECK FAILED\n", g_fail);
	return 1;
}