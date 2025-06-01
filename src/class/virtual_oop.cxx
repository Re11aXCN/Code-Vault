#ifndef _VIRTUAL_OOP_H_
#define _VIRTUAL_OOP_H_
#include <iostream>
class Animal {
public:
	virtual void print(std::string s) {
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
	//void f1(int) const override; //错误：final不能被重写
};
class Parent {
private:
	virtual void first_name() { std::cout << "Parent::first_name()" << std::endl; }
	void last_name() { std::cout << "Parent::last_name()" << std::endl; }
public:
	virtual void money() { std::cout << "Parent::money()" << std::endl; }
	void name() { first_name(); last_name(); }

	void height(std::string h) { std::cout << "Parent::height(std::string)" << std::endl; }
	void height(int h) { std::cout << "Parent::height(int)" << std::endl; }

	virtual void car() { std::cout << "Parent::car()" << std::endl; }
};
class Son : public Parent {
public:
	// 覆盖父类的first_name
	void first_name() /*override*/ { std::cout << "Son::first_name()" << std::endl; }
	
	void last_name() { std::cout << "Son::last_name()" << std::endl; }

	// 覆盖父类的money
	virtual void money() { std::cout << "Son::money()" << std::endl; }

	void use_parent_money() { Parent::money(); };

	void height(std::string h, int ih) { std::cout << "Son::height(std::string,int)" << std::endl; }
	void height(int ih, std::string h) { std::cout << "Son::height(int,std::string)" << std::endl; }
};
int main() {
	// 一句话总结父类声明为virtual的方法不管其可见性如何，被子类同名定义后，就是override,会调用子类方法
	// 具体实现是虚函数表
	Son son;
	son.first_name();
	son.last_name();
	son.name();	///< 这里是多态，调用父类的name()，Son::first_name() Parent::last_name()
	son.money();
	son.height("", 18); // Son::height(std::string,int)
	son.height(20, ""); // Son::height(int,std::string)
	son.use_parent_money();// Parent::money()

	std::cout << std::endl;
	Parent* parent = &son; ///< 多态
	parent->name(); ///< Son::first_name() Parent::last_name()
	parent->height(180);//Parent::height(int)
	parent->money();// Son::money()
	parent->car(); //Parent::car()

	std::cout << std::endl;
	Parent* parent2 = new Parent();
	parent2->name();// Parent::first_name() Parent::last_name()
	std::cout << std::endl;

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
	auto vptr = reinterpret_cast<void**>(&dog);
	///< 有几个虚函数，虚函数表就存放几个指针，所有Dog的对象都指向一个虚函数表
	///< 所以主要开销不是表，而是每个对象都要存储一个指针指向虚函数表，且多了寻址开销
	auto vtable = reinterpret_cast<void**>(*vptr); // 如果print不加virtual，虚表为空地址0xcccccccc
	using Func = void(*)(void*/*this*/, std::string);
	auto print_func = reinterpret_cast<Func>(vtable[0]);
	print_func(&dog, "YYY"); // __cdecl Dog::print(std::string s) YYY

}
#endif // !_VIRTUAL_OOP_H_
