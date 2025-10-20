#include <concepts>
#include <format>
#include <vector>
#include <functional>
#include <stdexcept>
#include <iostream>
#include <string>
#include <atomic>


template<class Ty, size_t size>
struct array {
	Ty* begin() { return arr; };
	Ty* end() { return arr + size; };
	Ty arr[size];
};

// 保证{1,2,3,........}是同一个类型，保证形参包类型和Arg0一致， 
// && ...折叠表达式，将所有形参与运算看看最终是false还是true来判断类型是否一致，0和1 && 0和2 && 0和3 && .......
// sizeof...(Args) == 0 || 处理形参包为特殊0情况，为的时候是requires(true)
/* template <class Arg0, class ...Args> requires (sizeof...(Args) == 0 || (std::same_as<Arg0, Args> && ...)) */
/* array(Arg0 arg0, Args...args) -> array<Arg0, sizeof...(Args) + 1>; */

/*
// 导出类型的最大类型除了特殊情况，如 array arr{1,3.14}; // int 和 double, 应该让1被推到为double
// 使用common_type_t
*/
template <class Arg0, class ...Args>
array(Arg0 arg0, Args...args) -> array<std::common_type_t<Arg0, Args...>, sizeof...(Args) + 1>;// 计算形成包大小并加上1（Arg0）

/* template <class T> */
/* array(std::initializer_list<T> ilist) -> array<T, ilist.size()>; */
// initializer_list != initializer-list

int main() {
	::array arr{ 1, 3.14, 2.718f };
	for (const auto& i : arr) {
		std::cout << i << ' ';
	}
}
