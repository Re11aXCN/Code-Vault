#include "other.h"
#include <vector>
#include <iostream>
#include <tuple>
#include <algorithm>

//void show(variant<int, double, string> t)
//{
//	// auto = <class T>
//	auto visitor = [&](auto arg) {
//		cout << arg << endl; // 3 times instantiated
//		};
//	if (t.index() == 0)
//		visitor(get<int>(t));
//	else if (t.index() == 1)
//		visitor(get<double>(t));
//	else if (t.index() == 2) {
//		visitor(get<string>(t));
//	}
//}


/*
// 模版自动化，可以自动适应添加的vector类型
using ObjectList = tuple< vector<int>, vector<double>, vector<string> > objects;
*/
template <class V>
struct variant_to_tuple_of_vector;

template <class ...Ts>	// 参数包
struct variant_to_tuple_of_vector<variant<Ts...>> {
	using type = tuple<vector<Ts>...>;
};

static variant_to_tuple_of_vector<Object>::type objects;

#if 0
// <>内一定是编译器常量，()内一定是运行时变量
template <size_t N, class Lambda>
void static_for(Lambda&& lambda) {
	// 递归思想实现静态编译时的for循环
	if constexpr (N > 0) {
		static_for<N - 1>(lambda);
		lambda(integral_constant<size_t, N - 1>{});
	}
}
#else
// index_sequence实现循环，而且可以打破循环更好
template <size_t N, class Lambda>
auto static_for(Lambda&& lambda) {
	return[&] <size_t ...Is> (index_sequence<Is...>) {
		return (lambda(integral_constant<size_t, Is>{}), ...);	// 类的构造函数才用{},当然可以用()，但是{}更清晰说明是类的构造函数
	}(make_index_sequence<N>{});
}

template <size_t N, class Lambda>
auto static_for_break_if_false(Lambda&& lambda) {
	return[&] <size_t ...Is> (index_sequence<Is...>) {
		return (lambda(integral_constant<size_t, Is>{}) && ...);
	}(make_index_sequence<N>{});
}

template <size_t N, class Lambda>
auto static_for_break_if_true(Lambda&& lambda) {
	return[&] <size_t ...Is> (index_sequence<Is...>) {
		return (lambda(integral_constant<size_t, Is>{}) || ...);
	}(make_index_sequence<N>{});
}
#endif

void add_object(Object o) {
	/* [&] <size_t ...Is> (index_sequence<Is...>) { */
	/*     return ((o.index() == Is && (get<Is>(objects).push_back(get<Is>(o)), false)) && ...); */
	/* }(make_index_sequence<variant_size_v<Object>>{}); */
	static_for_break_if_false<variant_size_v<Object>>([&](auto ic) {
		if (o.index() == ic) {
			get<ic>(objects).push_back(get<ic>(o));
			return false;
		}
		return true;
		});
}

void print_objects() {
	/* [&] <size_t ...Is> (index_sequence<Is...>) { */
	/*     return (ranges::for_each(get<Is>(objects), [&] (auto o) { */
	/*         return (cout << o << endl, void()); */
	/*     }), ...); */
	/* }(make_index_sequence<variant_size_v<Object>>{}); */
	static_for<variant_size_v<Object>>([&](auto ic) {
		for (auto const& o : get<ic>(objects)) {
			cout << o << endl;
		}
		/*
		for (auto const& o : get<ic>(objects)) {
			visit([] (auto const &o))
			{
				cout << o << endl
			}, o); // 同名也没关系，优先访问visit内部的o，而不是for的o，即使[&]捕获了for的o
		}
		*/
		});
}
