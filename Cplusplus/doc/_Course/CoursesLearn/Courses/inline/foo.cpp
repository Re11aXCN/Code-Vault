#include <cstdio>
#include "foo.h"
// static int i = 1;	//static �ѷ��������ڵ�ǰ���뵥Ԫ���ⲿ���ɼ�

// inline Υ����һ����ԭ�򣬿����ٶ����Ԫ����ͬһ�����ţ�������ֻ�����ѡ��һ������
inline int i = 5;

void foo()
{
	printf("main %d\n", i);
	i += 1;
}