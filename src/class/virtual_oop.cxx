#ifndef _VIRTUAL_OOP_H_
#define _VIRTUAL_OOP_H_
#include <iostream>
class Animal {
public:
	void print(std::string s) {
		std::cout << __FUNCTION__ << s << std::endl;
	}
};
class Dog : public Animal {
public:
	void print(std::string s) {
		std::cout << __FUNCTION__ << s << std::endl;
	}
};
class Cat : public Animal {
public:
	void print(std::string s) {
		std::cout << __FUNCTION__ << s << std::endl;
	}
};
void test_Animal(Animal& animal) {
	animal.print("XXX");
}
using Func = void(*)(void*/*this*/, std::string);
int main() {
	Dog dog;
	Cat cat;
	// 1. 未给Animal 定义virtual print
	///< 原因test_Animal只能够知道类型是Animal，只能访问Animal的print函数
	test_Animal(dog); // __cdecl Animal::print(std::string s) XXX
	test_Animal(cat); // __cdecl Animal::print(std::string s) XXX

	// 2. 给Animal 定义virtual print
	///< 原因加上virtual，将回去虚函数表中找到对于的重载函数，然后再去调用
	test_Animal(dog); // __cdecl Animal::print(std::string s) XXX
	test_Animal(cat); // __cdecl Dog::print(std::string s) XXX

	// 3. 操作虚函数表
	///< 具体为64位系统，会在头部的前8位保存一个指向虚函数表的指针
	auto vptr = reinterpret_cast<void **>(&dog);
	///< 有几个虚函数，虚函数表就存放几个指针，所有Dog的对象都指向一个虚函数表
	///< 所以主要开销不是表，而是每个对象都要存储一个指针指向虚函数表，且多了寻址开销
	auto vtable = reinterpret_cast<void **>(*vptr);

	auto print_func = reinterpret_cast<Func>(vtable[0]);
	print_func(&dog, "YYY"); // __cdecl Dog::print(std::string s) YYY
}
struct B {
	virtual void f1(int)const;
	virtual void f2();
	void f3();
};
struct C : B {
	virtual void f1(int) const override final ; //正确：f1与基类中的f1匹配
	//void f2(int) override; 	//错误：B没有形如f2(int)的函数
	//void f3() override;		//错误：f3不是虚函数
	//void f4() override;		//错误：B没有名为f4的函数
};
struct D : C {
	void f1(int) const override; //错误：final不能被重写
};
#endif // !_VIRTUAL_OOP_H_
