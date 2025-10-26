#include <memory>
#include <string>
#include <vector>

// 掌管资源的类？四大函数全部删光！然后安心定义析构函数
struct Resource {
	Resource() {
		// 分配资源
	}

	// 删除了 移动构造 其他默认都删除可以不写
	Resource(Resource&&) = delete;
	Resource(Resource const&) = delete;            // 可省略不写
	Resource& operator=(Resource&&) = delete;      // 可省略不写
	Resource& operator=(Resource const&) = delete; // 可省略不写

	~Resource() {
		// 释放资源 

		//if (p != NULL) {
		//    puts("释放资源");
		//    free(p);
		//}
	}
};

// 如果这个类需要作为参数传递，需要移动怎么办？
std::unique_ptr<Resource> p = std::make_unique<Resource>();

// 每次都要 std::move 不方便，我想要随心所欲的浅拷贝，就像 Java 对象一样，怎么办？
std::shared_ptr<Resource> q = std::make_shared<Resource>();

// 不管理资源的类？那就都不用定义了！编译器会自动生成
struct Student {
	std::string name;
	int age;
	std::vector<int> scores;
	std::shared_ptr<Resource> bigHouse;

	// 编译器自动生成 Student 的拷贝构造函数为成员全部依次拷贝，不用你自己定义了

	// Resource bar(std::move(foo));
	// 等价于，std::move不参与移动，移动是移动构造做的事情，std::move只是把类型转为&&
	// Resource bar(static_cast<Resource &&>(foo));
};

/*
// 可以写包装
struct Resource {
private:
	struct Self {
		void* p;
		Self() {
			puts("分配资源");
			p = malloc(1);
		}
		~Self() {
			puts("释放资源");
			free(p);
		}
	};
	std::shared_ptr<Self> self;	// 浅拷贝
public:
	Resource() {
		self = std::make_shared<Self>();

	}
	void speak() {
		printf("旺旺，我的资源句柄是 %p\n", self->p);
	}
};

void func(Resource x) {
	x.speak();
}
*/

struct Self {
	void* p;
	Self() {
		puts("分配资源");
		p = malloc(1);
	}

	Self(Self&&) = delete;

	~Self() {
		puts("释放资源");
		free(p);
	}
	virtual void speak() const {
		printf("旺旺，我的资源句柄是 %p\n", p);
	}
};

// 也可以分开写，更明确指定shared_ptr还是unique_ptr还是&
struct Resource {
private:
	std::shared_ptr<Self> self;	// 浅拷贝
	Resource(std::shared_ptr<Self>self_) {
		self = self_;
	}
public:
	Resource() {
		self = std::make_shared<Self>();

	}
	// 浅拷贝
	Resource(Resource const& that) = default;

	// 智能指针深拷贝
	// const self 指针不能变，但是指针指向的内容可变
	Resource clone() const {
		return std::make_shared<Self>(*self);
	}
	void speak() const {
		self->speak();
	}
};



void func(Resource& x) {
	x.speak();
	auto y = x.clone();
	y.speak();
}

void proc(std::shared_ptr<FILE> p) {
	fgetc(p.get()); // 或者fgetc(&*p); 但是p为nullptr的时候fgetc(&*p)为未定义行为，get不会
}

int main() {
	auto x = Resource();
	func(x);
	// func(Resource()); // 第一种写法


	auto p = std::shared_ptr<FILE>(fopen("CMakeLists.txt", "r"), fclose);	// 类型擦除
	// 为了性能delete是传到模板
	auto p = std::unique_ptr<FILE, std::default_delete<FILE>>(fopen("CMakeLists.txt", "r"));
	proc(p);
}