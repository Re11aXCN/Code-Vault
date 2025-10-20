#pragma once

#include <memory>
//#include <cstdio>
// 

// struct _IO_FILE; // 不导入#include <cstdio>实现
// 但是不支持 struct _IO_FILE p; 不是指针情况

// P-IMPL 模式，移动到cpp实现
struct Resource {
private:
	// FILE *p;
	// struct _IO_FILE *p;

	struct Self;    // 前向声明

	std::unique_ptr<Self> self;

	//std::shared_ptr<Self> self;

public:
	Resource();
	void speak();
	~Resource();
};

/*
#include "resource.hpp"
using namespace std;
int main() {
	Resource res;
	res.speak();
	return 0;
}
*/