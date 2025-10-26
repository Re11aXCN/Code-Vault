#include <format>
#include <vector>
#include <functional>
#include <iostream>
#include <string>

using namespace std;

struct Frac {
	int a, b;
};

// �ػ�
template <>
struct std::formatter<Frac, char> {
	std::formatter<int, char> format_int;

	// parse �ȴ���^10
	constexpr typename std::basic_format_parse_context<char>::iterator
		parse(std::basic_format_parse_context<char>& pc) {
		// cout << string_view(pc.begin(), pc.end() - pc.begin()); // ���^10
		return format_int.parse(pc);
	}
	// ������ɽ���format��Frac�ػ�������
	template <class Out>
	constexpr typename std::basic_format_context<Out, char>::iterator
		format(Frac const& value, std::basic_format_context<Out, char>& fc) const {
		/* return std::format_to(fc.out(), "{}/{}", value.a, value.b); */
		format_int.format(value.a, fc);
		*fc.out() = '/';    // out���ǵ�����������д���
		return format_int.format(value.b, fc);
	}
};

// ʵ���Լ���print

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
