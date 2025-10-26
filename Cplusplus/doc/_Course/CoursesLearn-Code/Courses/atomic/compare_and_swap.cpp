#include "mtpool.hpp"

struct TestNaive {
	int data = 2;

	void entry(MTIndex<0>) {
		data = data * data;
	}

	void entry(MTIndex<1>) {
		data = data * data;
	}

	void teardown() {
		MTTest::result = data;
	}
};

struct TestAtomicWrong {
	std::atomic<int> data = 2;

	void entry(MTIndex<0>) {
		data = data * data; // fetch没有乘法，以下的compare_exchange_strong、TestAtomicCAS等是小彭老师实现解决办法
		// 哪怕用了原子变量也不能够保证原子性
		// 等价于三个操作 data.store(data.load() + 1); data.store原子，data.load()原子，data.load() + 1不原子，data.store(data.load());不原子
		// data = data + data; 
		// data.store(data.load() * data.load()); // ERR
		// data.store(data.load() + 1); // ERR


		// data.fetch_add(1); // OK
	}

	void entry(MTIndex<1>) {
		data = data * data;
	}

	void teardown() {
		MTTest::result = data;
	}
};

// CAS 指令内部原理如下
bool compare_exchange_strong(std::atomic<int>& data, int& old_data, int new_data) {
	/* 以下相当于 lock_guard _(data); 是锁住的很安全*/
	if (data == old_data) {
		data = new_data;
		return true;
	}
	else {
		old_data = data;
		return false;
	}
}
bool compare_exchange_weak(std::atomic<int>& data, int& old_data, int new_data) {
	if (data == old_data && rand()) {
		data = new_data;
		return true;
	}
	else {
		old_data = data;
		return false;
	}
}

struct TestAtomicCAS { // compare-and-swap
	std::atomic<int> data = 2;

	void entry(MTIndex<0>) {
		auto old_data = data.load(std::memory_order_relaxed);
	again:
		auto new_data = old_data * old_data;
		if (!data.compare_exchange_strong(old_data, new_data, std::memory_order_relaxed))
			goto again;
	}

	void entry(MTIndex<1>) {
		auto old_data = data.load(std::memory_order_relaxed);
	again:
		auto new_data = old_data * old_data;
		if (!data.compare_exchange_strong(old_data, new_data, std::memory_order_relaxed))
			goto again;
	}

	void teardown() {
		MTTest::result = data;
	}
};

struct TestAtomicCASWeak { // compare-and-swap
	std::atomic<int> data = 2;

	void entry(MTIndex<0>) {
		auto old_data = data.load(std::memory_order_relaxed);
	again:
		auto new_data = old_data * old_data;
		if (!data.compare_exchange_weak(old_data, new_data, std::memory_order_relaxed))
			goto again;
	}

	void entry(MTIndex<1>) {
		auto old_data = data.load(std::memory_order_relaxed);
	again:
		auto new_data = old_data * old_data;
		if (!data.compare_exchange_weak(old_data, new_data, std::memory_order_relaxed))
			goto again;
	}

	void teardown() {
		MTTest::result = data;
	}
};

int main() {
	MTTest::runTest<TestNaive>();
	MTTest::runTest<TestAtomicWrong>();
	MTTest::runTest<TestAtomicCAS>();
	MTTest::runTest<TestAtomicCASWeak>();
	return 0;
}
