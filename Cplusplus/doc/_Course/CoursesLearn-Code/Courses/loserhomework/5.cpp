#include <format>
#include <vector>
#include <functional>
#include <stdexcept>
#include <iostream>
#include <string>

using namespace std;

template <class F, class ...Args>
struct scope_guard { // scope_guard<F, int, double>
	F f;
	std::tuple<Args...> args;
	scope_guard(F f_, Args ...args_) : f(std::move(f_)), args(std::move(args_)...) {
	}
	~scope_guard() {
		std::apply(std::move(f), std::move(args)); // std::invoke(f, get<0>(args), get<1>(args), ...);
	}
};

// 自动推导类型，C++17起，要不然，需要手动decltype，scope_guard<decltype(f)>a{f};
// c++20 甚至不用写以下的代码，对于只有template <class F>构造函数含有F，编译器会自动推导F类型
template <class F, class ...Args>
scope_guard(F f, Args ...args) -> scope_guard<F, Args...>;

/*
C++中在之前只要return了之后就会进行提前析构，或者离开}
 FILE *fp = fopen("CMakeLists.txt", "r");是C语言的东西
 我们定义一个guard

 因为栈的设计，谁最后构造谁就最先析构
*/
int xmain() {
	puts("进入 main");
	{
		puts("进入局部");
		puts("打开文件");
		FILE* fp = fopen("CMakeLists.txt", "r");
		printf("打开的文件是：%p\n", fp);
		struct Test {
			void f(FILE* fp, int& i) const {
				puts("Test被调用了！");
				printf("关闭文件：%p %d\n", fp, i);
				fclose(fp);
			}
		};
		int i = 42;
		/* auto membf = &Test::f; */
		/* auto membv = &Test::mb; */
		/* Test t; */
		/*
		* invoke识别成员变量函数？
		std::invoke(membf,t,fp,i);//(t.*membf)(fp,i)
		std::invoke (pembv,t);//t.*membv
		*/
		/* (t.*membf)(fp, i); */
		// void (Test::*)(FILE *, int &)
		// void (*)(FILE *, int &)
		scope_guard a{ &Test::f, Test{}, fp, std::ref(i) };
		throw std::runtime_error("自定义异常信息");
		if (1) {
			puts("提前返回");
			return 2;
		}
		puts("离开局部");
	} // 析构
	puts("离开 main");
	return 1;
}

int main() {
	try {
		xmain();
	}
	catch (std::exception const& e) {
		puts(e.what());
	}
}
