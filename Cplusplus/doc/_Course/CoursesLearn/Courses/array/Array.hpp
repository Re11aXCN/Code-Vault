#pragma once

#include <cstddef> // size_t
#include <stdexcept> // std::runtime_error
#include <string> // std::to_string
#include <iterator> // std::reverse_iterator

// C++ 标准规定：单下划线+大写字母（_Identifier）或 双下划线+小写字母（__identifier）的标识符是保留字。理论上用户不得使用，只允许标准库和编译器使用。此处小彭老师打算只在最简单的 array 容器中严格服从一下这个规则作为演示，正经标准库里的代码都是这样的（为避免和用户定义的符号产生冲突），此后其他容器的课程都会用日常的写法，不给同学平添阅读难度

#define _LIBPENGCXX_THROW_OUT_OF_RANGE(__i, __n) throw std::runtime_error("out of range at index " + std::to_string(__i) + ", size " + std::to_string(__n))

// __assume(0) __builtin_unreachable() 就是 throw关键字
#if defined(_MSC_VER)
#define _LIBPENGCXX_UNREACHABLE() __assume(0)
#elif defined(__clang__)
#define _LIBPENGCXX_UNREACHABLE() __builtin_unreachable()
#elif defined(__GNUC__)
#define _LIBPENGCXX_UNREACHABLE() __builtin_unreachable()
#else
#define _LIBPENGCXX_UNREACHABLE() do {} while (1)
#endif

// size_t自动匹配int32位 uint32_t是4GB长度，还是long64位
template <class _Tp, size_t _N>
struct Array {
	// using定义的使用
	// Array<int32_t,42>::iterator
	// Array<int32_t,42>::value_type
	using value_type = _Tp;
	using pointer = _Tp*;
	using const_pointer = _Tp const*;
	using reference = _Tp&;
	using const_reference = _Tp const&;
	using iterator = _Tp*;
	using const_iterator = _Tp const*;
	using reverse_iterator = std::reverse_iterator<_Tp*>;
	using const_reverse_iterator = std::reverse_iterator<_Tp const*>;

	_Tp _M_elements[_N];

	_Tp& operator[](size_t __i) noexcept {
		return _M_elements[__i];
	}

	_Tp const& operator[](size_t __i) const noexcept {
		return _M_elements[__i];
	}

	_Tp& at(size_t __i) {
		// at比[]安全写，越界处理，安全永远比性能重要
		if (__i >= _N) [[__unlikely__]]
			_LIBPENGCXX_THROW_OUT_OF_RANGE(__i, _N);
			return _M_elements[__i];
	}

	_Tp const& at(size_t __i) const {
		if (__i >= _N) [[__unlikely__]]
			_LIBPENGCXX_THROW_OUT_OF_RANGE(__i, _N);
			return _M_elements[__i];
	}

	// 是否异常取决于val是否能够无异常拷贝，所以noexcept内部写个判断为false还是true确定fill是否有noexcept
	// void fill(_Tp const& __val) noexcept(noexcept(m_elements[0] = val)) {
	void fill(_Tp const& __val) noexcept(std::is_nothrow_copy_assignable_v<_Tp>) {
		for (size_t __i = 0; __i < _N; __i++) {
			_M_elements[__i] = __val;
		}
	}

	void swap(Array& __that) noexcept(std::is_nothrow_swappable_v<_Tp>) {
		for (size_t __i = 0; __i < _N; __i++) {
			std::swap(_M_elements[__i], __that._M_elements[__i]);
		}
	}

	_Tp& front() noexcept {
		return _M_elements[0];
	}

	_Tp const& front() const noexcept {
		return _M_elements[0];
	}

	_Tp& back() noexcept {
		return _M_elements[_N - 1];
	}

	_Tp const& back() const noexcept {
		return _M_elements[_N - 1];
	}

	static constexpr bool empty() noexcept {
		return false;
	}

	// 为了能够静态调用Array<int32_t,42>::size();因为不依赖于任何编译器变量
	// Array<int32_t, std::decay_t<decltype(a)>::size()> b;

	static constexpr size_t size() noexcept {
		return _N;
	}

	static constexpr size_t max_size() noexcept {
		return _N;
	}

	_Tp const* cdata() const noexcept {
		return _M_elements;
	}

	_Tp const* data() const noexcept {
		return _M_elements;
	}

	_Tp* data() noexcept {
		return _M_elements;
	}

	// https://zh.cppreference.com/w/cpp/iterator
	// LegacyRandomAccessIterator最强迭代器
	// 迭代器只需要满足 !=, ++, *符号的重载

	_Tp const* cbegin() const noexcept {
		return _M_elements;
	}

	_Tp const* cend() const noexcept {
		return _M_elements + _N;
	}

	_Tp const* begin() const noexcept {
		return _M_elements;
	}

	_Tp const* end() const noexcept {
		return _M_elements + _N;
	}

	_Tp* begin() noexcept {
		return _M_elements;
	}

	_Tp* end() noexcept {
		return _M_elements + _N;
	}

	std::reverse_iterator<_Tp const*> crbegin() const noexcept {
		return std::make_reverse_iterator(_M_elements);
	}

	std::reverse_iterator<_Tp const*> crend() const noexcept {
		return std::make_reverse_iterator(_M_elements + _N);
	}

	std::reverse_iterator<_Tp const*> rbegin() const noexcept {
		return std::make_reverse_iterator(_M_elements);
	}

	std::reverse_iterator<_Tp const*> rend() const noexcept {
		return std::make_reverse_iterator(_M_elements + _N);
	}

	std::reverse_iterator<_Tp*> rbegin() noexcept {
		return std::make_reverse_iterator(_M_elements);
	}

	std::reverse_iterator<_Tp*> rend() noexcept {
		return std::make_reverse_iterator(_M_elements + _N);
	}
};


// 特化为0的情况
template <class _Tp>
struct Array<_Tp, 0> {
	using value_type = _Tp;
	using pointer = _Tp*;
	using const_pointer = _Tp const*;
	using reference = _Tp&;
	using const_reference = _Tp const&;
	using iterator = _Tp*;
	using const_iterator = _Tp const*;
	using reverse_iterator = _Tp*;
	using const_reverse_iterator = _Tp const*;

	_Tp& operator[](size_t __i) noexcept {
		_LIBPENGCXX_UNREACHABLE();
	}

	_Tp const& operator[](size_t __i) const noexcept {
		_LIBPENGCXX_UNREACHABLE();
	}

	_Tp& at(size_t __i) {
		_LIBPENGCXX_THROW_OUT_OF_RANGE(__i, 0);
	}

	_Tp const& at(size_t __i) const {
		_LIBPENGCXX_THROW_OUT_OF_RANGE(__i, 0);
	}

	void fill(_Tp const&) noexcept {
	}

	void swap(Array&) noexcept {
	}

	_Tp& front() noexcept {
		_LIBPENGCXX_UNREACHABLE();
	}

	_Tp const& front() const noexcept {
		_LIBPENGCXX_UNREACHABLE();
	}

	_Tp& back() noexcept {
		_LIBPENGCXX_UNREACHABLE();
	}

	_Tp const& back() const noexcept {
		_LIBPENGCXX_UNREACHABLE();
	}

	static constexpr bool empty() noexcept {
		return true;
	}

	static constexpr size_t size() noexcept {
		return 0;
	}

	static constexpr size_t max_size() noexcept {
		return 0;
	}

	_Tp const* cdata() const noexcept {
		return nullptr;
	}

	_Tp const* data() const noexcept {
		return nullptr;
	}

	_Tp* data() noexcept {
		return nullptr;
	}

	_Tp const* cbegin() const noexcept {
		return nullptr;
	}

	_Tp const* cend() const noexcept {
		return nullptr;
	}

	_Tp const* begin() const noexcept {
		return nullptr;
	}

	_Tp const* end() const noexcept {
		return nullptr;
	}

	_Tp* begin() noexcept {
		return nullptr;
	}

	_Tp* end() noexcept {
		return nullptr;
	}

	_Tp const* crbegin() const noexcept {
		return nullptr;
	}

	_Tp const* crend() const noexcept {
		return nullptr;
	}

	_Tp const* rbegin() const noexcept {
		return nullptr;
	}

	_Tp const* rend() const noexcept {
		return nullptr;
	}

	_Tp* rbegin() noexcept {
		return nullptr;
	}

	_Tp* rend() noexcept {
		return nullptr;
	}
};

// Array arr{1,2,3}
template <class _Tp, class ..._Ts>
Array(_Tp, _Ts...) -> Array<_Tp, 1 + sizeof...(_Ts)>;

// 有个C语言风格的数组方法，toArray
// int ca[] = {1,2,3};
// 等价于 auto a = toArray(ca);
template <class T, size_t N>
Array<std::decay_t<T>, N> toArray(T(&arr)[N]) {
	Array<std::decay_t<T>, N> result; // 创建一个和传入数组类型相同的 Array 对象

	// 将传入的 C 语言风格数组中的元素复制到 result 中
	//for (size_t i = 0; i < N; ++i) {
	//	result[i] = arr[i];
	//}
	std::copy(std::begin(arr), std::end(arr), result.begin());
	return result;
}

