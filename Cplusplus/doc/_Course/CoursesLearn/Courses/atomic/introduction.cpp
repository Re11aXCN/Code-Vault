#include "mtpool.hpp"
using namespace std;

// atomic 有两个功能：原子 & 内存序
// 原子的作用：保证对一个变量操作期间，其他线程不会对此变量操作，那就称为原子操作
// 内存序的作用：构造一个事件发生先后的顺序关系

// 大多数普通的变量都不用是原子的，只有一个变量被多个线程同时访问（且其中一个是有副作用的写访问）时，才需要把它变成原子变量
// 所有其他变量，如果也设为原子的话，CPU 的压力会非常大，必须时时刻刻提心吊胆，编译器也会难以优化
// 我们只需要把少数多个线程同时访问的变量设为原子的就可以，帮 CPU 和编译器明确哪些是需要保证多线程并发访问安全的

//	x86 TSO 保证的内存序以下模型都一样，都是seq_cst
// CPU，缓存，编译器优化 -> 导致乱序
// 对少数关键变量的内存访问，指定内存序，可以阻断优化，避免这些关键变量的乱序
// 而普通的无关紧要的变量的访问依然可以乱序执行

// seq_cst = Sequentially-Consistent 顺序一致模型
// acq_rel = acquire + release
// reaxled = 无内存序，仅仅原子

// ARM RISC-V 弱内存模型

struct TestRelaxed {
	// volatile用于嵌入式可以，用在这里多线程还是可能出现数据竞争
	std::atomic_int data = 0;   // atomic_int线程安全
	// 只有多个线程都访问某一个变量才加atomic_int，如果不管什么数据都加atomic_int
	// 那么编译器得不到优化cpu会很忙

	void entry(MTIndex<0>) {  // 0 号线程
		data.store(42, std::memory_order_relaxed);
		/*
		memory_order_relaxed = memory_order::relaxed;
		memory_order_consume = memory_order::consume;
		memory_order_acquire = memory_order::acquire;
		memory_order_release = memory_order::release;
		memory_order_acq_rel = memory_order::acq_rel;
		memory_order_seq_cst = memory_order::seq_cst;
		*/
	}
	void entry(MTIndex<1>) {  // 1 号线程
		while (data.load(std::memory_order_relaxed) == 0)
			;
		MTTest::result = data;
	}
};

struct Test {

	int data = 0;
	std::atomic_int flag = 0;

	void entry(MTIndex<0>) {
		// 内存序
		data = 42;
		flag.store(1);	// 顺序一致模型，flag之前的数据都不可能优化到
		// flag.store(1，std::memory_order_seq_cst);	// memory_order_seq_cst会损失性能，因为store之前的数据缓存全部失效，重新读一遍

		/*
		未定义写法
		flag.store(1);
		// flag执行完，可能会被打断,data是0读不到42
		data = 42;
		*/

		/*
		设定为无内存序的写法，不能够保证顺序执行，因为data和flag不相关，没有依赖关系
		data = 42;
		flag.store(1，std::memory_order_relaxed);
		*/
	}
	void entry(MTIndex<1>) {
		while (flag.load() == 0)
			;
		MTTest::result = data;
	}
};

struct TestSeqCst {

	int data = 0;
	std::atomic_int flag = 0;

	void entry(MTIndex<0>) {
		data = 42;
		flag.store(1, std::memory_order_seq_cst);
	}
	void entry(MTIndex<1>) {
		while (flag.load(std::memory_order_seq_cst) == 0)
			;
		MTTest::result = data;
	}
};



struct TestRelAcq {

	int data = 0;
	std::atomic_int flag = 0;

	void entry(MTIndex<0>) {
		data = 42;
		// 保证写生效
		flag.store(1, std::memory_order_release);	// release说必须前面执行完，才到我
	}
	void entry(MTIndex<1>) {
		// 保证读生效
		while (flag.load(std::memory_order_acquire) == 0)// acquire说我执行完，前面release的数据必须传过来
			;
		MTTest::result = data;
	}
};

int main() {
	// 运行架构得是ARM的才能够出现不同情况
	// X64 X86 运行结果一致 都为10000
	MTTest::runTest<Test>();
	MTTest::runTest<TestRelaxed>();
	MTTest::runTest<TestRelAcq>();
	MTTest::runTest<TestSeqCst>();

	return 0;
}