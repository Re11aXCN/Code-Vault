#include "minilog.h"

// 模版和类成员函数自带inline关键字 不显示而已
// 防止.h多次展开编译器报错重命名，编译器不知道链接哪一个，inline作用就是能够使得名字多次使用
// 让符号不持有单一定义
// __attribute__((always_inline))才是真正的优化“内联”
// static 定义的变量，只要每个cpp引用了.h文件，就都会有一个这个变量，且不共享！！！
// 即 a.cpp修改变量，b.cpp文件变量不更新
// c++17可以给变量设置inline，如 inline log_level g_max_level 使得全局共享
// .cpp文件变量都指向头文件的那一个变量

// consteval只能编译器调用，constexpr可以编译器调用可以运行时调用
using namespace std::literals::string_literals;
int main() {
	// 系统设计的原因，std::getenv先调用，所以环境变量设置的日志路径或者日志等级会被覆盖
	minilog::set_log_file("test_log.txt");
	minilog::set_log_level(minilog::log_level::debug);
	minilog::log_trace("This is a trace message.");
	minilog::log_info("hello, the answer is {}", 42);
	minilog::log_critical("this is right-aligned [{:>+10.04f}]", 3.14);

	minilog::log_warn("good job, {1:.5s} for making {0}", "minilog", "archibate");
	minilog::set_log_level(minilog::log_level::trace); // default log level is info

	int my_variable = 42;
	MINILOG_P(my_variable); // shown when log level lower than debug

	minilog::log_trace("below is the color show :)");
#define _FUNCTION(name) minilog::log_##name(#name);
	MINILOG_FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION

		return 0;
}
