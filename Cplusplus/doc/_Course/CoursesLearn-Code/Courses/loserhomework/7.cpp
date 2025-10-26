#include <format>
#include <vector>
#include <functional>
#include <stdexcept>
#include <iostream>
#include <string>
#include <atomic>

struct MyException : std::exception {
	const char* s;
	MyException(const char* s_) : s(s_) {
	}
	~MyException() {
	}
	const char* what() const noexcept {
		return s;
	}
};

struct Error {
	virtual std::string What() const noexcept {
		return "Error";
	}
	virtual ~Error() = default;
};

// 热hot 冷cold 代码，noexcept会把cold代码省去
// 热代码经常用，冷代码非常小概率用，且离得远
// 通勤上班热，被车撞救护车接你冷，救护车不可能在你周围天天等着你被撞，而是在医院

struct HttpError : Error {
	virtual std::string What() const noexcept override {
		return "HTTP 404 ERROR";
	}
};

void test() {
	std::string s;
	throw new MyException("信息");
	throw HttpError();
	throw MyException("信息");
	throw "sss";
}

void catcher(auto test) {
	try {
		test();
		// ...捕获任何类型的异常，不用写多个catch处理不同类型异常
	}
	catch (...) {
		puts("男主：电脑，硬盘，赶紧，rm -rf");
		// 扔出异常指针std::current_exception()
		// 需要在扔一边std::rethrow_exception
		std::rethrow_exception(std::current_exception());
	}
}

/*
void test(){
	// 如果写std::runtime_error const *,catch必须写const，不写的话，catch可以写const可以不写
	throw (std::runtime_error *)new std::runtime_error("指针")；
}
int main(){
	try
		test();
	catch (std:exception const *e){
		printf("%p\n",e);
		puts(e->what());
		delete e; // 如果使用javabean扔出异常，要进行delete
	}
}
*/

int main() {
	/*
	继承，儿子大小转父亲大小，会被砍掉
	拷贝
	exception构造一遍，虚表指针执向exception的*what()
	std::exception e = MyException("异常");
	puts(e.what());

	// 把父亲的引用指向儿子，没有发生拷贝
	std::exception const& e = MyException("异常");
	// 等价于
	MyException me  = MyException("异常");
	std::exception const& e = me;
	*/
	try {
		catcher(test);
	}
	catch (Error const& e) {
		puts(e.What().c_str());
	}
	catch (std::exception const& e) {
		puts(e.what());
	}
	catch (std::exception const* e) {
		puts(e->what());
		delete e;
	}
	catch (int i) {
		printf("%d\n", i);
	}
	catch (...) {
		printf("未知错误\n");
	}
	/*
	c++默认异常处理，只要没有捕获异常，但是你又trycatch，且有异常，但是没catch，依次调用
	std::terminate();
	std::abort();
	std::raise(SIGABRT);
	再调用操作系统，杀死进程
	*/
	return 0;
}
