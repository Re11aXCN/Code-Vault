#include <format>
#include <vector>
#include <functional>
#include <iostream>
#include <string>

using namespace std;

struct ComponentBase {
protected:
	inline static size_t component_type_count = 0;
};

// template <class T>能够父类访问子类
template <class T>
struct Component : ComponentBase {
private:
	inline static const size_t s_component_type_id = component_type_count++;
public:
	static constexpr size_t component_type_id() {
		return s_component_type_id;
	}
};


struct A : Component<A> {};
struct B : Component<B> {};
struct C : Component<C> {};
// 在main方法调用之前，调用其他东西，静态初始化
// 如
// static int dummy = puts("main之前");

#if defined(__GNUC__)
__attribute__((constructor)) void before_main() {
	puts("main之前");
}
__attribute__((destructor)) void after_main() {
	puts("main之后");
}
// __crt__main会先遍历obj文件，搜索所有函数指针执行一遍
#endif

int main() {
	printf("A: %zd\n", A::component_type_id());
	printf("B: %zd\n", B::component_type_id());
	printf("A: %zd\n", A::component_type_id());
	printf("A: %zd\n", A::component_type_id());
	printf("A: %zd\n", A::component_type_id());
	printf("C: %zd\n", C::component_type_id());
	return 1;
}
