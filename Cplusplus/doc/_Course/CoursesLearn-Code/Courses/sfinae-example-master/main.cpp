#include <cstdio>
#include <vector>
#include <type_traits>
#include <string>
#include "print.h"

#define REQUIRES(x) std::enable_if_t<(x), int> = 0

struct mystudent {
	void dismantle() {
		printf("rm -rf stu.db\n");
	}

	void rebel(int i) {
		printf("rm -rf gench-%d\n", i);
	}
};

struct myclass {
	void dismantle() {
		printf("rm -rf course\n");
	}
};

struct myteacher {
	void rebel(int i) {
		printf("rm -rf gench-%d\n", i);
	}
};

struct myvoid {
};

template <class T, class = void>
struct has_dismantle {
	static constexpr bool value = false;
};

// std::void_t目的是为了知道decltype(std::declval<T>().dismantle())是否编译成功
// 编译成功才能够进入，得到true，否则是上述的false
template <class T>
struct has_dismantle<T, std::void_t<decltype(std::declval<T>().dismantle())>> {
	static constexpr bool value = true;
};
// 上述两个模板写法实现了多个类的特化，不需要手动写多个类/结构体特化模板

template <class T, class = void>
struct has_rebel {
	static constexpr bool value = false;
};

template <class T>
struct has_rebel<T, std::void_t<decltype(std::declval<T>().rebel(std::declval<int>()))>> {
	static constexpr bool value = true;
};

template <class T, REQUIRES(has_dismantle<T>::value)>
void gench(T t) {
	t.dismantle();
}

template <class T, REQUIRES(!has_dismantle<T>::value&& has_rebel<T>::value)>
void gench(T t) {
	for (int i = 1; i <= 4; i++) {
		t.rebel(i);
	}
}

template <class T, REQUIRES(!has_dismantle<T>::value && !has_rebel<T>::value)>
void gench(T t) {
	printf("no any method supported!\n");
}

// gench是判断类是否具有某个成员方法(区别参数的)
/*
// c++ 20写法

template <class T>
void gench(T t) {
	if constexpr (requires { t.dismantle(); })
		t.dismantle();
	else if constexpr (requires (int i) { t.rebel(i); }) // 20写法，之前写法t.rebel(std::declval<int>())
		for (int i = 1; i <= 4; i++) {
			t.rebel(i);
		}
	else
		printf("no any method supported!\n");
}

*/

int main() {
	myclass mc;
	mystudent ms;
	myteacher mt;
	myvoid mv;
	gench(mc);
	gench(ms);
	gench(mt);
	gench(mv);
	return 0;
}

/*
* #include <type_traits>
template <class T>
auto func(T t)
{
	// T 可以写 decltype(t)，但是只适用于一般的类型如int double
	// 如果是t的类型是 int const & ，那么decltype(t)则是int const &不能够与int匹配
	// 替换为std::decay_t<decltype(t)>

	// 对于t是vector使用 typename T::value_type ,
	// 什么时候加typename，如果使用了模板变量T，或者间接用了t，如std::decay_t<decltype(t[0])>
	// 原因如给个vector<int>i，那么在func模板中声明 std::decay<decltype(t[0])>::type * i
	// 我们不知道特化的::type，::type * i是一个乘法还是声明一个指针
	// 注意decay_t和decay的区别，decay_t简化了typename
	if constexpr (std::is_same_v<T,int>)    //使得编译器编译到能够t+1需要加上constexpr
		return t + 1;
	else
		return t.substr(1);
}

template <class F>
auto invoke(F f)
{
	printf("entered \n");
	// std::is_void_v<decltype(f())> 等价于 std::is_void_v<std::invoke_result_t<F>>
	// 或者std::is_void_v<decltype(F()())> ，更进一步 std::is_void_v<std::decalval<F const &>()()>
	if constexpr (std::is_void_v<decltype(f())>) //通过这样写能够保存f()的返回值然后给ret用
	{
		f();
		printf("Leaved \n");
	}
	else
	{
		auto ret = f();
		printf("Leaved \n");
		return ret;
	}
}

invoke([]() -> int
{
	return 1;
})

invoke([]() -> void
{
	return 0;
})

上述是一个invoke实现int或者void不同的响应
也可以拆分为两个invoke分别处理
requires是c++20新增，处理invoke(F f)模板函数名、参数一样重定义的情况，但实际类型不一样
template <class F>
	requires(std::is_void_v<std::invoke_result_t<F>>)
auto invoke(F f)
{
	printf("entered \n");
	f();
	printf("Leaved \n");
}

template <class F>
	requires(!std::is_void_v<std::invoke_result_t<F>>)
auto invoke(F f)
{
	printf("entered non-void \n");
	auto ret = f();
	printf("Leaved non-void \n");
	return ret;
}

// 不是c++20这样写
template <class F,
		std::enable_if_t<
			!std::is_void_v<std::invoke_result_t<F>>
		, int> = 0 >

或者宏简写
#define REQUIRES(x) std::enable_if_t<(x), int> = 0

template <class F, REQUIRES(!std::is_void_v<std::invoke_result_t<F>>) >
template <class F, REQUIRES(!std::is_void_v<decltype(F()())>) > // 先来后道不能写f()，用F()()替换

// 如果使用lambda表达式没有了构造函数那么F()()替换为std::declval<F>()()
// 更全面std::declval<F const &>()()
*/