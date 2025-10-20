#include <iostream>
#include <string>
#include "scienum.h"
#define Log(x) (std::cout << "File: " \
<< __FILE__ \
<< ", Function: " \
<< __func__ \
<< "(), " \
<< ", PrettyFunction: " \
<< __FUNCSIG__ \
<< "(), Line: " \
<< __LINE__ \
<< std::endl)
// __func__��__FUNCTION__ ���ƣ�__PRETTY_FUNCTION__ ��GCC�������ĺ�
enum Color {
	RED, GREEN, BLUE, YELLOW,
};
template<class T>
void test(const T& X)
{
	Log(X);
}
void test(const char* str)
{
	Log(str);
}
void test(const std::string& str)
{
	Log(str);
}
void test()
{

}

template <class T>
std::string get_type_name()
{
	std::string str = __FUNCSIG__;
	auto pos = str.find_last_of("<") + 1;
	auto pos2 = str.find_first_of(">", pos);
	return str.substr(pos, pos2 - pos);
}
// ģ�������͡�������<class T, T N>��ö��Ҳ������
// �������Ʒ����ö�����͵�����ȡ����
template <class T, T N>
std::string get_int_name()
{
	std::string str = __FUNCSIG__;
	auto pos = str.find_last_of("<") + 1;
	auto pos2 = str.find_first_of(">", pos);
	return str.substr(pos, pos2 - pos);
}

template <int N>
struct integral_constant
{
	static constexpr int value = N;
};

template <int Beg, int End, class F>
void static_for_test(F const& func)
{
	if constexpr (Beg == End)
	{
		return;
	}
	else
	{
		// func����ÿ�ζ������ʵ���������ǰ�lambda���ʽ������
		func(integral_constant<Beg>()); //���Ե��ñ�׼���std::integral_constant
		static_for_test<Beg + 1, End>(func);
	}
}

template <class T>
std::string get_int_name_dynamic(T n)
{
#if 0
	// ת����get_int_name��ÿ��T N��������һ��ʵ��
	if (n == (T)1) return get_int_name<T, (T)1>();
	if (n == (T)2) return get_int_name<T, (T)2>();
	if (n == (T)3) return get_int_name<T, (T)3>();

	// �Ż�������for������ʱ���õģ�������Ҫ�����ٴθĶ���Ϊ����ʱ��forѭ��
	for (int i = 0; i < 3; i++)
	{
		if (n == (T)i) return get_int_name<T, (T)i>();
	}
#endif
	// ͨ��static_for_test����ʵ������ͨ��lambda���ʽ�������ÿ��ʵ����
	std::string ret;
	// Ϊʲôдauto��дint����Ϊ�����int��func(Beg), <T, (T)i>��ֻ���Ǿ�̬������int i��һ��û����
	// ���Ǵ��ڶ���func(Beg)��Beg�ͱ�Ϊ�˶�̬������
	// static_for_test<0, 256>([&] <int i>() , func<Beg>()����C++20֧�ֵ�д�������ڰ汾��֧��
	// auto �൱�� <class T>(T i)
	static_for_test<0, 256>([&](auto i)
		{
			//constexpr auto i = ic.value;
			//if (n == (T)i) return get_int_name<T, (T)i>(); //ע��return ���ܹ����ص�����
			if (n == (T)i.value) ret = get_int_name<T, (T)i.value>();
		});
	return ret;
}

int main1() {
	enum ColorTest {
		RED, GREEN, BULE
	};
	std::cout << get_int_name<ColorTest, (ColorTest)2>() << std::endl;
	//ColorTest c = RED; //����ʱ�ĵ���
	//std::cout << get_int_name<ColorTest, c>() << std::endl; // ģ��������֮ǰ�����Ա���
	// ��ȷд�� 
	constexpr ColorTest c = RED;
	std::cout << get_int_name<ColorTest, c>() << std::endl;

	// ������ʱ������������Ҫ����ʱ��ȡ����ô�죿
	ColorTest c2 = GREEN;
	std::cout << get_int_name_dynamic(c2) << std::endl;

	std::cout << get_type_name<uint32_t>() << std::endl;
	test("Hello");
	test(3);
	std::cout << scienum::get_enum_name(YELLOW) << std::endl; // ����
	std::cout << scienum::enum_from_name<Color>("YELLOW") << std::endl;// ����
}
