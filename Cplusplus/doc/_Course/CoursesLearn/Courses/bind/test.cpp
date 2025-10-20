#include <functional>
#include <iostream>
#include <vector>

void (Bar::* memfn)(std::string, std::string);	// ����
void (*putfn)(std::string, std::string);		// ��Ա����

/*
// �󶨼�lambdaд��
template <class Self, class MemFn>
auto bind(Self* self, MemFn memfn) {
	return [self, memfn](auto ...t) { // define t
		(self->*memfn)(t...);
		};
}
*/

// ����ָ��汾
template <class Self, class MemFn>
auto bind(Self self, MemFn memfn) {
	return [self = std::move(self), memfn](auto ...t) { // define t
		((*self).*memfn)(t...);
		};
}
// weak�ػ�
template <class Self, class MemFn>
auto bind(std::weak_ptr<Self> self, MemFn memfn) {
	return [self = std::move(self), memfn](auto ...t) { // define t
		auto ptr = self.lock();
		if (ptr != nullptr) ((*ptr).*memfn)(t...);
		};
}

// �βΰ�
template <class ...T>
struct Signal {
	std::vector<std::function<void(T...)>> m_callbacks;

	/*
	template <class Self, class MemFn>
	void connect(Self* self, MemFn memfn) {
		m_callbacks.push_back(bind(self, memfn));
	}
	*/
	// ����ָ�룬��bindƥ��
	template <class Self, class MemFn>
	void connect(Self self, MemFn memfn) {
		m_callbacks.push_back(bind(std::move(self), memfn));
	}

	void connect(std::function<void(T...)> callback) {
		m_callbacks.push_back(std::move(callback));
	}

	void emit(T... t) {	// Tչ����t���壬
		for (auto&& callback : m_callbacks)
			callback(t...); // tչ��
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
// ���ף���������ת��
#define FUN(__fun) [&] (auto &&...__t) { return __fun(std::forward<decltype(__t)>(__t)...); } 
// ����shared����ָ������ֵ����
// __VA_OPT__��ʹ��������/C++/�����������		/Zc:preprocessor
#define FUNC(__fun, ...) [=] (auto &&...__t) { return __fun(__VA_ARGS__ __VA_OPT__(,) std::forward<decltype(__t)>(__t)...); } 
// __VA_OPT__(X):���__VA_ARGS__Ϊ�գ������������X��չ��
// __VA_OPT__(X) :���__VA_ARGS__��Ϊ�գ������������Xչ��

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
	// δ������Ϊ
	Bar bar;	// ��һ�ε������Ѿ�������������input.main_loop();����ѭ�����ã�δ������Ϊ
	input.on_input.connect(&bar, &Bar::on_input);
	*/

	/*
	// ʹ��shared_ptr ?
	auto bar = std::make_shared<Bar>();		// ����1
	input.on_input.connect(bar.get(), &Bar::on_input);	// getֻ�ӳٳ���һ��������û��
	*/
	/*
	// һ�ֽ���취
	auto bar = std::make_shared<Bar>();	// ��Ȼ�Ѿ�����
	input.on_input.connect([bar = *bar](int i) {	// ���ڲ������һ�����ݸ�����������û�й�ϵ
		bar.on_input(i);
		});
	*/
	/*
	// �ڶ��ֽ���취
	auto bar = std::make_shared<Bar>();	// 1
	input.on_input.connect([bar = std::make_shared<Bar>(bar)](int i) {	// ������ ������Ϊ2
		bar->on_input(i);
		});
	// ����}��������Ϊ1 ��OK��ֱ��input����������֮��Bar�����ű�Ϊ0��������
	*/

	/* ���� ��bind �Ĳ�����Ϊ����ָ��д�� */
	auto bar = std::make_shared<Bar>();	// 1
	input.on_input.connect(bar, &Bar::on_input);	// ��������ָ��

	// &bar = bar������ʹ��sharedptr��ʱ��Ҫ��������
	auto bar = std::make_shared<Bar>();	// 1
	input.on_input.connect([&bar = bar](int i) {	// ����1
		bar->on_input(i);
		});	// ����} ��Ϊ0 ��δ������Ϊ

	// �޸�FUNֵ�����ʹ��
	auto bar = std::make_shared<Bar>();
	input.on_input.connect(FUNC(bar->on_input));

}/* ����} ������0 -> ~Bar���ã�����δ������Ϊ*/

void dummy(Input& input) {
	auto bar = std::make_shared<Bar>(); // 1 0
	std::weak_ptr<Bar>weak_bar = bar;	// 1 1	weak���ӳ���������
	//std::shared_ptr<Bar>shared_bar = bar;//2 1
	input.on_input.connect([weak_bar](int i) {
		std::shared_ptr<Bar> shared_bar = weak_bar.lock();// תΪǿָ�룬�ж��Ƿ��Ѿ�������
		if (shared_bar != nullptr) {
			shared_bar->on_input(i);
		}
		else {
			std::cout << "weak�����ϼ�û�ˣ�\n";
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

	// input.on_exit.connect(bind(&bar, &Bar::on_exit));	// �����涨���bind

	// ��qtЧ�ʸ�
	input.on_exit.connect(&bar, &Bar::on_exit);	// ��Signal�ṹ���ڲ�connect��������Ϊģ�棬����дbind

	// input.on_exit.connect(bar.on_exit);	// �ܲ�������д��c++�﷨�涨����
	input.on_input.connect([&](auto ...t) { bar.on_input(t...); });	// OK

	input.on_exit.connect([&](auto ...t) { bar.on_exit(t...); }); // OK
	input.on_exit.connect(FUN(bar.on_exit));	// ��װΪ�꣬OK

	input.on_exit.connect(test);	// �����ص�test���У��������˲��֮��OK
	// ��Ҫָ��
	input.on_exit.connect(static_cast<void(*)(std::string, std::string)>(test));
	// ̫����Ƥ
	input.on_exit.connect(FUN(test));	//OK

	// ����Ƥ������ṹ��
	input.on_exit.connect(test_);	// OK

	//=============================================================
	dummy(input);
	input.main_loop();
	return 0;
}

// ���ʹ��unique
/*
C++23


==================================
Signal:

std::vector<std::move_only_function<void(T...)>> m_callbacks;

// ����ָ�룬��bindƥ��
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
	input.on_input.connect(std::move(bar), &Bar::on_input);	// ��������ָ��
}

*/