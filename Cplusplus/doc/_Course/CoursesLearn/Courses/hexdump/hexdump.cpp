#define _CRT_SECURE_NO_WARNINGS

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <cctype>
#include <iomanip>
#include "range/v3/view/chunk.hpp"
#include "cxxopts.hpp"
#include <ranges>

// 如果是CMakeLists.txt构建的项目，不用修改项目的include配置
// 否则 C/C++ 附加包含目录加上 $(ProjectDir)hexdump\include;

/*
https://github.com/ericniebler/range-v3/blob/master/include/range/v3/view/chunk.hpp
https://github.com/jarro2783/cxxopts
*/
// c++20 
// template <std::ranges::range Range>
// void hexdump(Range const &range,std::size_t width = 16)
// 等价于
// void hexdump(auto const &range,std::size_t width = 16) requires (std::ranges:range<decltype(range)>
// 等价于
// 更先进的写法
void hexdump(std::ranges::input_range auto const& range, std::size_t width = 16) {
	// range_value_t对C语言的数组对C++vector都适用
	using T = std::ranges::range_value_t<decltype(range)>;
	// 00096e90  6d 5f 64 65 76 69 63 65  20 72 64 3b 0a 20 20 20  |m_device rd;.   |
	std::size_t addr = 0;
	std::vector<char> saved;	// 缓冲区存储一下
	for (auto chunk : range | ranges::views::chunk(width)) {
		std::cout << std::setw(8) << std::setfill('0') << std::hex << addr << ' ';
		for (auto c : chunk) {
			std::cout << ' ' << std::right << std::hex << std::setw(2 * sizeof(T)) << std::setfill('0');
			std::cout << (std::uint64_t)(std::make_unsigned_t<T>)c;
			++addr;
			if constexpr (sizeof(T) == sizeof(char) && std::convertible_to<T, char>) {
				saved.push_back(static_cast<char>(c));
			}
		}
		// 判断是否是char，是就进行ASCII转换打印
		if constexpr (sizeof(T) == sizeof(char) && std::convertible_to<T, char>) {
			if (addr % width != 0) {
				for (std::size_t i = 0; i < (width - addr % width) * 3; i++) {
					std::cout << ' ';
				}
			}
			std::cout << "  |";
			for (auto c : saved) {
				// 检测字符是否能打印，不能打印替换为.
				if (!std::isprint(c)) {
					c = '.';
				}
				std::cout << c;
			}
			std::cout << "|";
			saved.clear();
		}
		std::cout << '\n';
	}
}




template <class It>
struct Range {
	It b, e;

	It begin() const {
		return b;
	}

	It end() const {
		return e;
	}
};

template <class It>
Range(It, It) -> Range<It>;

/*
struct IstreamRange {
	std::istreambuf_iterator<char> b, e;

	IstreamRange(std::istream& is)
		: b(std::istreambuf_iterator<char>(is))
		, e(std::istreambuf_iterator<char>()) {}

	auto begin() const {
		return b;
	}

	auto end() const {
		return e;
	}
};
*/
int main(int argc, char** argv) { // 解析main参数，使用cxxopts库
#if 0
	// 小彭老师使用了自己的工具脚本 可以通过命令cmr < a.txt > b.txt 读入a.txt文件 输出十六进制到b.txt，目前没有获取该工具
	/*
	rand_device()调用rand_s 调用RtlGenRandom 真随机，消耗硬件的熵来生成的
	*/
	// 控制台输入字符输出16进制
	cxxopts::Options options("hexdump", "A command line utility for printing the hexadecimal and ASCII representation of a file.");
	options.add_options()("f,file", "Input file", cxxopts::value<std::string>()->default_value("-"));
	options.add_options()("w,width", "Number of bytes per line", cxxopts::value<std::size_t>()->default_value("16"));
	// add help
	options.add_options()("h,help", "Print usage");
	auto args = options.parse(argc, argv);
	if (args.count("help")) {
		std::cout << options.help() << '\n';
		return 0;
	}
	auto path = args["file"].as<std::string>();
	auto width = args["width"].as<std::size_t>();
	if (path == "-") {
		hexdump(IstreamRange(std::cin), width);
		return 0;
	}
	std::ifstream ifs(path);
	if (!ifs.good()) {
		std::cerr << std::strerror(errno) << ": " << path << '\n';
		return errno;
	}
	hexdump(IstreamRange(ifs), width);
#else
	std::string path = "D:\\dev\\Courses\\Test\\hexdump\\CMakeLists.txt";
	std::ifstream ifs(path);
	// 文件不存在进行异常判断，并输出什么原因
	if (!ifs.good()) {
		std::cerr << std::strerror(errno) << ": " << path << '\n';
		return errno;
	}

	std::istreambuf_iterator<char> b{ ifs }, e{};	// char遍历打开的文件，流式读写
	// 边读边输出
	hexdump(Range{ b, e });

	// 读完文件才输出，文件很大，那么很久没输出
	//std::string content(b, e);
	//hexdump(content);

#endif
	return 0;
}
