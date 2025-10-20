//import std;

#include <type_traits>
#include <iostream>
#include <vector>
#include <iterator>  
#include <algorithm>  
#include <array>

//#if 0
//template<class T1, class T2>
//auto add(T1 a, T2 b)// #接口实现2
//{
//	using type = std::common_type_t<typename T1::value_type, typename T2::value_type>;
//	if constexpr (std::is_same_v<typename T1::value_type, type>)
//	{
//		T1 ret;
//		auto a_iter = a.begin();
//		auto b_iter = b.begin();
//		for (; a_iter != a.end(), b_iter != b.end(); a_iter++, b_iter++) {
//			ret.emplace_back(*a_iter + *b_iter);
//		}
//		if (a_iter != a.end()) { // 处理剩下的数据
//			for (; a_iter != a.end(); a_iter++) ret.emplace_back(*a_iter);
//		}
//		if (b_iter != b.end()) { // 处理剩下的数据
//			for (; b_iter != b.end(); b_iter++) ret.emplace_back(*b_iter);
//		}
//		return ret;
//	}
//	else
//	{
//		T2 ret;
//		auto a_iter = a.begin();
//		auto b_iter = b.begin();
//		for (; a_iter != a.end(), b_iter != b.end(); a_iter++, b_iter++) {
//			ret.emplace_back(*a_iter + *b_iter);
//		}
//		if (a_iter != a.end()) { // 处理剩下的数据
//			for (; a_iter != a.end(); a_iter++) ret.emplace_back(*a_iter);
//		}
//		if (b_iter != b.end()) { // 处理剩下的数据
//			for (; b_iter != b.end(); b_iter++) ret.emplace_back(*b_iter);
//		}
//		return ret;
//	}
//
//
//}
//#endif // 0

template<class T1, class T2,
	template<typename> class Container = std::vector,
	class ResultType = std::common_type_t<typename T1::value_type, typename T2::value_type>>
	Container<ResultType> add(const T1& a, const T2& b)
{
	Container<ResultType> ret;
	auto a_it = std::begin(a), a_end = std::end(a);
	auto b_it = std::begin(b), b_end = std::end(b);

	auto a_size = std::distance(a_it, a_end);
	auto b_size = std::distance(b_it, b_end);

	auto min_size = std::min(a_size, b_size);
	std::transform(a_it, a_it + min_size, b_it, std::back_inserter(ret),
		[](auto x, auto y) { return static_cast<ResultType>(x) + static_cast<ResultType>(y); });

	if (a_size > b_size) {
		std::transform(a_it + min_size, a_end, std::back_inserter(ret),
			[](auto x) { return static_cast<ResultType>(x); });
	}
	else if (b_size > a_size) {
		std::transform(b_it + min_size, b_end, std::back_inserter(ret),
			[](auto y) { return static_cast<ResultType>(y); });
	}

	return ret;
}

template <template<typename...> class Container1, typename T1,
	template<typename...> class Container2, typename T2>
auto add1(const Container1<T1>& a, const Container2<T2>& b) {
	// std::decay_t引用和cv限定符的去除,确保类型统一, 如 int const & -> int
	Container1<std::decay_t<decltype(std::declval<T1>() + std::declval<T2>())>> ret;
	auto a_iter = a.begin();
	auto b_iter = b.begin();
	for (; a_iter != a.end() && b_iter != b.end(); ++a_iter, ++b_iter) {
		ret.emplace_back(*a_iter + *b_iter);
	}
	// Handle remaining elements in a
	for (; a_iter != a.end(); ++a_iter) {
		ret.emplace_back(*a_iter);
	}
	// Handle remaining elements in b
	for (; b_iter != b.end(); ++b_iter) {
		ret.emplace_back(*b_iter);
	}
	return ret;
}

int main()
{
	std::vector<int> a1 = { 1, 2, 3, 4 };
	std::vector<float> b1 = { 1, 2, 3 };
	auto c1 = add(a1, b1); // ok，会选择接口实现2而不是接口实现1
	auto c2 = add1(a1, b1);
	for (const auto& val : c1) std::cout << val << std::endl; // ok; 输出 2 4 6 4
	for (const auto& val : c2) std::cout << val << std::endl; // ok; 输出 2 4 6 4
}