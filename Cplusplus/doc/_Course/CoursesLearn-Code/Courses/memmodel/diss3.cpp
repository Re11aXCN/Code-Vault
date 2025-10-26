#include <cstdio>
#include <ranges>
using namespace std;

int main() {
	int a[2] = { 1, 2 };
	int* p = &a[0];
	printf("p     = %p\n", p);
	printf("p + 1 = %p\n", p + 1);
	printf("p + 2 = %p\n", p + 2);
	printf("%d\n", *p);
	printf("%d\n", *(p + 1));
	printf("%d\n", *(p + 2));

	// �ڴ����ѡ����8�ֽڣ�int4�ֽں�����Ҫ�ÿհ��
	struct A {
		// ע��˳��ͬ����sizeof�Ĵ�С��ͬ
		// int pading;	// 24
		double d;
		int a;
		// int pading;	// 16
	};

	constexpr size_t i = sizeof(A);		// 16
	constexpr size_t j = alignof(A);	// 8

	return 0;
}
