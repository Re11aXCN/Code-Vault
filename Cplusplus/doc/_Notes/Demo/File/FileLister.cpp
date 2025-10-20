#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <regex>
#include <optional>

namespace fs = std::filesystem;

class FileLister {
public:
	void parseArguments(int argc, char* argv[]) {
		for (int i = 1; i < argc; ++i) {
			std::string arg = argv[i];

			if (arg == "--input" && i + 1 < argc) {
				inputPath = fs::path(argv[++i]);
			}
			else if (arg == "--output" && i + 1 < argc) {
				outputPath = fs::path(argv[++i]);
			}
			else if (arg == "--recursive" && i + 1 < argc) {
				maxDepth = std::stoi(argv[++i]);
			}
			else if (arg == "--pattern" && i + 1 < argc) {
				filePattern = argv[++i];
				// 转换为小写以便大小写不敏感比较
				std::transform(filePattern.begin(), filePattern.end(), filePattern.begin(),
					[](unsigned char c) { return std::tolower(c); });
			}
			else if (arg == "--dir-name" && i + 1 < argc) {
				dirPattern = argv[++i];
				std::transform(dirPattern.begin(), dirPattern.end(), dirPattern.begin(),
					[](unsigned char c) { return std::tolower(c); });
			}
			else if (arg == "--no-newline") {
				newline = false;
			}
			else if (arg == "--add-end") {
				last = true;
			}
			else if (arg == "--separator" && i + 1 < argc) {
				separator = argv[++i];
			}
		}
	}

	void listFiles() {
		if (!fs::exists(inputPath) || !fs::is_directory(inputPath)) {
			throw std::runtime_error("Input path is not a valid directory");
		}

		std::ostream* output = &std::cout;
		std::ofstream outFile;

		if (!outputPath.empty()) {
			outFile.open(outputPath);
			if (!outFile) throw std::runtime_error("Cannot open output file");
			output = &outFile;
		}

		std::vector<fs::path> files;
		collectFiles(inputPath, 0, files);

		if (files.empty()) return;
		if (separator.empty() && !newline) separator = ";";
		// 修复输出逻辑：统一处理分隔符
		for (size_t i = 0; i < files.size(); ++i) {
			// 输出文件名
			*output << files[i].filename().string();

			// 处理分隔符
			if (i < files.size() - 1) { // 不是最后一个文件
				if (newline) {
					*output << separator <<'\n'; // 换行模式使用换行符
				}
				else {
					*output << separator; // 不换行模式使用指定分隔符
				}
			}
		}

		// 是否再最后换行模式下，最后以后添加分隔符，所有模式都以换行符结束
		if(last && newline)
			*output << separator << '\n';
		else
			*output << '\n';
	}

private:
	fs::path inputPath;
	fs::path outputPath;
	int maxDepth = 0; // 0表示不递归
	std::string filePattern = "*.*";
	std::string dirPattern = "*";
	bool newline = true;
	bool last = false;
	std::string separator = "";

	void collectFiles(const fs::path& path, int currentDepth, std::vector<fs::path>& files) {
		for (const auto& entry : fs::directory_iterator(path)) {
			// 检查是否是文件且匹配模式
			if (entry.is_regular_file() && matchPattern(entry.path().filename().string(), filePattern)) {
				files.push_back(entry.path());
			}
			// 检查是否是目录且需要递归
			else if (entry.is_directory() &&
				(maxDepth < 0 || currentDepth < maxDepth) &&
				matchPattern(entry.path().filename().string(), dirPattern)) {
				collectFiles(entry.path(), currentDepth + 1, files);
			}
		}
	}

	bool matchPattern(const std::string& filename, const std::string& pattern) const {
		if (pattern == "*" || pattern == "*.*") return true;

		// 转换为小写以便大小写不敏感比较
		std::string fnLower = filename;
		std::transform(fnLower.begin(), fnLower.end(), fnLower.begin(),
			[](unsigned char c) { return std::tolower(c); });

		// 处理简单扩展名匹配（如 ".lib"）
		if (pattern.find('*') == std::string::npos &&
			pattern.find('?') == std::string::npos) {
			// 确保模式以点开头
			std::string cleanPattern = pattern;
			if (cleanPattern[0] != '.') {
				cleanPattern = "." + cleanPattern;
			}

			// 检查文件扩展名是否匹配
			return fnLower.size() >= cleanPattern.size() &&
				fnLower.substr(fnLower.size() - cleanPattern.size()) == cleanPattern;
		}

		// 处理通配符模式（如 "*.lib"）
		// 转换通配符模式为正则表达式
		std::string regexPattern = std::regex_replace(pattern,
			std::regex("\\."), "\\.");
		regexPattern = std::regex_replace(regexPattern,
			std::regex("\\*"), ".*");
		regexPattern = std::regex_replace(regexPattern,
			std::regex("\\?"), ".");
		regexPattern = "^" + regexPattern + "$";

		// 使用大小写不敏感的正则表达式匹配
		std::regex re(regexPattern, std::regex_constants::icase);
		return std::regex_match(fnLower, re);
	}
};

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cout << "Usage: " << argv[0] << " --input <path> [options]\n"
			<< "Options:\n"
			<< "  --output <path>       Output file path\n"
			<< "  --recursive <depth>   Recursion depth (0=current only, -1=infinite)\n"
			<< "  --pattern <ext>       File extension pattern (e.g. 'lib', '*.txt')\n"
			<< "  --dir-name <pattern>  Directory name pattern for recursion\n"
			<< "  --no-newline          Output without newlines\n"
			<< "  --add-end			    Add Custom separator to last filename end if no set --no-newline\n"
			<< "  --separator <char>    Custom separator (newline default empty, no-newline default ';')\n";

		return 1;
	}

	try {
		FileLister lister;
		lister.parseArguments(argc, argv);
		lister.listFiles();
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}