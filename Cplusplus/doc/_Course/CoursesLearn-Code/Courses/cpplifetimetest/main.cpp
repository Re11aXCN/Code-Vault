#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include "cppdemangle.h"

struct Test {
	char buf[1024];
	std::string name;
	explicit Test(std::string name = "") : name(std::move(name)) {
		printf("Test() %s\n", name.c_str());
	}
	Test(Test&& that) noexcept : name(that.name) {
		// that -> this	为什么不是const，因为需要修改原来的为nullptr
		that.name = "null";
		name += "_move";
		printf("Test(Test &&) %s\n", name.c_str());
	}
	Test& operator=(Test&& that) noexcept {
		name += "_move";
		printf("Test &operator=(Test &&) %s\n", name.c_str());
		name = that.name;
		return *this;
	}
	Test(Test const& that) : name(that.name) {	// 拷贝构造和explicit定义的哪个构造一样，只不过是const&而已
		name += "_copy";
		printf("Test(Test const &) %s\n", name.c_str());
	}
	Test& operator=(Test const& that) {
		name = that.name;
		name += "_copy";
		printf("Test &operator=(Test const &) %s\n", name.c_str());
		return *this;
	}
	~Test() noexcept {
		printf("~Test() %s\n", name.c_str());
	}
};
#if 0
/*
void func() {
	int *p = new int;
	int *p2 = p;
	delete p;
	delete p2;  // 房子删两遍double free

	// 引出unique_ptr 删除了拷贝/赋值构造
	std::unique_ptr<int> p(new int);
	// 转移所有权
	std::unique_ptr<int> p2 = std::move(p); // 等价于(std::unique<int> &&)(p)，static_cast<std::unique<int> &&)>(p)
	// 自己转给自己不做任何操作，因为需要先把move的内容内存删除，自己给自己这不行
	p2 = std::move(p2);

	std::shared_ptr<int> p(new int); // 引用计数，会递归析构，计数器（原子变量）为0的时候删除内存
	std::weak_ptr<int> wp = p;	// 幽灵住进房子，不计人数，防止struct内变量双重引用，永远释放不了
	p.reset(); // 清空

}// 出了}删除的是指针，但是指针指向的内存没有释放，delete作用就是释放内存
 // 类比房产证和房子，房产证没了，但是房子还在
*/

/*
void func(Test t) {
	Test t2("t");
	puts("func");
} //t2生命周期

void funcref(Test const & t) {
	puts("func");
}

void funcmove(Test & t) {
	puts("func");
}
main()
{
	int i = 0x7fffffff;
	int k = (i+i)/2; // 没/开优化，debug:-2, release:0x7fffffff，建议改为无符号整数
	int k = (int)((uint32_t)i+i)/2;

	// 参数t的生命周期是在分号;之后 相当于 {func(Test("t")), puts("===="); }
	func(Test("t")), puts("====");

	// 先调用构造，在调用拷贝构造，再析构拷贝构造，再析构构造
	Test t("t");
	func(t), puts("====");  // 等价于{Test t_copy(t); funcref(t_copy), puts("====");

	Test t("t");
	func(std::move(t)), puts("====");  // 等价于{Test t_move(std::move(t)); funcmove(t_move), puts("===="); }
	// 注意move后t的资源就为空了，如果出了}相当于delete nullptr，对于一些编译器支持，但是有些编译器不支持
	// 移动构造不要想得太神圣，也是个构造，出了}也要析构
}
*/

/*
std:string const &func(std::string const &s) {
	return s;
}

main() {
// 内存泄露，ret想要延长生命周期，func返回了const& ,但是func(std::string("hello"))出了";"就被销毁
// 此时ret还持有 构造出来的字符串显然不对
	auto ret = func(std::string("hello"));
}

*/

/*
void func(std::unique_ptr<Test>t) {
	puts ("func");
}
int main(){
	// 一次构造一次析构，没有调用移动
	// 理解智能指针，创建了一个指针指向一块资源，房产证和房子，
	// move只是将房产证转移，但是房子不移动，因为移动开销大
	std::unique_ptr<Test>t(new Test("t"));
	func(std::move(t)):
	// Test t("t");	// 构造
	// Test t2 = t; // 拷贝构造，等价于Test t2(t)，因为新定义了个变量，如果加了explicit就不能 = 写，需Test t2(t)
	// t2 = t;		// 拷贝赋值，拷贝赋值不会进行析构，因为没有创建新的对象
	// ================================================
	// 重点，只能够隐式转换一次
	Test t("t"); // ok，隐式转换一次，const char 转string
	Test t = "t"; // error，隐式转换两次，隐式转换构造后再隐式转换string不行
	// ================================================
}

*/

/*
万能引用
void func(Test const &t)
{
	puts("Test const &t");
}
void func2(Test &&t)
{
	// 为什么外面移动了，函数内部也需要移动
	// 因为&&t 加了括号会退化为 &
	using T = decltype(t); // 左值
	using T = decltype((t));// 右值
	func(std::move(t)); // func（）括号！
}
template <typename T>
void func3(T &&t){	// const &会把 &&踢掉，万能引用
	printf("T %s\n",cppdemangle<T>().c_str());
	// func(t); // func(std::move(t));
	#if 0
	if constexpr (std:is_lvalue_reference_v<T>){
		func(t);
	}
	else {
		func(std:move(t));
	}
	#endif
	// 为什么需要T，因为加了(t)，始终是 const &  / &,得不到&&
	// T 等价于 decltype(t),注意这个()不是我们想的括号会退化
	func(std::forward<T>(t));
}

template <typename ...T>
void func3(T &&...t){	// 增加参数包，能够适配多个参数，参数数量不同
	func(std::forward<T>(t)...);
}

main()
{
	Test t("t");
	func2(std::move(t));

	const Test t("t");
	func3(t);		// const &
	Test t("t");
	func3(t);		// &

	Test t("t");
	func3(std::move(t));// &, func3内部需要再次加std::move(t)才行 变为 &&

	// 但是又要兼容const &、&、&&，引出了std::forward
}

*/


#endif

/* void func(Test t) { */
/*     puts("func"); */
/* } */
/*  */
/* void funcref(Test &t) { */
/*     puts("func"); */
/* } */

/* std::string const &func(std::string const &s) { */
/*     return s; */
/* } */

/* void func(std::unique_ptr<Test> t) { */
/*     puts("func"); */
/* } */

// 主类型 std::string
// 弱引用类型 std::string_view、const char *
// 主类型 std::vector<T>
// 弱引用类型 std::span<T>、(T *, size_t)
// 主类型 std::unique_ptr<T>
// 弱引用类型 T *
// 主类型 std::shared_ptr<T>
// 弱引用类型 std::weak_ptr<T>、T *

std::vector<std::function<void()>> g_funcs;

void func(auto f) {
	g_funcs.emplace_back(std::move(f));
}

void bao() {
	for (auto f : g_funcs) {
		f();
	}
}

// int* g_i = 0;

void foo() {
	// int i = 1;
	// g_i = &i; // i在lambda结束就死掉了
	std::shared_ptr<int> i(new int(1));	// 使用一次i就++ 用shared_ptr，不捕获&因为会bao
	// 所以捕获值，而不是引用，如果引用死掉了，就bao
	func([=] {
		printf("1i = %d\n", (*i)++);
		});
	func([=] {
		printf("2i = %d\n", (*i)++);
		});
}

int main() {
	foo();
	bao();
	//print(*g_i); g_i使用死掉的i内存泄露了
}

#if 0
std::string split(std::string s, char c) {
	auto pos = s.find(c);
	if (pos == std::string::npos) {
		return s;
	}
	return s.substr(0, pos);
}

void func(Test&& t) {
	puts("func(Test &&t)");
}

void func(Test const& t) {
	puts("func(Test const &t)");
}

void func(int i, int j) {
	puts("func(int i, int j)");
}

#define FWD(x) std::forward<decltype(x)>(x)
template <typename ...T>
void func2(T &&...t) {
	func(FWD(t)...);
}

int main() {
	Test t("t");
	func2(std::move(t));
	func2(t);
	func2(3, 4);
	/* t = t2; */
	/* Test t2(std::move(t)); */
	/* t2 = std::move(t); */
	/* std::unique_ptr<Test> t(new Test("t")); */
	/* func(std::move(t)); */
	/* auto ret = func(std::string("hello")); */
	/* printf("%s\n", ret.c_str()); */
	/* Test t("t"); */
	/* func(std::move(t)), puts("==="); */
	/* { Test t_move(std::move(t)); funcref(t_move), puts("==="); } */
	/* puts("---"); */
	return 0;
}
#endif
