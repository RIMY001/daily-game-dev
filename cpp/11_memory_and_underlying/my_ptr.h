#pragma once
#include <utility>
namespace my {

template <typename T>
struct ControlBlock {
	long strong_cnt;
	long weak_cnt;
	static long g_cb_alive;	//	静态所有对象共享

	//	缓冲区，用来放 T,和 ControlBlock 同一块内存
	alignas(T) char storage[sizeof(T)];
	T* ptr_;

	//	删除器
	using Deleter = void(*)(T*);	//	这是一个函数指针
	Deleter deleter;
	
	ControlBlock() : strong_cnt(1), weak_cnt(0), ptr_(nullptr), deleter(nullptr) { ++g_cb_alive; }	//函数退化成指针
	//	外部裸指针版本构造
	ControlBlock(T* p, Deleter d) : strong_cnt(1), weak_cnt(0), ptr_(p), deleter(d) { ++g_cb_alive; }
	~ControlBlock() { --g_cb_alive; }

	static void default_del(T* p) { p->~T(); ::operator delete(p); }

	//	获取 T 对象的地址
	T* get_ptr() noexcept {
		return ptr_;
	}
	//	shared_ptr 计数减少，释放旧资源
	void dec_strong() {
		//	释放当前持有的资源
		this->strong_cnt--;
		if (this->strong_cnt == 0) {
			deleter(ptr_);

			if (this->weak_cnt == 0) {
				this->~ControlBlock<T>();
				::operator delete(this);
			}
		}
	}
	//	weak_ptr 计数减少，释放资源
	void dec_weak() {
		this->weak_cnt--;
		if (this->weak_cnt == 0 && this->strong_cnt == 0) {
			this->~ControlBlock<T>();
			::operator delete(this);
		}
	}
};
//	类外初始化静态成员
template <typename T>
long ControlBlock<T>::g_cb_alive = 0;

template <typename T>
class shared_ptr {
private:
	ControlBlock<T>* cb_;
	shared_ptr(ControlBlock<T>* cb) noexcept : cb_(cb) {}
	template<typename U, typename... Args>
	friend shared_ptr<U> make_shared(Args&&... args);
	template<typename>
	friend class weak_ptr;

public:
	shared_ptr() noexcept;	//	空
	explicit shared_ptr(T* p);	//	接管裸指针
	//  这里 shared_ptr 自动 = shared_ptr<T>，语法糖
	shared_ptr(const shared_ptr&) noexcept;	//	拷贝，计数 +1
	shared_ptr(shared_ptr&&) noexcept;	//	移动，源置空
	shared_ptr& operator=(const shared_ptr&) noexcept;
	shared_ptr& operator=(shared_ptr&&) noexcept;
	~shared_ptr();

	T& operator*() const;
	T* operator->() const;
	T* get() const noexcept;
	long use_count() const noexcept;
	void reset() noexcept;	//	放弃所有权
	explicit operator bool() const noexcept;
};

template <typename T> 
class weak_ptr {
private:
	ControlBlock<T>* cb_;
public:
	weak_ptr() noexcept;
	weak_ptr(const weak_ptr<T>&) noexcept;
	weak_ptr(const shared_ptr<T>&) noexcept;	//	不增加强引用计数
	weak_ptr(weak_ptr<T>&&) noexcept;
	weak_ptr& operator=(const weak_ptr<T>&) noexcept;
	weak_ptr& operator=(const shared_ptr<T>&) noexcept;
	weak_ptr& operator=(weak_ptr<T>&&) noexcept;
	~weak_ptr();

	shared_ptr<T> lock() const noexcept;	//	对象还在就提升，否则返回空
	bool expired() const noexcept;
	long use_count() const noexcept;
};

template <typename T, typename... Args>
shared_ptr<T> make_shared(Args&&... args);	//	一次分配（控制块 + 对象同一块内存）Demo版本：两次new，方便理解(已改）
}

template <typename T>
my::shared_ptr<T>::shared_ptr() noexcept : cb_(nullptr) {}

template <typename T>
my::shared_ptr<T>::shared_ptr(T* p) {
	if (p == nullptr) {
		cb_ = nullptr;
		return;
	}
	cb_ = new ControlBlock<T>(p, ControlBlock<T>::default_del);
}

template <typename T>
my::shared_ptr<T>::shared_ptr(const my::shared_ptr<T>& other) noexcept : cb_(other.cb_) {
	if (other.cb_) { other.cb_->strong_cnt++; }
}

template <typename T>
my::shared_ptr<T>::shared_ptr(my::shared_ptr<T>&& other) noexcept : cb_(other.cb_) {
	other.cb_ = nullptr;
}

template <typename T>
my::shared_ptr<T>& my::shared_ptr<T>::operator=(const my::shared_ptr<T>& other) noexcept {
	//	this 指向等号左边的对象，other 则是右边对象的引用（不是指针！）
	//	this 是栈上临时指针，函数调用时产生，函数退出自动销毁，不属于对象
	if (this == &other) return *this;	//	自赋值保护,防止将自己持有的资源释放后又通过引用访问可能会导致的UB

	if (cb_) cb_->dec_strong();

	//	接管 other 的控制块
	if (other.cb_) {
		other.cb_->strong_cnt++;
	}
	cb_ = other.cb_;
	return *this;
}

template <typename T>
my::shared_ptr<T>& my::shared_ptr<T>::operator=(my::shared_ptr<T>&& other) noexcept{
	if (this == &other) return *this;

	if (cb_) cb_->dec_strong();

	//	偷走对方的控制块
	cb_ = other.cb_;
	other.cb_ = nullptr;
	return *this;
}

template <typename T>
my::shared_ptr<T>::~shared_ptr() {
	//	释放当前持有的资源
	if (cb_) cb_->dec_strong();
}

template <typename T>
T& my::shared_ptr<T>::operator*() const {
	return *cb_->get_ptr();	//	注意返回的是引用类型，不能返回 cb_->ptr ，要解引用
}

template <typename T>
T* my::shared_ptr<T>::operator->() const {
	return cb_->get_ptr();
}

template <typename T>
T* my::shared_ptr<T>::get() const noexcept {
	return cb_ ? cb_->get_ptr() : nullptr;	//	注意检查是否是空指针
}

template <typename T>
long my::shared_ptr<T>::use_count() const noexcept {
	return cb_ ? cb_->strong_cnt : 0;
}

template <typename T> 
void my::shared_ptr<T>::reset() noexcept {
	//	释放当前持有的资源
	if(cb_) cb_->dec_strong();
	cb_ = nullptr;
}

template <typename T>
my::shared_ptr<T>::operator bool() const noexcept {
	return cb_ != nullptr;
}

template <typename T>
my::weak_ptr<T>::weak_ptr() noexcept : cb_(nullptr) {}

template <typename T>
my::weak_ptr<T>::weak_ptr(const my::weak_ptr<T>& other) noexcept : cb_(other.cb_) {
	if (cb_) cb_->weak_cnt++;
}

template <typename T>
my::weak_ptr<T>::weak_ptr(const my::shared_ptr<T>& other) noexcept : cb_(other.cb_) {
	if (cb_) {
		cb_->weak_cnt++;
	}
}

template <typename T>
my::weak_ptr<T>::weak_ptr(my::weak_ptr<T>&& other) noexcept :cb_(other.cb_) {
	other.cb_ = nullptr;
}

template <typename T>
my::weak_ptr<T>& my::weak_ptr<T>::operator=(const my::weak_ptr<T>& other) noexcept {
	if (cb_ == other.cb_) return *this;

	//	释放旧资源
	if (cb_) cb_->dec_weak();
	//	接管 other 的控制块
	cb_ = other.cb_;
	if (other.cb_) other.cb_->weak_cnt++;
	return *this;
}

template <typename T>
my::weak_ptr<T>& my::weak_ptr<T>::operator=(const my::shared_ptr<T>& other) noexcept {
	if (cb_ == other.cb_) return *this;

	//	释放旧资源
	if (cb_) cb_->dec_weak();
	//	接管 other 的控制块
	cb_ = other.cb_;
	if (other.cb_) other.cb_->weak_cnt++;
	return *this;
}

template <typename T>
my::weak_ptr<T>& my::weak_ptr<T>::operator=(my::weak_ptr<T>&& other) noexcept {
	if (this == &other) return *this;

	//	释放旧资源
	if (cb_) cb_->dec_weak();
	//	偷走对方的控制块
	cb_ = other.cb_;
	other.cb_ = nullptr;
	return *this;
}

template <typename T>
my::weak_ptr<T>::~weak_ptr() {
	//	释放自己的资源
	if (cb_) cb_->dec_weak();
}

template <typename T>
my::shared_ptr<T> my::weak_ptr<T>::lock() const noexcept {
	if (!cb_ || cb_->strong_cnt == 0) {
		return my::shared_ptr<T>();
	}
	cb_->strong_cnt++;
	return my::shared_ptr<T>(cb_);
}

template <typename T>
bool my::weak_ptr<T>::expired() const noexcept {
	return cb_ == nullptr || cb_->strong_cnt == 0;
}

template <typename T>
long my::weak_ptr<T>::use_count() const noexcept {
	return cb_ ? cb_->strong_cnt : 0;
}

template <typename T, typename... Args>
my::shared_ptr<T> my::make_shared(Args&&... args) {
	// 1. 一次性分配整块内存：大小 = ControlBlock<T>（内含storage）
	void* raw = ::operator new(sizeof(ControlBlock<T>));
	// 2. placement new 构造ControlBlock（此时ptr_是空，deleter是空）
	ControlBlock<T>* cb = new (raw) ControlBlock<T>();
	try {
		// 3. 在storage缓冲区上placement new构造T对象
		T* obj = new (cb->storage) T(std::forward<Args>(args)...);
		// 4. 回填 ptr_ 指向刚构造好的T
		cb->ptr_ = obj;
		// 5. 设置专属删除器：只析构，不delete内存！
		cb->deleter = [](T* q) {
			q->~T();
			};
	}
	catch (...) {
		// 【异常安全补充思路】T构造抛异常：
		// CB已经构造出来了，但是T构造失败，需要销毁CB、释放内存，防止泄漏
		cb->~ControlBlock<T>();
		::operator delete(raw);
		throw;
	}
	return my::shared_ptr<T>(cb);
}
