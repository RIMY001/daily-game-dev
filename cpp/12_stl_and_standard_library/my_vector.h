#pragma once
#include <cstddef>	// size_t
#include <initializer_list>
#include <stdexcept>	// out_of_range
#include <utility>	// forward, move

namespace my {
	template <typename T>
	class vector {
    public:
        // ========== 类型定义（STL 容器规范） ==========
        using value_type = T;
        using size_type = size_t;
        using difference_type = ptrdiff_t;
        using reference = T&;
        using const_reference = const T&;
        using pointer = T*;
        using const_pointer = const T*;
        using iterator = T*;
        using const_iterator = const T*;

        // ========== 构造与析构 ==========
        vector() noexcept;
        explicit vector(size_type count);       // 构造 count 个默认初始化元素
        vector(size_type count, const T& value); // 构造 count 个 value 拷贝
        vector(std::initializer_list<T> il);    // 初始化列表构造
        ~vector();

        // ========== 拷贝控制 ==========
        vector(const vector& other);                // 深拷贝
        vector(vector&& other) noexcept;            // 移动构造，偷缓冲区
        vector& operator=(const vector& other);     // 拷贝赋值（建议用 copy-and-swap）
        vector& operator=(vector&& other) noexcept; // 移动赋值
        vector& operator=(std::initializer_list<T> il);

        // ========== 元素访问 ==========
        reference       operator[](size_type pos) noexcept;
        const_reference operator[](size_type pos) const noexcept;
        reference       at(size_type pos);
        const_reference at(size_type pos) const;

        reference       front() noexcept;
        const_reference front() const noexcept;
        reference       back() noexcept;
        const_reference back() const noexcept;

        pointer         data() noexcept;
        const_pointer   data() const noexcept;

        // ========== 迭代器 ==========
        iterator        begin() noexcept;
        const_iterator  begin() const noexcept;
        const_iterator  cbegin() const noexcept;
        iterator        end() noexcept;
        const_iterator  end() const noexcept;
        const_iterator  cend() const noexcept;

        // ========== 容量操作 ==========
        bool        empty() const noexcept;
        size_type   size() const noexcept;
        size_type   capacity() const noexcept;
        void        reserve(size_type new_cap);   // 只扩容不缩容
        void        resize(size_type new_size);   // 调整元素个数
        void        resize(size_type new_size, const T& value);
        void        shrink_to_fit();              // 释放冗余容量

        // ========== 修改操作 ==========
        void push_back(const T& value);
        void push_back(T&& value);

        template <typename... Args>
        reference emplace_back(Args&&... args);   // 原地构造，返回新元素引用

        void pop_back() noexcept;
        void clear() noexcept;                    // 销毁所有元素，保留容量
        void swap(vector& other) noexcept;        // 交换两个 vector 内部状态

    private:
        // ========== 底层三指针（经典 STL 实现方案） ==========
        T* _start;          // 内存块起始地址
        T* _finish;         // 已构造元素的尾后位置
        T* _end_of_storage; // 整个内存块的尾后位置
	};
}