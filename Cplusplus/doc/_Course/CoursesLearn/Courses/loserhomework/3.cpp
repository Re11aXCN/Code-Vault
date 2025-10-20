#include <format>
#include <vector>
#include <functional>
#include <iostream>
#include <string>

using namespace std;

struct Frac {
	int a, b;
};

// 特化
template <>
struct std::formatter<Frac, char> {
	std::formatter<int, char> format_int;

	// parse 先处理^10
	constexpr typename std::basic_format_parse_context<char>::iterator
		parse(std::basic_format_parse_context<char>& pc) {
		// cout << string_view(pc.begin(), pc.end() - pc.begin()); // 输出^10
		return format_int.parse(pc);
	}
	// 解析完成进行format，Frac特化的类型
	template <class Out>
	constexpr typename std::basic_format_context<Out, char>::iterator
		format(Frac const& value, std::basic_format_context<Out, char>& fc) const {
		/* return std::format_to(fc.out(), "{}/{}", value.a, value.b); */
		format_int.format(value.a, fc);
		*fc.out() = '/';    // out就是迭代器，可以写多个
		return format_int.format(value.b, fc);
	}
};

// 实现自己的print

template <class ...Args>
void print(std::string_view fmt, Args &&...args) {
	auto str = vformat(fmt, make_format_args(args...));
	std::cout << str << '\n';
}

int main() {
	Frac f{ 1,10 };
	print("({:^10})", f);
	return 1;
}
