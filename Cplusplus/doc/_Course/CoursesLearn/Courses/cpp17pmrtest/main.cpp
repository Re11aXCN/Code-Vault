#include "ticktock.h"
#include <memory_resource>
#include "memory_resource_inspector.h"
#include <vector>
#include <list>
#include <deque>

// ��̬�ڴ���Դpmr
// newdelete < sync < unsync < monot

int main() {
	memory_resource_inspector mem{ std::pmr::new_delete_resource() };

	std::pmr::vector<int> s{ &mem };
	for (int i = 0; i < 4096; i++) {
		s.push_back(i);
	}
}

/*
*
* ͨ���ȷ����ڴ����ӿ�list��д�������������ڴ浽�ں�̬�Ŀ���

// ����������Դ�ģ�
// Ϊʲô�����ϵ�my_custom_allocator����Ϊ���һ������(�ܴ���Ϊ[65536 * 10])���ϵĿ������ƶ����ƶ�ȥ����Ҫ��ö�
static char g_buf[65536 * 30];
struct my_memory_resource
{
	// char m_buf[65536 * 10]; // ջ��һ���Է���������ջ���
	// �滻Ϊvector
	// std::vector<char> m_buf = std::vector<char>(65536 * 30);
	// ����
	char *m_buf = g_buf;
	size_t m_watermark = 0;

   char *do_allocate(size_t n, size_t align) // align��Ϊ���ڴ����
   {
		m_watermark = (m_watermark + align -1) / align * align;
		char *p = m_buf + m_watermark;
		m_watermark += n * sizeof(T);
		return p;
   }
};

template <class T>
struct my_custom_allocator
{
	my_memory_resource *m_resource��
	using value_type = T;

	my_custom_allocator(my_memory_resource *resource)
	:m_resource(resource){}

	T *allocate(size_t n)
	{
		// return (T*)buf; // ʼ��ָ���׵�ַ
		//================================
		//char *p = m_resource->buf + m_resource->watermark;
		//m_resource->watermark += n * sizeof(T);
		//return (T*)p;
		//================================
		// �����ÿ������ٴη�װ
		 char *p = m_resource->do_allocate(n * sizeof(T), alignof(T));
		return (T*)p;

	}
	void deallocate(T *p, size_t n)
	{ // ʲô������ }

	my_custom_allocator() = default;

	template <class U>
	constexpr my_custom_allocator(my_custom_allocator<U> const &other) noexcept
	{
	}

	constexpr bool operator==(my_custom_allocator const &other) noexcept
	{
		return this == &other;
	}
};
{
	std::list<char, my_custom_allocator<char>> a;
	// prev8�ֽ� next8�ֽ� char1�ֽ� ���padding7�ֽ� node24
	TICK(list);
	for (int i = 0; i < 65536; i++){
		a.push_back(42);
	}
	TICK(list);
}
TOCK(list);
=================================================
// C
{
   char *p = (char *)malloc(4);
   free(p);
}

{
	// C++ û�� allocator
	char* p = new char[4];
	delete[]p;
}

{
	// C++ �� allocator
	std::allocator<char> alloc;
	char *p = alloc.allocate(4);
	// delete��free���ڴ����ĳ�����Ϣ��deallocate��Ҫָ��Ϊ�˸�ͨ�ã��еķ�����û�г�������Ҫָ����
	alloc.deallocate(p, 4); }

{
	my_custom_allocator alloc;
	char *p = alloc.allocate(4);
	alloc.deallocate(p, 4);
}
*/