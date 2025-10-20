#include <print>
#include <vector>


/* 平凡类型不用封装get/set，直接public，*/
/* vector 不是平凡的，setSize会导致data改变*/
int main() {
	std::vector<int> v;
	std::println("{} {}", (void*)v.data(), v.size()); // v.getData(), v.getSize()
	v.resize(14); // v.setSize(14)
	std::println("{} {}", (void*)v.data(), v.size()); // v.getData(), v.getSize()
	v.resize(16); // v.setSize(16)
	std::println("{} {}", (void*)v.data(), v.size()); // v.getData(), v.getSize()
	return 0;
}
