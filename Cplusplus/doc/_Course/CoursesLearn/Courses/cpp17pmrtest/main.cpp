#include "ticktock.h"
#include <memory_resource>
#include "memory_resource_inspector.h"
#include <vector>
#include <list>
#include <deque>

// 多态内存资源pmr
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
* 通过先分配内存来加快list读写，减少了申请内存到内核态的开销

// 真正持有资源的，
// 为什么不整合到my_custom_allocator，因为如果一个对象(很大因为[65536 * 10])不断的拷贝，移动来移动去开销要大得多
static char g_buf[65536 * 30];
struct my_memory_resource
{
	// char m_buf[65536 * 10]; // 栈上一次性分配可能造成栈溢出
	// 替换为vector
	// std::vector<char> m_buf = std::vector<char>(65536 * 30);
	// 或者
	char *m_buf = g_buf;
	size_t m_watermark = 0;

   char *do_allocate(size_t n, size_t align) // align是为了内存对齐
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
	my_memory_resource *m_resource：
	using value_type = T;

	my_custom_allocator(my_memory_resource *resource)
	:m_resource(resource){}

	T *allocate(size_t n)
	{
		// return (T*)buf; // 始终指向首地址
		//================================
		//char *p = m_resource->buf + m_resource->watermark;
		//m_resource->watermark += n * sizeof(T);
		//return (T*)p;
		//================================
		// 解引用开销大再次封装
		 char *p = m_resource->do_allocate(n * sizeof(T), alignof(T));
		return (T*)p;

	}
	void deallocate(T *p, size_t n)
	{ // 什么都不做 }

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
	// prev8字节 next8字节 char1字节 填充padding7字节 node24
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
	// C++ 没有 allocator
	char* p = new char[4];
	delete[]p;
}

{
	// C++ 有 allocator
	std::allocator<char> alloc;
	char *p = alloc.allocate(4);
	// delete和free有内存分配的长度信息，deallocate需要指明为了更通用（有的分配器没有长度所以要指定）
	alloc.deallocate(p, 4); }

{
	my_custom_allocator alloc;
	char *p = alloc.allocate(4);
	alloc.deallocate(p, 4);
}
*/