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

// template <class T>�ܹ������������
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
// ��main��������֮ǰ������������������̬��ʼ��
// ��
// static int dummy = puts("main֮ǰ");

#if defined(__GNUC__)
__attribute__((constructor)) void before_main() {
	puts("main֮ǰ");
}
__attribute__((destructor)) void after_main() {
	puts("main֮��");
}
// __crt__main���ȱ���obj�ļ����������к���ָ��ִ��һ��
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
