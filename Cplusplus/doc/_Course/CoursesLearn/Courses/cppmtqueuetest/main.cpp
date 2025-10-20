#include <condition_variable>
#include <vector>
#include <format>
#include <iostream>
#include <queue>
#include <stack>
#include <thread>

// std::stack<int> q; // 栈，先进后出 (FILO)
std::queue<int> q; // 队列，先进先出 (FIFO)
std::mutex mtx;
std::condition_variable cv;

/*
锁是为了防止圣诞老人和小孩同时操作，一个盒子伸一个手，圣诞老人放礼物，
小孩不能够取礼物，保证秩序

圣诞老人六点零一分送礼物，小孩六点到没看到有礼物走了，过一个小时再来
圣诞老人放礼物很快不到一分钟就放完，但是小孩得到七点才能够拿取，59分钟浪费
所以引入条件变量
圣诞老人送礼物到了之后会发送一个电信号，小朋友在家里坐在沙发看电视，电信号
会传递到沙发电小朋友跳了起来让小朋友知道礼物送到了，去拿
cv.notify_one() / cv.notify_all()
*/

int pop_gift() {
	std::unique_lock lk(mtx); // 等价于mtx.lock();mtx.unlock()
	cv.wait(lk, [] { return !q.empty(); }); // 礼物为空就一直等待
	auto gift = q.front();  // 得先取出数据在pop
	if (gift != 0)
		q.pop();    // void
	return gift;
}

void push_gift(int i) {
	std::unique_lock lk(mtx);   // lk出了}自动析构
	q.push(i);
	lk.unlock();
	cv.notify_one();
}

void producer() {
	for (int i = 1; i < 100; i++) {
		push_gift(i);
	}
	push_gift(0);
}

void consumer(int id) {
	while (int gift = pop_gift()) { // gift != 0

		// cout的原子性仅限于单个<<，还有一个<<std::endl就不支持了
		// 控制台打印会出现"我得到了44我得到了45"，没有换行，用\n
		//reinterpret_cast<const char*>(u8"小朋友 {} 得到了 {}\n")
		std::cout << std::format("小朋友 {} 得到了 {}\n", id, gift);
	}
}

int main() {
	/*
	生产者和消费者会并行执行
	std:thread producer_thread(producer);
	std:thread consumer_thread(consumer);
	producer_thread.join();
	consumer_thread.join();

	以下写法错误，这样写相当于单线程，等于没用
	std:thread producer_thread(producer);
	producer_thread.join();
	std:thread consumer_thread(consumer);
	consumer_thread.join();
	*/


	// 线程构造接收一个函数实例，或者lambda表达式
	// join等待，jthread不用写join，出了}会自动调用，vector也会自动析构然后调用join，方便
	std::jthread producer1_thread(producer);
	std::vector<std::jthread> consumer_threads; // 小朋友列表
	for (int i = 0; i < 32; i++) {
		std::jthread consumer_thread(consumer, i);
		// move consumer_thread被销毁为nullptr
		consumer_threads.push_back(std::move(consumer_thread));
	}
	return 0;
}
