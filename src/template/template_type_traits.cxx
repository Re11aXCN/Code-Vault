#ifndef _TYPE_TRAITS_H_
#define _TYPE_TRAITS_H_
#include <type_traits>
#include <utility>
#include <iostream>
#include <source_location>
#include <vector>
#include <boost/type_index.hpp>

void determine_type()
{
	struct Integer { int i = 0; };
	struct Float { float f = 0.0f; };
	struct CInteger { const int ci = 0; };
	struct RefInteger {
		int i = 0;  int& ri = i;
	};
	struct CharPointer { char* pc = nullptr; };
	int x = 0;
	std::cout << std::boolalpha;
	std::cout << std::is_integral_v<decltype(Integer::i)>
		<< std::is_floating_point_v<decltype(Float::f)>
		<< std::is_const_v<decltype(CInteger::ci)>
		<< std::is_reference_v<decltype(RefInteger::ri)>
		<< std::is_pointer_v<decltype(CharPointer::pc)>
		<< std::is_same_v<decltype(x), int>
		<< std::endl;
}

void print_type_info()
{
	int x = 10;
	decltype(x) y = 20;  // y 的类型是 int
	decltype((x)) z = x; // z 的类型是 int&
	std::cout << std::boolalpha;
	std::cout << "decltype(x) = " << typeid(decltype(x)).name() << " " << std::is_reference_v<decltype(x)> << std::endl;
	std::cout << "decltype((x)) = " << typeid(decltype((x))).name() << " " << std::is_reference_v<decltype((x))> << std::endl;
	std::cout << "Boost::decltype((x)) = "
		<< boost::typeindex::type_id_with_cvr<decltype((x))>().pretty_name()
		<< std::endl;
	std::cout << "decltype(std::declval<std::string>().size()) = " << typeid(decltype(std::declval<std::string>().size())).name() << std::endl;
	
	auto print_type = []<typename T>(T&& t) -> void {
		std::cout << std::source_location::current().function_name() << " : " << typeid(T).name() << std::endl;
	};
	print_type(std::vector<int>{});
}

void type_degrade()
{
	// 1. 数组退化为指针
	auto print_size = [](int arr[5]) {
		// 这里 arr 实际是指针，不是数组
		std::cout << "In function: "
			<< std::is_same_v<decltype(arr), int*> << "\n"; // true
		};
	int arr[5] = { 1, 2, 3, 4, 5 };
	std::cout << "In main: "
		<< std::is_same_v<decltype(arr), int[5]> << "\n"; // true

	print_size(arr); // 传入时退化为 int*

	// 2. 函数退化为指针
	auto func = []() {  };
	std::cout << "In main: "
		<< std::is_same_v<decltype(func), void()> << "\n"; // true
	auto call_func = [](void f()) {
		// 这里 f 实际是函数指针
		std::cout << "In call: "
			<< std::is_same_v<decltype(f), void(*)()> << "\n"; // true
	};
	call_func(func); // 传入时退化为 void(*)()

	// 3. 顶层 const 被忽略
	const int ci = 10;
	auto process_ci = [](int v) { v = 20; std::cout << v << std::endl; };
	process_ci(ci); // 传入时忽略顶层 const

	// 4. 模板参数推导时的类型退化
	auto deduce = []<typename T>(T t) {};
	int xX = 10;
	const int& rxX = xX;

	deduce(rxX); // T 被推导为 int（移除了 const 和引用）

	auto forward = []<typename T> (std::remove_reference_t<T>&arg) constexpr -> T&& {
		return static_cast<T&&>(arg);
	};
}
int main()
{
	determine_type();
	print_type_info();
	return 0;
}
#endif // !_TYPE_TRAITS_H_
