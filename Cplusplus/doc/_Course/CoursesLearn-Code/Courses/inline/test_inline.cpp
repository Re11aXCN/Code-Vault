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

// ͷ�ļ��ĺ�������inline���Σ�Ϊ�˷�ֹ.hճ����ʱ���ظ�����
// class��struct��������ʵ��Ĭ��inline

// .h����class/struct����������.cpp���壬��ô�����ķ���Ϊextern
// .h����class/struct�����������ⲿ���壬Ҫ�ֶ���inline������

inline int i = 3;

namespace {
	//�������ֿռ�Ҳ�ܹ���ֹ��ͻ
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