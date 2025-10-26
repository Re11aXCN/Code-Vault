#include <format>
#include <vector>
#include <functional>
#include <stdexcept>
#include <iostream>
#include <string>
#include <atomic>

int main() {
	// c++17可以不会进行拷贝
	auto a = std::atomic<int>(5);
	// auto b = a; // error，禁止不通过load()进行拷贝，删除了拷贝方法
	return 0;
}
