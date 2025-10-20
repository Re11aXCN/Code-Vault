#include "Function.hpp"
#include "print.h"

// C语言函数指针，策略模式
struct S
{
#ifdef POINT
	int* x;
	int* y;
#else
	int& x;
	int& y;
#endif // POINT

};

void fun_hello(void* arg)
{
	auto a = (S*)arg;
	//printf("hello%d", *(int*)arg);
#ifdef POINT
	printf("hello %d %d", *a->x, *a->y);
#else
	printf("hello %d %d\n", a->x, a->y);
#endif // POINT
}

typedef void (*pfunc_t)(void* arg);

void repeat(pfunc_t func, void* arg)
{
	func(arg);
}

// C++lambda表达式简化逻辑
struct S2
{
	int x;
	int y;
	//void call() //持有一个成员方法调用
	//{
	//	printf("hello %d %d", x, y);
	//}
	void operator()(int i) const
	{
		// x = 1;
		printf("hello %d %d\n", x + i, y);
	}
};

/*
模板无法分离声明和定义，一个文件搞声明，另一个文件搞定义实现不行，索引为什么有个hpp文件
这时候有需求想要存在结构体内部，这时候就需要std::function<void(int)>不过要指定类型
*/
template <class Fn>
void repeat2(Fn const& func)
{
	// func.call(); // call不通用，参数需要进行多次写
	// 仿函数
	func(1);
}

// 实现简易的std::function<void(int)>
void repeat3(Function<void(int)> const& func)
{
	// func.call(); // call不通用，参数需要进行多次写
	// 仿函数
	func(1);
}

int main()
{
	//int x = 1;
	//repeat(fun_hello, NULL);
	//repeat(fun_hello, &x);

#ifdef POINT
	S s;
	int x = 1, y = 2;
	s.x = &x, s.y = &y;
#else
	int x = 1, y = 2;
	S s{ x,y };
#endif // POINT
	repeat(fun_hello, &s);

	S2 s2{ x,y };
	repeat2(s2);

	// lambda就是一个语法糖，简化了写结构体的仿函数，仿函数默认为const
	repeat2([&x, &y](int i) // 只写一个&，内部用到什么变量就帮你捕获什么
		{
			printf("hello %d %d\n", x + i, y);
		});
	//repeat2([x, y]()
	//	{
	//		// GCC=会报错，原因是因为结构体内的operator()是const不能够修改，要加mutable
	//		// msvc报错x，需要左值，加上mutable也能够修复
	//		// 意思是一样的，mutable生成的结构体把仿函数的const去掉
	//		x = 1;	
	//		printf("hello %d %d\n", x, y);
	//	});

	repeat3([&x, &y](int i) // 只写一个&，内部用到什么变量就帮你捕获什么
		{
			printf("hello %d %d\n", x + i, y);
		});
}

