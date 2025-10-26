#include <format>
#include <vector>
#include <functional>
#include <iostream>
#include <string>

using namespace std;

// 自定义字面量
auto operator""_f(const char* s, size_t len) {
	auto fmt = string(s, len);
	// 引用捕获会内存泄漏
	// ...args在前定义参数包，args...在后展开参数包
	// make_format_args(args...)，展开make_format_args参数
	// make_format_args(args)... ，展开make_format_args(0),make_format_args(1).........
	return [=](auto &&...args) {
		// vformat效率低于format，但是支持的参数内容多
		return vformat(fmt, make_format_args(args...));
		};
}

string operator""_mystr(const char* s, size_t len) {
	return string(s, len);
}

int main() {
	// 输出4 5，{}填写的数字是指定顺序
	std::cout << format("乐：{1}{0}*\n", 5, 4);

	// 返回lambda，先将"乐 :{} *\n"_f转为auto lambda = "乐 :{} *\n"_f，采用lambda调用5，lambda(5)

	std::cout << "乐 :{} *\n"_f(5);
	std::cout << "乐 :{0} {0} *\n"_f(5);
	std::cout << "乐 :{:b} *\n"_f(0b01010101);
	std::cout << "{:*<10}"_f("卢瑟");
	std::cout << '\n';
	int n{};
	std::cin >> n;
	std::cout << "π：{:.{}f}\n"_f(std::numbers::pi_v<double>, n);
	return 1;
}
