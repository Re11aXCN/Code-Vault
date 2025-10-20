#include <cstdio>
#include <vector>
#include <type_traits>
#include <string>
#include "print.h"

#define REQUIRES(x) std::enable_if_t<(x), int> = 0

struct mystudent {
	void dismantle() {
		printf("rm -rf stu.db\n");
	}

	void rebel(int i) {
		printf("rm -rf gench-%d\n", i);
	}
};

struct myclass {
	void dismantle() {
		printf("rm -rf course\n");
	}
};

struct myteacher {
	void rebel(int i) {
		printf("rm -rf gench-%d\n", i);
	}
};

struct myvoid {
};

template <class T, class = void>
struct has_dismantle {
	static constexpr bool value = false;
};

// std::void_tĿ����Ϊ��֪��decltype(std::declval<T>().dismantle())�Ƿ����ɹ�
// ����ɹ����ܹ����룬�õ�true��������������false
template <class T>
struct has_dismantle<T, std::void_t<decltype(std::declval<T>().dismantle())>> {
	static constexpr bool value = true;
};
// ��������ģ��д��ʵ���˶������ػ�������Ҫ�ֶ�д�����/�ṹ���ػ�ģ��

template <class T, class = void>
struct has_rebel {
	static constexpr bool value = false;
};

template <class T>
struct has_rebel<T, std::void_t<decltype(std::declval<T>().rebel(std::declval<int>()))>> {
	static constexpr bool value = true;
};

template <class T, REQUIRES(has_dismantle<T>::value)>
void gench(T t) {
	t.dismantle();
}

template <class T, REQUIRES(!has_dismantle<T>::value&& has_rebel<T>::value)>
void gench(T t) {
	for (int i = 1; i <= 4; i++) {
		t.rebel(i);
	}
}

template <class T, REQUIRES(!has_dismantle<T>::value && !has_rebel<T>::value)>
void gench(T t) {
	printf("no any method supported!\n");
}

// gench���ж����Ƿ����ĳ����Ա����(���������)
/*
// c++ 20д��

template <class T>
void gench(T t) {
	if constexpr (requires { t.dismantle(); })
		t.dismantle();
	else if constexpr (requires (int i) { t.rebel(i); }) // 20д����֮ǰд��t.rebel(std::declval<int>())
		for (int i = 1; i <= 4; i++) {
			t.rebel(i);
		}
	else
		printf("no any method supported!\n");
}

*/

int main() {
	myclass mc;
	mystudent ms;
	myteacher mt;
	myvoid mv;
	gench(mc);
	gench(ms);
	gench(mt);
	gench(mv);
	return 0;
}

/*
* #include <type_traits>
template <class T>
auto func(T t)
{
	// T ����д decltype(t)������ֻ������һ���������int double
	// �����t�������� int const & ����ôdecltype(t)����int const &���ܹ���intƥ��
	// �滻Ϊstd::decay_t<decltype(t)>

	// ����t��vectorʹ�� typename T::value_type ,
	// ʲôʱ���typename�����ʹ����ģ�����T�����߼������t����std::decay_t<decltype(t[0])>
	// ԭ�������vector<int>i����ô��funcģ�������� std::decay<decltype(t[0])>::type * i
	// ���ǲ�֪���ػ���::type��::type * i��һ���˷���������һ��ָ��
	// ע��decay_t��decay������decay_t����typename
	if constexpr (std::is_same_v<T,int>)    //ʹ�ñ��������뵽�ܹ�t+1��Ҫ����constexpr
		return t + 1;
	else
		return t.substr(1);
}

template <class F>
auto invoke(F f)
{
	printf("entered \n");
	// std::is_void_v<decltype(f())> �ȼ��� std::is_void_v<std::invoke_result_t<F>>
	// ����std::is_void_v<decltype(F()())> ������һ�� std::is_void_v<std::decalval<F const &>()()>
	if constexpr (std::is_void_v<decltype(f())>) //ͨ������д�ܹ�����f()�ķ���ֵȻ���ret��
	{
		f();
		printf("Leaved \n");
	}
	else
	{
		auto ret = f();
		printf("Leaved \n");
		return ret;
	}
}

invoke([]() -> int
{
	return 1;
})

invoke([]() -> void
{
	return 0;
})

������һ��invokeʵ��int����void��ͬ����Ӧ
Ҳ���Բ��Ϊ����invoke�ֱ���
requires��c++20����������invoke(F f)ģ�庯����������һ���ض�����������ʵ�����Ͳ�һ��
template <class F>
	requires(std::is_void_v<std::invoke_result_t<F>>)
auto invoke(F f)
{
	printf("entered \n");
	f();
	printf("Leaved \n");
}

template <class F>
	requires(!std::is_void_v<std::invoke_result_t<F>>)
auto invoke(F f)
{
	printf("entered non-void \n");
	auto ret = f();
	printf("Leaved non-void \n");
	return ret;
}

// ����c++20����д
template <class F,
		std::enable_if_t<
			!std::is_void_v<std::invoke_result_t<F>>
		, int> = 0 >

���ߺ��д
#define REQUIRES(x) std::enable_if_t<(x), int> = 0

template <class F, REQUIRES(!std::is_void_v<std::invoke_result_t<F>>) >
template <class F, REQUIRES(!std::is_void_v<decltype(F()())>) > // �����������дf()����F()()�滻

// ���ʹ��lambda���ʽû���˹��캯����ôF()()�滻Ϊstd::declval<F>()()
// ��ȫ��std::declval<F const &>()()
*/