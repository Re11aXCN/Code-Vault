#include <cstdio>
#include "foo.h"
// static int i = 1;	//static 把符合限制在当前编译单元，外部不可见

// inline 违背单一定义原则，可以再多个单元出现同一个符号，链接器只会随机选择一个符号
inline int i = 5;

void foo()
{
	printf("main %d\n", i);
	i += 1;
}