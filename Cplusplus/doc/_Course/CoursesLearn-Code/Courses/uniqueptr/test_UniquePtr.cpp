#include <cstdio>
#include <vector>
#include <iostream>
#include "UniquePtr.hpp"

// �����һ����ţ����������Զ��Ѱ����ı���������������
// ǳ����˫free

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
* cat�ı���age������
���þ����������õı�����˥��Ϊ��ͨ����������дage�ͻ��ӡ�����֣���������δ������Ϊ
UniquePtr<Cat> makeUniqueCat(int age){
	return UniquePtr<Cat>(new Cat(age));
}
��������������������õģ�������������ת��std::forward��&&��������
*/

void func(FILE* fp /* take ownership*/) {	// c������Ҫ�û����Ȩ
	fclose(fp);
}

int main() {

	/*
	UniquePtr<FILE> fp(fopen("a.txt","r"));
	// func(fp): // ����c������Ҫ�û����Ȩ
	// func(fp.get()); //get��ָ����������� free�����飬func�ͷ���һ�飬Ȼ��main��}���ͷ���һ��
	func(fp.release()); // ������Ҫrelease���ڲ�������exchange
	*/

	std::vector<UniquePtr<Animal>> zoo;
	int age = 3;
	zoo.push_back(makeUnique<Cat>(age));
	zoo.push_back(makeUnique<Dog>(age));
	for (auto const& a : zoo) {	//������ɾ�ˣ��ƶ���Ϣ����Ҫ��const��&����Ч
		a->speak();
	}
	age++;
	for (auto const& a : zoo) {
		a->speak();
	}
	return 0;
}
