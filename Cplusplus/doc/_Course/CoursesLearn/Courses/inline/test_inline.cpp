#include <iostream>
#include <string>
#include "foo.h"
/*
int i; // definition
extern int i; // decleration
void foo(); // decleration
extern void foo(); // decleration
void foo() {}// definition
*/

// 头文件的函数都用inline修饰，为了防止.h粘贴的时候重复定义
// class、struct内声明并实现默认inline

// .h定义class/struct声明方法，.cpp定义，那么声明的方法为extern
// .h定义class/struct声明方法，外部定义，要手动加inline在声明

inline int i = 3;

namespace {
	//匿名名字空间也能够防止冲突
}

int main() {
	std::cout << "main " + std::to_string(i) + "!\n";
	foo();
	foo();
	foo();
	foo();
	std::cout << "main " + std::to_string(i) + "!\n";
	return 0;
}