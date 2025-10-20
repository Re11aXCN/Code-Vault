#include "resource.hpp"
#include <cstdio>

struct Resource::Self {
	FILE* p;

	Self() {
		puts("打开文件");
		p = fopen("CMakeCache.txt", "r");
	}

	Self(Self&&) = delete;

	void speak() {
		printf("使用文件 %p\n", p);
	}

	~Self() {
		puts("关闭文件");
		fclose(p);
	}
};

Resource::Resource() : self(std::make_unique<Self>()) {
}

// 如果是shared_ptr那么不用写析构，因为基于类型擦除
//Resource::Resource() : self(std::make_shared<Self>()) {
//}

void Resource::speak() {
	return self->speak();
}

Resource::~Resource() = default;    // unique_ptr智能指针会自动删除，必须要写才能够知道self存在
