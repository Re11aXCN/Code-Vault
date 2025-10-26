#include <functional>
#include <iostream>
#include <vector>

void (Bar::* memfn)(std::string, std::string);	// 对象
void (*putfn)(std::string, std::string);		// 成员函数

/*
// 绑定简化lambda写法
template <class Self, class MemFn>
auto bind(Self* self, MemFn memfn) {
	return [self, memfn](auto ...t) { // define t
		(self->*memfn)(t...);
		};
}
*/

// 智能指针版本
template <class Self, class MemFn>
auto bind(Self self, MemFn memfn) {
	return [self = std::move(self), memfn](auto ...t) { // define t
		((*self).*memfn)(t...);
		};
}
// weak特化
template <class Self, class MemFn>
auto bind(std::weak_ptr<Self> self, MemFn memfn) {
	return [self = std::move(self), memfn](auto ...t) { // define t
		auto ptr = self.lock();
		if (ptr != nullptr) ((*ptr).*memfn)(t...);
		};
}

// 形参包
template <class ...T>
struct Signal {
	std::vector<std::function<void(T...)>> m_callbacks;

	/*
	template <class Self, class MemFn>
	void connect(Self* self, MemFn memfn) {
		m_callbacks.push_back(bind(self, memfn));
	}
	*/
	// 智能指针，与bind匹配
	template <class Self, class MemFn>
	void connect(Self self, MemFn memfn) {
		m_callbacks.push_back(bind(std::move(self), memfn));
	}

	void connect(std::function<void(T...)> callback) {
		m_callbacks.push_back(std::move(callback));
	}

	void emit(T... t) {	// T展开，t定义，
		for (auto&& callback : m_callbacks)
			callback(t...); // t展开
	}
};

struct Foo {
	void on_input(int i, int j) const {
		std::cout << "Foo of age " << age << " got i=" << i << ", j=" << j << '\n';
	}

	int age = 14;

	~Foo() {
		std::cout << "Foo destruct\n";
	}
};

struct Bar {
	void on_input(int i) const {
		std::cout << "Bar of age " << age << " got " << i << '\n';
	}

	void on_exit(std::string msg1, std::string msg2) const {
		std::cout << "Bar got exit event: " << msg1 << " " << msg2 << '\n';
	}

	int age = 42;
};

struct Input {
	void main_loop() {
		int i;
		while (std::cin >> i) {
			on_input.emit(i);
		}
	}

	Signal<int> on_input;
	Signal<std::string, std::string> on_exit;
};

// #define FUN(fun) [&] (auto...t) { return fun(t...); } 
// 进阶，万能引用转发
#define FUN(__fun) [&] (auto &&...__t) { return __fun(std::forward<decltype(__t)>(__t)...); } 
// 改用shared智能指针后得用值捕获
// __VA_OPT__的使用在属性/C++/命令行中添加		/Zc:preprocessor
#define FUNC(__fun, ...) [=] (auto &&...__t) { return __fun(__VA_ARGS__ __VA_OPT__(,) std::forward<decltype(__t)>(__t)...); } 
// __VA_OPT__(X):如果__VA_ARGS__为空，则里面的内容X不展开
// __VA_OPT__(X) :如果__VA_ARGS__不为空，则里面的内容X展开

void test(std::string msg1, std::string msg2) {
	std::cout << "Bar got exit event: " << msg1 << " " << msg2 << '\n';
}

void test(int msg1, std::string msg2) {
	std::cout << "Bar got exit event: " << msg1 << " " << msg2 << '\n';
}

struct Test {
	void operator()(std::string msg1, std::string msg2) {
		std::cout << "Bar got exit event: " << msg1 << " " << msg2 << '\n';
	}

	void  operator()(int msg1, std::string msg2) {
		std::cout << "Bar got exit event: " << msg1 << " " << msg2 << '\n';
	}
} test_;


void dummy(Input& input) {
	/*
	// 未定义行为
	Bar bar;	// 第一次调用完已经被析构，但是input.main_loop();还在循环引用，未定义行为
	input.on_input.connect(&bar, &Bar::on_input);
	*/

	/*
	// 使用shared_ptr ?
	auto bar = std::make_shared<Bar>();		// 计数1
	input.on_input.connect(bar.get(), &Bar::on_input);	// get只延迟长了一次寿命，没用
	*/
	/*
	// 一种解决办法
	auto bar = std::make_shared<Bar>();	// 虽然已经析构
	input.on_input.connect([bar = *bar](int i) {	// 但内部深拷贝了一份数据根外面析构的没有关系
		bar.on_input(i);
		});
	*/
	/*
	// 第二种解决办法
	auto bar = std::make_shared<Bar>();	// 1
	input.on_input.connect([bar = std::make_shared<Bar>(bar)](int i) {	// 拷贝， 计数变为2
		bar->on_input(i);
		});
	// 出了}。计数变为1 ，OK，直到input对象析构了之后，Bar计数才变为0进行析构
	*/

	/* 综上 将bind 的参数变为智能指针写法 */
	auto bar = std::make_shared<Bar>();	// 1
	input.on_input.connect(bar, &Bar::on_input);	// 捕获智能指针

	// &bar = bar，所以使用sharedptr的时候不要捕获引用
	auto bar = std::make_shared<Bar>();	// 1
	input.on_input.connect([&bar = bar](int i) {	// 引用1
		bar->on_input(i);
		});	// 出了} 变为0 ，未定义行为

	// 修改FUN值捕获的使用
	auto bar = std::make_shared<Bar>();
	input.on_input.connect(FUNC(bar->on_input));

}/* 出了} ，计数0 -> ~Bar调用，还是未定义行为*/

void dummy(Input& input) {
	auto bar = std::make_shared<Bar>(); // 1 0
	std::weak_ptr<Bar>weak_bar = bar;	// 1 1	weak不延长生命周期
	//std::shared_ptr<Bar>shared_bar = bar;//2 1
	input.on_input.connect([weak_bar](int i) {
		std::shared_ptr<Bar> shared_bar = weak_bar.lock();// 转为强指针，判断是否已经被析构
		if (shared_bar != nullptr) {
			shared_bar->on_input(i);
		}
		else {
			std::cout << "weak发现老家没了！\n";
		}
		});
}//[0 1]->~Bar()

int main() {
	Input input;
	Bar bar;
	Foo foo;
	input.on_input.connect([&](int i) {
		bar.on_input(i);
		});
	//input.on_input.connect([&](int i, int j) {
	//	foo.on_input(i, j);
	//	}); 
	input.on_exit.connect([&](std::string msg1, std::string msg2) {
		bar.on_exit(msg1, msg2);
		});

	// input.on_exit.connect(bind(&bar, &Bar::on_exit));	// 在外面定义的bind

	// 比qt效率高
	input.on_exit.connect(&bar, &Bar::on_exit);	// 在Signal结构体内部connect方法定义为模版，不用写bind

	// input.on_exit.connect(bar.on_exit);	// 能不能这样写，c++语法规定不行
	input.on_input.connect([&](auto ...t) { bar.on_input(t...); });	// OK

	input.on_exit.connect([&](auto ...t) { bar.on_exit(t...); }); // OK
	input.on_exit.connect(FUN(bar.on_exit));	// 封装为宏，OK

	input.on_exit.connect(test);	// 有重载的test不行，但是套了层宏之后OK
	// 需要指名
	input.on_exit.connect(static_cast<void(*)(std::string, std::string)>(test));
	// 太长套皮
	input.on_exit.connect(FUN(test));	//OK

	// 不套皮，定义结构体
	input.on_exit.connect(test_);	// OK

	//=============================================================
	dummy(input);
	input.main_loop();
	return 0;
}

// 如果使用unique
/*
C++23


==================================
Signal:

std::vector<std::move_only_function<void(T...)>> m_callbacks;

// 智能指针，与bind匹配
template <class Self, class MemFn>
void connect(Self self, MemFn memfn) {
	m_callbacks.push_back(bind(std::move(self), memfn));
}

void connect(std::move_only_function<void(T...)> callback) {
	m_callbacks.push_back(std::move(callback));
}

==================================

void dummy(Input& input) {
	auto bar = std::make_unique<Bar>();	// 1
	input.on_input.connect(std::move(bar), &Bar::on_input);	// 捕获智能指针
}

*/