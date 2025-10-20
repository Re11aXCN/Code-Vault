#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <format>
#include <source_location>
#include <chrono>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <windows.h>

namespace minilog {

	/*
	void log(std::string msg, std::source_location loc = std::source_location::current())
	{
		std::cout << loc.file_name() << ":" << loc.line() << " [Info] " << msg << '\n';
	}

	main()
	{
		log("hello");   //通过函数给默认赋值简化写法，即使在函数内默认赋值了，也是在调用的地方赋值
	}
	*/

	// 日志等级
	// 用enum class 防止枚举的名字与方法的名字相同导致编译器提示重命名
#define MINILOG_FOREACH_LOG_LEVEL(f) \
    f(trace) \
    f(debug) \
    f(info) \
    f(critical) \
    f(warn) \
    f(error) \
    f(fatal)

	enum class log_level : std::uint8_t //指定基类性小一点 unsigned char
	{
#define _FUNCTION(name) name,
		MINILOG_FOREACH_LOG_LEVEL(_FUNCTION)// 宏定义了日志等级
#undef _FUNCTION	// 取消_FUNCTION定义
	};

	namespace details {
		// 设置颜色
#if defined(__linux__) || defined(__APPLE__)
		inline constexpr char k_level_ansi_colors[(std::uint8_t)log_level::fatal + 1][8] = {
			"\E[37m",   // trace - white
			"\E[35m",   // debug - purple
			"\E[32m",   // info - green
			"\E[34m",   // critical - blue
			"\E[33m",   // warn - yellow
			"\E[31m",   // error - red
			"\E[31;1m", // fatal - red (bold)
		};
		inline constexpr char k_reset_ansi_color[4] = "\E[m";
#define _MINILOG_IF_HAS_ANSI_COLORS(x) x
#elif defined(_WIN32)
		inline constexpr int k_level_colors[] = {
			FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN,  // trace - white
			FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE,                     // debug - purple
			FOREGROUND_INTENSITY | FOREGROUND_GREEN,                                     // info - green
			FOREGROUND_INTENSITY | FOREGROUND_BLUE,                                      // critical - blue
			FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,                     // warn - yellow
			FOREGROUND_INTENSITY | FOREGROUND_RED,                                        // error - red
			FOREGROUND_RED,                                        // fatal - red (high intensity)
		};

		inline void set_console_color(log_level lev) {
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, k_level_colors[(int)lev]);
		}

		inline void reset_console_color() {
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}
#define _MINILOG_IF_HAS_ANSI_COLORS(x)
#else
#define _MINILOG_IF_HAS_ANSI_COLORS(x)
		inline constexpr char k_level_ansi_colors[(std::uint8_t)log_level::fatal + 1][1] = {
			"",
			"",
			"",
			"",
			"",
			"",
			"",
		};
		inline constexpr char k_reset_ansi_color[1] = "";
#endif

		inline std::string log_level_name(log_level lev) {
			switch (lev) {
#define _FUNCTION(name) case log_level::name: return #name;
				MINILOG_FOREACH_LOG_LEVEL(_FUNCTION)	// 嵌套宏，使用宏展开日志等级case的情况方便
#undef _FUNCTION
			}
			return "unknown";
		}

		inline log_level log_level_from_name(std::string lev) {
#define _FUNCTION(name) if (lev == #name) return log_level::name;
			MINILOG_FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION
				return log_level::info;
		}

		template <class T>
		struct with_source_location {   // 包装隐式构造std::format_string<Args...>>fmt
		private:
			T inner;
			std::source_location loc;

		public:
			template <class U> requires std::constructible_from<T, U>
			consteval with_source_location(U&& inner, std::source_location loc = std::source_location::current())
				: inner(std::forward<U>(inner)), loc(std::move(loc)) {}
			constexpr T const& format() const { return inner; }
			constexpr std::source_location const& location() const { return loc; }
		};

		/*
		template <typename...Args>
		void log_info(with_source_location<std::format_string<Args...>>fmt, Args &&...args)
		{
			auto const& loc = fmt.Location();
			std::cout < loc.file_name() << "" < Loc.line() << "[Info]" < std : vformat(fmt.format().get(), std:make_format_args(args...)) << '\n';
		}
		*/
		/*
		template <typename... Args>
		void log_info(std::string_view format_str, Args&&... args) {
			std::string log_msg = std::vformat(format_str, std::make_format_args(std::forward<Args>(args)...));
			// 这里添加输出日志的代码
		}
		*/

		// 属性->配置属性->调试->环境->MINILOG_LEVEL=warn  注意没有逗号
		inline log_level g_max_level = []() -> log_level {
			if (auto lev = std::getenv("MINILOG_LEVEL")) {
				return details::log_level_from_name(lev);
			}
			return log_level::info;
			} ();

		// MINILOG_LEVEL=a.log
		inline std::ofstream g_log_file = []() -> std::ofstream {
			if (auto path = std::getenv("MINILOG_FILE")) {
				return std::ofstream(path, std::ios::app); // 追加读写
			}
			return std::ofstream();
			} ();

		inline void output_log(log_level lev, std::string msg, std::source_location const& loc) {
			std::chrono::zoned_time now{ std::chrono::current_zone(), std::chrono::system_clock::now() };
			msg = std::format("{} {}:{} [{}] {}", now, loc.file_name(), loc.line(), details::log_level_name(lev), msg);
			if (g_log_file) {
				g_log_file << msg + '\n';
			}
			if (lev >= g_max_level) {
#ifdef _WIN32
				set_console_color(lev);
				std::cout << msg << std::endl;
				reset_console_color();
#else
				std::cout << _MINILOG_IF_HAS_ANSI_COLORS(k_level_ansi_colors[(std::uint8_t)lev] + )
					msg _MINILOG_IF_HAS_ANSI_COLORS(+k_reset_ansi_color) + '\n';
#endif
			}
		}


	}

	inline void set_log_file(std::string path) {
		details::g_log_file = std::ofstream(path, std::ios::app);
	}

	inline void set_log_level(log_level lev) {
		details::g_max_level = lev;
	}

	template <typename... Args>
	void generic_log(log_level lev, details::with_source_location<std::format_string<Args...>> fmt, Args &&...args) {
		auto const& loc = fmt.location();
		auto msg = std::vformat(fmt.format().get(), std::make_format_args(args...));
		details::output_log(lev, std::move(msg), loc);
	}


	// 使用宏将
	// generic_log(log_level::debug, "n={}", 45)
	// 简化为
	// log_debug("n={}", 45)
#define _FUNCTION(name) \
template <typename... Args> \
void log_##name(details::with_source_location<std::format_string<Args...>> fmt, Args &&...args) { \
    return generic_log(log_level::name, std::move(fmt), std::forward<Args>(args)...); \
}
	MINILOG_FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION

		// C语言特性 "n" "={}"有个空格会进行拼接为"n={}"
		// log_debug("n={}", 45) 进一步封装 MINILOG_P(45)
		// 用到命名空间的宏，参数都加上::防止冲突，::minilog
#define MINILOG_P(x) ::minilog::log_debug(#x "={}", x)

}
