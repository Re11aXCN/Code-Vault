#include <vector>
#include <functional>
#include <iostream>
#include <string>

using namespace std;
template <class T>
inline constexpr bool false_v = false;

template <class T, class Func>
auto& operator|(vector<T>& vec, Func const& func) {
	// ��ֹһЩ��Ӧ�õ��������������vec | f2 | f | 1; //���ֲ����Ե���func
	if constexpr (std::is_invocable_v<Func, T&>)
		for (auto& element : vec) {
			func(element);
		}
	else {
		printf("error:a mao a gou dou lai le shi ba?\n");
		static_assert(false_v<Func>, i"amaoagoudoulai");//��дfalse�� false_v<Func>����/�ӳٱ���
	}
	return vec;
}

//template <class T, class Func> requires (std::invocable<Func, T&>
// �ȼ���
//template <class T, std::invocable<T&> Func>
// �ȼ���
template <class T>
auto& operator|(vector<T>& vec, std::invocable<T&> auto const& func) {
	for (auto& element : vec) {
		func(element);
	}
	return vec;
}

// ��׼�ⲻ֧���ַ��������������Ҫ����

/*
// ��֧������+
auto operator+(string s,int i){
	return s + to_string(i);
}
auto operator+(int i,string s){
	return to_string(i) + s;
}
*/

int main() {
	// CTAD: Class template argument deduction���Զ��Ƶ�����<>ģ���������
	// ���Բ�дvector<int>��function<void(int const &)>
	std::vector vec{ 1, 2, 3 };
	std::function f{ [](const int& i) {std::cout << i << ' '; } };
	auto f2 = [](int& i) {i *= i; };
	// ��ʼ��������Ԫ�ز�������ӡԪ��
	vec | f2 | f;
}
