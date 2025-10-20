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

// �����CMakeLists.txt��������Ŀ�������޸���Ŀ��include����
// ���� C/C++ ���Ӱ���Ŀ¼���� $(ProjectDir)hexdump\include;

/*
https://github.com/ericniebler/range-v3/blob/master/include/range/v3/view/chunk.hpp
https://github.com/jarro2783/cxxopts
*/
// c++20 
// template <std::ranges::range Range>
// void hexdump(Range const &range,std::size_t width = 16)
// �ȼ���
// void hexdump(auto const &range,std::size_t width = 16) requires (std::ranges:range<decltype(range)>
// �ȼ���
// ���Ƚ���д��
void hexdump(std::ranges::input_range auto const& range, std::size_t width = 16) {
	// range_value_t��C���Ե������C++vector������
	using T = std::ranges::range_value_t<decltype(range)>;
	// 00096e90  6d 5f 64 65 76 69 63 65  20 72 64 3b 0a 20 20 20  |m_device rd;.   |
	std::size_t addr = 0;
	std::vector<char> saved;	// �������洢һ��
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
		// �ж��Ƿ���char���Ǿͽ���ASCIIת����ӡ
		if constexpr (sizeof(T) == sizeof(char) && std::convertible_to<T, char>) {
			if (addr % width != 0) {
				for (std::size_t i = 0; i < (width - addr % width) * 3; i++) {
					std::cout << ' ';
				}
			}
			std::cout << "  |";
			for (auto c : saved) {
				// ����ַ��Ƿ��ܴ�ӡ�����ܴ�ӡ�滻Ϊ.
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
int main(int argc, char** argv) { // ����main������ʹ��cxxopts��
#if 0
	// С����ʦʹ�����Լ��Ĺ��߽ű� ����ͨ������cmr < a.txt > b.txt ����a.txt�ļ� ���ʮ�����Ƶ�b.txt��Ŀǰû�л�ȡ�ù���
	/*
	rand_device()����rand_s ����RtlGenRandom �����������Ӳ�����������ɵ�
	*/
	// ����̨�����ַ����16����
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
	// �ļ������ڽ����쳣�жϣ������ʲôԭ��
	if (!ifs.good()) {
		std::cerr << std::strerror(errno) << ": " << path << '\n';
		return errno;
	}

	std::istreambuf_iterator<char> b{ ifs }, e{};	// char�����򿪵��ļ�����ʽ��д
	// �߶������
	hexdump(Range{ b, e });

	// �����ļ���������ļ��ܴ���ô�ܾ�û���
	//std::string content(b, e);
	//hexdump(content);

#endif
	return 0;
}
