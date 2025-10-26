#pragma once

#include <cstdio>
#include <utility>
//#include <concepts>

template <class T>
struct DefaultDeleter { // 默认使用 delete 释放内存
	void operator()(T* p) const {
		delete p;
	}
};

template <class T>
struct DefaultDeleter<T[]> { // 偏特化
	void operator()(T* p) const {
		delete[] p;
	}
};

//template <>
//struct DefaultDeleter<FILE> { // 全特化
//    void operator()(FILE *p) const {
//        fclose(p);
//    }
//};

// 同标准库的 std::exchange，旧的设置为nullptr，新的接收管理资源
// p = that.p;
// that.p = nullptr;
// return *this;
template <class T, class U>
T exchange(T& dst, U&& val) {
	T tmp = std::move(dst);
	dst = std::forward<U>(val);
	return tmp;
}

template <class T, class Deleter = DefaultDeleter<T>>
struct UniquePtr {
private:
	T* m_p;

	// 定义deleter的作用，释放资源的指定（删除器的不同），fclose；delete，free

	// 为什么不进行内部定义function<void(T*)> m_deleter ，然后构造初始化，可以制定删除资源的方法
	// 比如删除文件，fclose，因为function内部也有智能指针shared，开销也大
	// 所以定义了struct DefaultDeleter

	// 不写友元，子类exchange到父类就会报错，因为T和U类型不一样
	template <class U, class UDeleter>
	friend struct UniquePtr;

public:
	// 目的是为了不用资源的时候设置为nullptr，节省资源
	// UniquePtr<FILE> fp(fopen("a.txt","r")); fp = nullptr;
	// 封装了reset()方法意思功能一致，fp.reset()，会释放资源
	UniquePtr(std::nullptr_t dummy = nullptr) { // 默认构造函数
		m_p = nullptr;
	}

	/*
	* explicit 显示构造，防止在栈上创建
	int i;
	UniquePtr<int> a = &i; // 栈上构造会有风险，离开作用域就被弹出了
	UniquePtr<int> a(&i);	// 正确写法
	*/
	explicit UniquePtr(T* p) { // 自定义构造函数
		m_p = p;
	}

	template <class U, class UDeleter, class = std::enable_if_t<std::is_convertible_v<U*, T*>>> // 没有 C++20 的写法
	// template <class U, class UDeleter> requires (std::convertible_to<U *, T *>) // 有 C++20 的写法
	UniquePtr(UniquePtr<U, UDeleter>&& that) {  // 从子类型U的智能指针转换到T类型的智能指针
		m_p = exchange(that.m_p, nullptr);
	}

	~UniquePtr() { // 析构函数
		if (m_p)
			Deleter{}(m_p);
	}

	// {{{
	UniquePtr(UniquePtr const& that) = delete; // 拷贝构造函数
	UniquePtr& operator=(UniquePtr const& that) = delete; // 拷贝赋值函数

	UniquePtr(UniquePtr&& that) { // 移动构造函数
		m_p = exchange(that.m_p, nullptr);
	}

	UniquePtr& operator=(UniquePtr&& that) { // 移动赋值函数
		/*
		移动赋值必须先free 本身的资源
		为什么：打个比方，两个file a ，b 文件对象，各持有的a.txt，b.txt
		调用了移动构造，此时如果把 a = std::move(b);移动赋值，
		那么a的资源被覆盖，但是没有进行释放，内存泄漏
		this != &that防止自己给自己赋值
		*/
		if (this != &that) [[likely]] {
			if (m_p)
				Deleter{}(m_p);
			m_p = exchange(that.m_p, nullptr);
			}
		return *this;
	}
	// }}}

	T* get() const {
		return m_p;
	}

	T* release() {
		return exchange(m_p, nullptr);
	}

	void reset(T* p = nullptr) {
		if (m_p)
			Deleter{}(m_p);
		m_p = p;
	}

	T& operator*() const {	// 注意返回引用
		return *m_p;
	}

	T* operator->() const {
		return m_p;
	}
};

template <class T, class Deleter>
struct UniquePtr<T[], Deleter> : UniquePtr<T, Deleter> {};

/*
struct Myclass{
	int a,b,c;
}
auto p = UniquePtr<Myclass>(new Myclass(42,43,44));	// 写了两次Myclass不方便
// 所以需要makeUnique，Args &&...args参数打包
auto p = makeUnique<Myclass>(42,43,44);
p->a需要重载T* operator->() const，先找到成员m_p在获取a，即使嵌套weakptr也会递归找到最小的单元成员变量
*/
// &&万能引用
template <class T, class ...Args>
UniquePtr<T> makeUnique(Args &&...args) {
	return UniquePtr<T>(new T(std::forward<Args>(args)...));
}

/*
	makeUnique<int>() // new int()，()会初始化并填入0
	makeUniqueForOverwrite<int>()// new int 初始化不填入0，效率高些
*/

template <class T>
UniquePtr<T> makeUniqueForOverwrite() {
	return UniquePtr<T>(new T);
}
