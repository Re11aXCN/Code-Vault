#include "minilog.h"

// ģ������Ա�����Դ�inline�ؼ��� ����ʾ����
// ��ֹ.h���չ����������������������������֪��������һ����inline���þ����ܹ�ʹ�����ֶ��ʹ��
// �÷��Ų����е�һ����
// __attribute__((always_inline))�����������Ż���������
// static ����ı�����ֻҪÿ��cpp������.h�ļ����Ͷ�����һ������������Ҳ���������
// �� a.cpp�޸ı�����b.cpp�ļ�����������
// c++17���Ը���������inline���� inline log_level g_max_level ʹ��ȫ�ֹ���
// .cpp�ļ�������ָ��ͷ�ļ�����һ������

// constevalֻ�ܱ��������ã�constexpr���Ա��������ÿ�������ʱ����
using namespace std::literals::string_literals;
int main() {
	// ϵͳ��Ƶ�ԭ��std::getenv�ȵ��ã����Ի����������õ���־·��������־�ȼ��ᱻ����
	minilog::set_log_file("test_log.txt");
	minilog::set_log_level(minilog::log_level::debug);
	minilog::log_trace("This is a trace message.");
	minilog::log_info("hello, the answer is {}", 42);
	minilog::log_critical("this is right-aligned [{:>+10.04f}]", 3.14);

	minilog::log_warn("good job, {1:.5s} for making {0}", "minilog", "archibate");
	minilog::set_log_level(minilog::log_level::trace); // default log level is info

	int my_variable = 42;
	MINILOG_P(my_variable); // shown when log level lower than debug

	minilog::log_trace("below is the color show :)");
#define _FUNCTION(name) minilog::log_##name(#name);
	MINILOG_FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION

		return 0;
}
