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

// C��������ǳ���������˻�Ϊָ�룬C++array�������ȫ

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
	auto a = toArray(ca); // ʹ��toArray��C�������ת��ΪArray����  

	// ��֤���  
	for (size_t i = 0; i < 3; ++i) {
		// ע�⣺�������Array��ĳ�ַ�ʽ��������Ԫ�أ�����operator[]  
		// ���Arrayû���ṩ������Ҫ�޸�Array����������ַ��ʷ�ʽ  
		// ���� Array ��һ����Ա�����������������Ԫ��  
		std::cout << a.data()[i] << std::endl; // ����ֱ��ʹ��data��Ա  
	}
	for (const auto& elem : a) {
		std::cout << elem << ' ';
	}
	std::cout << '\n';
	/*
	size_t count 0;
	// for (auto it = a.begin(), eit = a.end(); it != eit; ++it)
	//�ȼ��ڣ��Ǹ��﷨��
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
