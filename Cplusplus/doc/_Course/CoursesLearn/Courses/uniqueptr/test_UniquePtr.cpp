#include <cstdio>
#include <vector>
#include <iostream>
#include "UniquePtr.hpp"

// 出了右花括号，编译器会自动把包裹的变量析构函数调用
// 浅拷贝双free

struct MyClass {
	int a, b, c;
};

struct Animal {
	virtual void speak() = 0;
	virtual ~Animal() = default;
};

struct Dog : Animal {
	int age;

	Dog(int age_) : age(age_) {
	}

	virtual void speak() {
		printf("Bark! I'm %d Year Old!\n", age);
	}
};

struct Cat : Animal {
	int& age;

	Cat(int& age_) : age(age_) {
	}

	virtual void speak() {
		printf("Meow! I'm %d Year Old!\n", age);
	}
};

/*
* cat的变量age是引用
引用经过不是引用的变量就衰变为普通变量，这样写age就会打印乱数字，空悬引用未定义行为
UniquePtr<Cat> makeUniqueCat(int age){
	return UniquePtr<Cat>(new Cat(age));
}
但又有需求变量不是引用的，所以有了万能转发std::forward，&&万能引用
*/

void func(FILE* fp /* take ownership*/) {	// c语言需要拿会控制权
	fclose(fp);
}

int main() {

	/*
	UniquePtr<FILE> fp(fopen("a.txt","r"));
	// func(fp): // 错误，c语言需要拿会控制权
	// func(fp.get()); //get出指针给它，但是 free了两遍，func释放了一遍，然后main的}又释放了一遍
	func(fp.release()); // 所以需要release，内部调用了exchange
	*/

	std::vector<UniquePtr<Animal>> zoo;
	int age = 3;
	zoo.push_back(makeUnique<Cat>(age));
	zoo.push_back(makeUnique<Dog>(age));
	for (auto const& a : zoo) {	//拷贝被删了，移动信息出来要加const，&更高效
		a->speak();
	}
	age++;
	for (auto const& a : zoo) {
		a->speak();
	}
	return 0;
}
