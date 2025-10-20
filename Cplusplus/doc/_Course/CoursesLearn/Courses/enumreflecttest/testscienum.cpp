#include <iostream>
#include <string>
#include "scienum.h"
#define Log(x) (std::cout << "File: " \
<< __FILE__ \
<< ", Function: " \
<< __func__ \
<< "(), " \
<< ", PrettyFunction: " \
<< __FUNCSIG__ \
<< "(), Line: " \
<< __LINE__ \
<< std::endl)
// __func__和__FUNCTION__ 类似，__PRETTY_FUNCTION__ 是GCC编译器的宏
enum Color {
	RED, GREEN, BLUE, YELLOW,
};
template<class T>
void test(const T& X)
{
	Log(X);
}
void test(const char* str)
{
	Log(str);
}
void test(const std::string& str)
{
	Log(str);
}
void test()
{

}

template <class T>
std::string get_type_name()
{
	std::string str = __FUNCSIG__;
	auto pos = str.find_last_of("<") + 1;
	auto pos2 = str.find_first_of(">", pos);
	return str.substr(pos, pos2 - pos);
}
// 模版有类型、和整数<class T, T N>，枚举也是整数
// 作用类似反射把枚举类型的名字取出来
template <class T, T N>
std::string get_int_name()
{
	std::string str = __FUNCSIG__;
	auto pos = str.find_last_of("<") + 1;
	auto pos2 = str.find_first_of(">", pos);
	return str.substr(pos, pos2 - pos);
}

template <int N>
struct integral_constant
{
	static constexpr int value = N;
};

template <int Beg, int End, class F>
void static_for_test(F const& func)
{
	if constexpr (Beg == End)
	{
		return;
	}
	else
	{
		// func作用每次都会进行实例化，我们把lambda表达式传进来
		func(integral_constant<Beg>()); //可以调用标准库的std::integral_constant
		static_for_test<Beg + 1, End>(func);
	}
}

template <class T>
std::string get_int_name_dynamic(T n)
{
#if 0
	// 转发给get_int_name，每个T N都会生成一个实例
	if (n == (T)1) return get_int_name<T, (T)1>();
	if (n == (T)2) return get_int_name<T, (T)2>();
	if (n == (T)3) return get_int_name<T, (T)3>();

	// 优化，但是for是运行时调用的，所以需要进行再次改动变为编译时的for循环
	for (int i = 0; i < 3; i++)
	{
		if (n == (T)i) return get_int_name<T, (T)i>();
	}
#endif
	// 通过static_for_test进行实例化，通过lambda表达式传入进行每次实例化
	std::string ret;
	// 为什么写auto不写int，因为如果是int，func(Beg), <T, (T)i>内只能是静态变量，int i传一次没问题
	// 但是传第二次func(Beg)的Beg就变为了动态变量了
	// static_for_test<0, 256>([&] <int i>() , func<Beg>()这是C++20支持的写法，低于版本不支持
	// auto 相当于 <class T>(T i)
	static_for_test<0, 256>([&](auto i)
		{
			//constexpr auto i = ic.value;
			//if (n == (T)i) return get_int_name<T, (T)i>(); //注意return 不能够返回到外面
			if (n == (T)i.value) ret = get_int_name<T, (T)i.value>();
		});
	return ret;
}

int main1() {
	enum ColorTest {
		RED, GREEN, BULE
	};
	std::cout << get_int_name<ColorTest, (ColorTest)2>() << std::endl;
	//ColorTest c = RED; //运行时的调用
	//std::cout << get_int_name<ColorTest, c>() << std::endl; // 模版在运行之前，所以报错
	// 正确写法 
	constexpr ColorTest c = RED;
	std::cout << get_int_name<ColorTest, c>() << std::endl;

	// 但是有时候又有需求需要运行时获取，怎么办？
	ColorTest c2 = GREEN;
	std::cout << get_int_name_dynamic(c2) << std::endl;

	std::cout << get_type_name<uint32_t>() << std::endl;
	test("Hello");
	test(3);
	std::cout << scienum::get_enum_name(YELLOW) << std::endl; // 正向
	std::cout << scienum::enum_from_name<Color>("YELLOW") << std::endl;// 逆向
}
