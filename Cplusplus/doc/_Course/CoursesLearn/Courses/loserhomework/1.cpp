#include <vector>
#include <functional>
#include <iostream>
#include <string>

using namespace std;
template <class T>
inline constexpr bool false_v = false;

template <class T, class Func>
auto& operator|(vector<T>& vec, Func const& func) {
	// 防止一些不应该的运算符操作，如vec | f2 | f | 1; //数字不可以调用func
	if constexpr (std::is_invocable_v<Func, T&>)
		for (auto& element : vec) {
			func(element);
		}
	else {
		printf("error:a mao a gou dou lai le shi ba?\n");
		static_assert(false_v<Func>, i"amaoagoudoulai");//不写false， false_v<Func>懒惰/延迟报错
	}
	return vec;
}

//template <class T, class Func> requires (std::invocable<Func, T&>
// 等价于
//template <class T, std::invocable<T&> Func>
// 等价于
template <class T>
auto& operator|(vector<T>& vec, std::invocable<T&> auto const& func) {
	for (auto& element : vec) {
		func(element);
	}
	return vec;
}

// 标准库不支持字符串和整数相加需要重载

/*
// 得支持两种+
auto operator+(string s,int i){
	return s + to_string(i);
}
auto operator+(int i,string s){
	return to_string(i) + s;
}
*/

int main() {
	// CTAD: Class template argument deduction，自动推导出了<>模板参数类型
	// 所以不写vector<int>，function<void(int const &)>
	std::vector vec{ 1, 2, 3 };
	std::function f{ [](const int& i) {std::cout << i << ' '; } };
	auto f2 = [](int& i) {i *= i; };
	// 初始化，进行元素操作，打印元素
	vec | f2 | f;
}
