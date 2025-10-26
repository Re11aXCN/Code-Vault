#include "mtpool.hpp"
#include <mutex>
struct TestNaive {
	int data = 0;

	void entry(MTIndex<0>) {
		data += 1;
	}

	void entry(MTIndex<1>) {
		data += 1;
	}

	void teardown() {
		MTTest::result = data;
	}
};

struct TestMutex {
	int data = 0;
	std::mutex mutex;

	void entry(MTIndex<0>) {
		mutex.lock();
		data += 1;
		mutex.unlock();
	}

	void entry(MTIndex<1>) {
		mutex.lock();
		data += 1;
		mutex.unlock();
	}

	void teardown() {
		MTTest::result = data;
	}
};

struct SpinMutexWrong {
	bool try_lock() {
		int old = 0;
		if (flag.compare_exchange_strong(old, 1, std::memory_order_relaxed, std::memory_order_relaxed))
			// load barrier
			return true;
		return false;
	}

	void lock() {
		int old = 0;
		while (!flag.compare_exchange_weak(old, 1, std::memory_order_relaxed, std::memory_order_relaxed))
			old = 0;
		// load barrier
	}

	void unlock() {
		// data += 1
		// store barrier
		flag.store(0, std::memory_order_relaxed);
	}

	std::atomic<int> flag{ 0 };
};

struct TestSpinMutexWrong {
	int data = 0;
	char space[64];
	SpinMutexWrong mutex;

	void entry(MTIndex<0>) {
		mutex.lock();
		data += 1;
		mutex.unlock();
	}

	void entry(MTIndex<1>) {
		mutex.lock();
		data += 1;
		mutex.unlock();
	}

	void teardown() {
		MTTest::result = data;
	}
};

/*
自旋锁

*/

struct SpinMutex {
	bool try_lock() {
		int old = 0;
		// 原子变量一个方法体内不能够出现两次
		// compare_exchange_weak合并为一个写
		if (flag.compare_exchange_strong(old, 1, std::memory_order_acquire, std::memory_order_relaxed))
			// load barrier
			return true;
		return false;
	}

	void lock() {
		int old = 0;
		while (!flag.compare_exchange_weak(old, 1, std::memory_order_acquire, std::memory_order_relaxed))
			old = 0;
		// load barrier
	}

	void unlock() {
		// data += 1
		// store barrier
		flag.store(0, std::memory_order_release);
	}

	std::atomic<int> flag{ 0 };
};

struct TestSpinMutex {
	int data = 0;
	char space[64];	// 让data和mutex不在同一个缓存区，因为一个缓冲区64字节大小，如果SpinMutex设置的为(old, 1, std::memory_order_relaxed))那么执行顺序得不到保证，就是SpinMutexWrong
	SpinMutex mutex;

	void entry(MTIndex<0>) {
		mutex.lock();
		data += 1;
		mutex.unlock();
	}

	void entry(MTIndex<1>) {
		mutex.lock();
		data += 1;
		mutex.unlock();
	}

	void teardown() {
		MTTest::result = data;
	}
};

int main() {
	MTTest::runTest<TestNaive>();
	MTTest::runTest<TestMutex>();
	MTTest::runTest<TestSpinMutexWrong>();
	MTTest::runTest<TestSpinMutex>();
	return 0;
}
