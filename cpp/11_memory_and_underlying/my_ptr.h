namespace my {

template <typename T>
struct ControlBlock {
	long strong_cnt;
	long weak_cnt;
	T* ptr;
	explicit ControlBlock(T* p) : strong_cnt(1), weak_cnt(0), ptr(p) {}
};

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
	weak_ptr(const shared_ptr<T>&) noexcept;	//	不增加强引用计数
	shared_ptr<T> lock() const noexcept;	//	对象还在就提升，否则返回空
	bool expired() const noexcept;
	long use_count() const noexcept;
};

template <typename T, typename... Args>
shared_ptr<T> make_shared(Args&&... args);	//	一次分配（控制块 + 对象同一块内存）Demo版本：两次new，方便理解
}

template <typename T>
my::shared_ptr<T>::shared_ptr() noexcept : cb_(nullptr) {}

template <typename T>
my::shared_ptr<T>::shared_ptr(T* p) : cb_(new ControlBlock<T>(p)) {}

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

	//	释放当前持有的资源
	if (cb_) {
		cb_->strong_cnt--;
		if (cb_->strong_cnt == 0) {
			delete cb_->ptr;
			if (cb_->weak_cnt == 0) {
				delete cb_;
			}
		}
	}

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

	//	释放旧资源
	if (cb_) {
		cb_->strong_cnt--;
		if (cb_->strong_cnt == 0) {
			delete cb_->ptr;
			if (cb_->weak_cnt == 0) {
				delete cb_;
			}
		}
	}

	//	偷走对方的控制块
	cb_ = other.cb_;
	other.cb_ = nullptr;
	return *this;
}

template <typename T>
my::shared_ptr<T>::~shared_ptr() {
	//	释放自己持有的资源
	if (cb_) {
		cb_->strong_cnt--;
		if (cb_->strong_cnt == 0) {
			delete cb_->ptr;
			if (cb_->weak_cnt == 0) {
				delete cb_;
			}
		}
	}
}

template <typename T>
T& my::shared_ptr<T>::operator*() const {
	return *cb_->ptr;	//	注意返回的是引用类型，不能返回 cb_->ptr ，要解引用
}

template <typename T>
T* my::shared_ptr<T>::operator->() const {
	return cb_->ptr;
}

template <typename T>
T* my::shared_ptr<T>::get() const noexcept {
	return cb_ ? cb_->ptr : nullptr;	//	注意检查是否是空指针
}

template <typename T>
long my::shared_ptr<T>::use_count() const noexcept {
	return cb_ ? cb_->strong_cnt : 0;
}

template <typename T> 
void my::shared_ptr<T>::reset() noexcept {
	if (cb_) {
		cb_->strong_cnt--;
		if (cb_->strong_cnt == 0) {
			delete cb_->ptr;
			if (cb_->weak_cnt == 0) {
				delete cb_;
			}
		}
		cb_ = nullptr;
	}
}

template <typename T>
my::shared_ptr<T>::operator bool() const noexcept {
	return cb_ != nullptr;
}

template <typename T>
my::weak_ptr<T>::weak_ptr() noexcept : cb_(nullptr) {}

template <typename T>
my::weak_ptr<T>::weak_ptr(const my::shared_ptr<T>& other) noexcept : cb_(other.cb_) {
	if (cb_) {
		cb_->weak_cnt++;
	}
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
	T* p = new T(std::forward<Args>(args)...);
	ControlBlock<T>* cb = new ControlBlock<T>(p);
	return my::shared_ptr<T>(cb);
}