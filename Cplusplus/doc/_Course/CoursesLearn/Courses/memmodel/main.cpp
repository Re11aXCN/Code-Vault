#include <cstdio>
#include <cstdlib>
using namespace std;

void modify(int* pa) {
	int* pb = pa + 1;  // 企图通过 a 的指针访问 b
	*pb = 3;           // 如果成功，那么 b 应该变成了 3
}

int func1() {
	int a = 1;
	int b = 2;
	modify(&a);
	return b;           // 时而 3，时而 2？
}

int func2() {
	struct {
		int a = 1;
		int b = 2;
	} s;
	modify(&s.a);
	return s.b;          // 3
}

int func3() {
	int a[2];
	a[0] = 1;
	a[1] = 2;
	modify(&a[0]);
	return a[1];         // 3
}

int main() {
	printf("%d\n", func3());

	// 对齐到缓冲区的大小64字节，超过long double 16字节大小
	struct alignas(64) A {
		long double ld;
		double d;
		float f;
	};

	A* p = (A*)malloc(sizeof(A)); // UB，未定义行为
	A* p = (A*)new char[sizeof(A)];// UB
	A* q = (A*)new (std::align_val_t(alignof(A)))char[sizeof(A)];
	A* q = new A;
	printf("%p\n", p);
	printf("%ld\n", (uintptr_t)p % 64);

	/*
使用const_cast场景
1. 本来就不是 const 变量，被转为 const 指针后，可以去掉 const
2. const 变量是一个结构体，其有着 mutable 的成员，那么这个成员可以去掉 const

使用reinterpret_cast场景
1. char * 可以访问 任何类型 的变量（方便做 memcpy）
2. unsigned int * 可以访问 int 变量（带有 unsigned 修饰不影响）
3. const int ** 可以访问 int * 变量（二级指针带有 const 修饰不影响）
4. 结构体 A 如果含有 int 成员，那么可以用 A * 访问 int 变量（允许通过结构体指针访问成员）

	*/

	return 0;
}
