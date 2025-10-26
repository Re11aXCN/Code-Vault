#include <array>
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <string>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>
#include "Array.hpp"

// C语言数组浅拷贝，会退化为指针，C++array深拷贝更安全

// concept Functor:
// Class::operator()
// void (*fp)()

// concept RandomAccessorIterator:
// *p
// p->...
// ++p
// --p
// p += n
// p -= n
// p + n
// p - n
// p[n]
// p1 - p2
// p1 != p2
// p1 < p2

template <class T, size_t N>
void iota(Array<T, N>& a) noexcept {
	T count = 0;
	// for (auto it = a.begin(), eit = a.end(); it != eit; ++it)
	for (auto& ai : a) {
		ai = count++; // a[i] = i;
	}
}

int main() {
	Array arr{ 1,2,3 };
	int ca[] = { 1, 2, 3 };
	auto a = toArray(ca); // 使用toArray将C风格数组转换为Array对象  

	// 验证结果  
	for (size_t i = 0; i < 3; ++i) {
		// 注意：这里假设Array有某种方式来访问其元素，比如operator[]  
		// 如果Array没有提供，则需要修改Array类来添加这种访问方式  
		// 假设 Array 有一个成员函数或操作符来访问元素  
		std::cout << a.data()[i] << std::endl; // 假设直接使用data成员  
	}
	for (const auto& elem : a) {
		std::cout << elem << ' ';
	}
	std::cout << '\n';
	/*
	size_t count 0;
	// for (auto it = a.begin(), eit = a.end(); it != eit; ++it)
	//等价于，是个语法糖
	for (auto &ai : a) {
		ai = count++;	//a[i] = i;
	}
	*/

	auto b = Array{ 2, 1, 0 };
	for (auto& bi : b) {
		std::cout << bi << '\n';
	}
	iota(b);
	for (auto& bi : b) {
		std::cout << bi << '\n';
	}
	std::cout << "front:" << b.front() << '\n';
	std::cout << "back:" << b.back() << '\n';
	return 0;
}
