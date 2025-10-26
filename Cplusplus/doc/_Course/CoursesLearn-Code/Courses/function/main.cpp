#include "Function.hpp"
#include "print.h"

// C���Ժ���ָ�룬����ģʽ
struct S
{
#ifdef POINT
	int* x;
	int* y;
#else
	int& x;
	int& y;
#endif // POINT

};

void fun_hello(void* arg)
{
	auto a = (S*)arg;
	//printf("hello%d", *(int*)arg);
#ifdef POINT
	printf("hello %d %d", *a->x, *a->y);
#else
	printf("hello %d %d\n", a->x, a->y);
#endif // POINT
}

typedef void (*pfunc_t)(void* arg);

void repeat(pfunc_t func, void* arg)
{
	func(arg);
}

// C++lambda���ʽ���߼�
struct S2
{
	int x;
	int y;
	//void call() //����һ����Ա��������
	//{
	//	printf("hello %d %d", x, y);
	//}
	void operator()(int i) const
	{
		// x = 1;
		printf("hello %d %d\n", x + i, y);
	}
};

/*
ģ���޷����������Ͷ��壬һ���ļ�����������һ���ļ��㶨��ʵ�ֲ��У�����Ϊʲô�и�hpp�ļ�
��ʱ����������Ҫ���ڽṹ���ڲ�����ʱ�����Ҫstd::function<void(int)>����Ҫָ������
*/
template <class Fn>
void repeat2(Fn const& func)
{
	// func.call(); // call��ͨ�ã�������Ҫ���ж��д
	// �º���
	func(1);
}

// ʵ�ּ��׵�std::function<void(int)>
void repeat3(Function<void(int)> const& func)
{
	// func.call(); // call��ͨ�ã�������Ҫ���ж��д
	// �º���
	func(1);
}

int main()
{
	//int x = 1;
	//repeat(fun_hello, NULL);
	//repeat(fun_hello, &x);

#ifdef POINT
	S s;
	int x = 1, y = 2;
	s.x = &x, s.y = &y;
#else
	int x = 1, y = 2;
	S s{ x,y };
#endif // POINT
	repeat(fun_hello, &s);

	S2 s2{ x,y };
	repeat2(s2);

	// lambda����һ���﷨�ǣ�����д�ṹ��ķº������º���Ĭ��Ϊconst
	repeat2([&x, &y](int i) // ֻдһ��&���ڲ��õ�ʲô�����Ͱ��㲶��ʲô
		{
			printf("hello %d %d\n", x + i, y);
		});
	//repeat2([x, y]()
	//	{
	//		// GCC=�ᱨ��ԭ������Ϊ�ṹ���ڵ�operator()��const���ܹ��޸ģ�Ҫ��mutable
	//		// msvc����x����Ҫ��ֵ������mutableҲ�ܹ��޸�
	//		// ��˼��һ���ģ�mutable���ɵĽṹ��ѷº�����constȥ��
	//		x = 1;	
	//		printf("hello %d %d\n", x, y);
	//	});

	repeat3([&x, &y](int i) // ֻдһ��&���ڲ��õ�ʲô�����Ͱ��㲶��ʲô
		{
			printf("hello %d %d\n", x + i, y);
		});
}

