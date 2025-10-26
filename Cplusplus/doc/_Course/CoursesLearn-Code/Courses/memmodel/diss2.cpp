#include <cstdio>
using namespace std;

[[gnu::noinline]] void modify(void* p) {
	*(short*)p = 3;               // 企图修改低 16 位，错误
	/*
	short x = 3;
	std::memcpy(p, &x, sizeof(short)); // 合法，因为memcpy是标准库的东西
	*/
}

[[gnu::noinline]] int func1() {
	int i = 0x10002;
	modify((void*)&i);
	return i;
}

int main() {
	printf("%#x", func1());
	return 0;
}
