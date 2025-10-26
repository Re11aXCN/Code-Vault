#include "cppdemangle.h"
#include "print.h"

void func(int&& a)
{
	print(__FUNCSIG__);
}

void func(int const& a)
{
	print(__FUNCSIG__);
}

struct MyStruct
{
	int a;
	int b;
	int& getA()&
	{
		print(__FUNCSIG__);
		return a;
	}
	int const& getA() const&
	{
		print(__FUNCSIG__);
		return a;
	}
	int&& getA()&&
	{
		print(__FUNCSIG__);
		return std::move(a);
	}
	int const&& getA() const&&
	{
		print(__FUNCSIG__);
		return std::move(a);
	}
};



int main()
{
	using namespace std::literals::string_literals;
	// 纯右值除了字符串字面量以外的字面量，如42，true，nullptr
	// "hello"，"world"s 是左值
	int a = 1;
	print(cppdemangle<decltype(1)>());	// rvalue——pvalue
	print(cppdemangle<decltype(a)>());	// rvalue——pvalue
	print(cppdemangle<decltype(+a)>());	// rvalue——pvalue
	print(cppdemangle<decltype(++a)>());// lvalue
	print(cppdemangle<decltype(a += 1)>());// lvalue
	print(cppdemangle<decltype((a))>());// lvalue
	print(cppdemangle<decltype(std::move(a))>());	// rvalue——xvalue

	print(cppdemangle<decltype(("hello"))>());	// char[6] const &
	print(cppdemangle<decltype(("world"s))>()); // class std::basic_string

	print(cppdemangle<decltype(MyStruct().a)>());	// rvalue——pvalue
	print(cppdemangle<decltype((MyStruct().a))>()); // rvalue——xvalue
	print(cppdemangle<decltype(MyStruct().getA())>());// rvalue——pvalue
	print(cppdemangle<decltype((MyStruct().getA()))>());// rvalue——pvalue

	print(cppdemangle<decltype(("world"s))>());

	MyStruct m;
	print(cppdemangle<decltype(m.a)>());	// rvalue——pvalue
	print(cppdemangle<decltype((m.a))>());  // lvalue
	print(cppdemangle<decltype(std::move(m).a)>()); // rvalue——pvalue
	print(cppdemangle<decltype((std::move(m).a))>());// rvalue——xvalue
	const MyStruct s;
	print(cppdemangle<decltype(s.getA())>());// rvalue——pvalue
	print(cppdemangle<decltype(std::move(s).getA())>());// rvalue——pvalue

	func(1);			// rvalue——pvalue
	func(std::move(a)); // rvalue——xvalue
	func(a);			// lvalue
}