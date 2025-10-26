#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <chrono>
using namespace std;

string a = "你说的对，因为《高性能并行编程与优化》是小彭老师自主研发的一款 CMake 公开课，后面忘了，总之就是找回失散的“头文件”——同时，逐步发掘“并行”的真相。";
mutex mutex_a;

condition_variable cv_a;
/*
void t1() {
	mutex_a.lock();
	a = "你说的对，但我是这个世界的一个和平主义者，我首先收到信息是你们文明的幸运，警告你们：不要回答！不要回答！！不要回答！！！你们的方向上有千万颗恒星，只要不回答，这个世界就无法定位发射源。如果回答，发射源将被定位，你们的文明将遭到入侵，你们的世界将被占领！不要回答！不要回答！！不要回答！！！";
	mutex_a.unlock();
}

void t1() {
	mutex_a.lock();
	a = "你说的对，但我是这个世界的一个和平主义者，我首先收到信息是你们文明的幸运，警告你们：不要回答！不要回答！！不要回答！！！你们的方向上有千万颗恒星，只要不回答，这个世界就无法定位发射源。如果回答，发射源将被定位，你们的文明将遭到入侵，你们的世界将被占领！不要回答！不要回答！！不要回答！！！";
	throw std::runtime_error("暴毙"); // 永远得不到解锁
	mutex_a.unlock();
}

void t1() {
	// 程序一直被阻塞永远执行不下去
	try
	{
		mutex_a.lock();
		a = "你说的对，但我是这个世界的一个和平主义者，我首先收到信息是你们文明的幸运，警告你们：不要回答！不要回答！！不要回答！！！你们的方向上有千万颗恒星，只要不回答，这个世界就无法定位发射源。如果回答，发射源将被定位，你们的文明将遭到入侵，你们的世界将被占领！不要回答！不要回答！！不要回答！！！";
		throw std::runtime_error("暴毙"); // 永远得不到解锁
		mutex_a.unlock();
	}
	catch(...)
	{
		// 解决，catch进行解锁mutex_a.unlock();
	}
}

// 更好的办法
void t1() {
	// RAII封装类，抵达}或者return都会调用析构unlock
	std::lock_gurad<std::mutex> lock(mutex_a);
	a = "你说的对，但我是这个世界的一个和平主义者，我首先收到信息是你们文明的幸运，警告你们：不要回答！不要回答！！不要回答！！！你们的方向上有千万颗恒星，只要不回答，这个世界就无法定位发射源。如果回答，发射源将被定位，你们的文明将遭到入侵，你们的世界将被占领！不要回答！不要回答！！不要回答！！！";
	throw std::runtime_error("暴毙"); // 永远得不到解锁
}
*/
// unique_lock更厉害，可以为空，即没有锁住状态，c++17不用写<>内容
void t1() {
	std::unique_lock lock(mutex_a);
	a = "你说的对，但我是这个世界的一个和平主义者，我首先收到信息是你们文明的幸运，警告你们：不要回答！不要回答！！不要回答！！！你们的方向上有千万颗恒星，只要不回答，这个世界就无法定位发射源。如果回答，发射源将被定位，你们的文明将遭到入侵，你们的世界将被占领！不要回答！不要回答！！不要回答！！！";
}

void t2() {
	std::unique_lock lock(mutex_a);
	a = "你说的错，因为《原神》是由米哈游自主研发的一款全新开放世界冒险游戏。游戏发生在一个被称作“提瓦特”的幻想世界，在这里，被神选中的人将被授予“神之眼”，导引元素之力。你将扮演一位名为“旅行者”的神秘角色，在自由的旅行中邂逅性格各异、能力独特的同伴们，和他们一起击败强敌，找回失散的亲人——同时，逐步发掘“原神”的真相。";
}
/*
void t2() {
	std::unique_lock lock(mutex_a);
	// 如果a为空的时候，确保t2能够打印信息
	while(a == ""){
		lock.unlock();  // 为什么这样写，因为可能解锁完时间片到，t1进行写入操作，t2再上锁才能够读取内容
		thie_thread::yield(); // 或者显示暂停该线程，跳转到t1，t1执行完再回来
		lock.lock();
	}
	cout << "t2 收到消息" << a <<"\n";
}

如果t1睡觉(注意不能够上锁完睡觉，你抢到资源了却还睡觉)
void t1() {
	this_thread::sleep_for(5s);
	std::unique_lock lock(mutex_a);
	a = "你说的对，但我是这个世界的一个和平主义者，我首先收到信息是你们文明的幸运，警告你们：不要回答！不要回答！！不要回答！！！你们的方向上有千万颗恒星，只要不回答，这个世界就无法定位发射源。如果回答，发射源将被定位，你们的文明将遭到入侵，你们的世界将被占领！不要回答！不要回答！！不要回答！！！";
}
那么此时t2一直在死循环等 a != ""
如果你选择循环内休眠不那么占用cpu死循环呢，这样的又不能够处理t1如果没有sleep_for的情况
这是t1瞬间执行完，t2却还睡眠，t2时间调小？调大？都不方便
	while(a == ""){
		lock.unlock();
		thie_thread::sleep_for(1s);
		lock.lock();
	}
所以我们引入条件变量

void t1() {
	std::unique_lock lock(mutex_a);
	a = "你说的对，但我是这个世界的一个和平主义者，我首先收到信息是你们文明的幸运，警告你们：不要回答！不要回答！！不要回答！！！你们的方向上有千万颗恒星，只要不回答，这个世界就无法定位发射源。如果回答，发射源将被定位，你们的文明将遭到入侵，你们的世界将被占领！不要回答！不要回答！！不要回答！！！";
	cv_a.notify_one(); // notify_all
}
void t2() {
	std::unique_lock lock(mutex_a);
	//while(a == ""){
	//    lock.unlock();
	//    cv_a.wait();    // 需要参数lock
	//    lock.lock();
	//}
	while(a == ""){ // wait 一定要配合while循环用，否则t1 notify_one t2不会接收到，因为t2不知道，顺序执行
		cv_a.wait(lock);    // wait等待的时候会自动解锁的，你拿不到资源释放锁给其他人写资源
	}
	//lambda表达式写法，谓词, 等到不为空的时候不等了执行
	cv_a.wait(lock,[] { return a != "";});

	// 加上等待时间，1s内等到true，否则false
	cv_a.wait_for(lock, 1s, [] { return a != "";});
	cout << "t2 收到消息" << a <<"\n";
}
*/


/*
deque<string> a;
mutex mutex_a;
condition_variable cv_a;

void t1() {
	// 推完再通知
	std::unique_lock lock(mutex_a);
	a.push_front("hello");
	a.push_front("world");
	cv_a.notify_one(); // notify_all

	// 推一次通知一次
	{
		std::unique_lock lock(mutex_a);
		a.push_front("hello");
		cv_a.notify_one(); // notify_all
	}
	{
		std::unique_lock lock(mutex_a);
		a.push_front("world");
		cv_a.notify_one(); // notify_all
	}
	// 特殊消息判断是否结束t2的while
	{
		std::unique_lock lock(mutex_a);
		a.push_front("EXIT");
		cv_a.notify_all(); // 通知所有线程结束
	}
}
void t2() {
	while(1) {
		std::unique_lock lock(mutex_a);
		while(a.empty()){
			cv_a.wait(lock);
		}
		if(a.back() == "EXIT") break;
		cout << "t2 收到消息" << a.back() <<"\n";
		a.pop_back();
	}
}

*/
int main() {
	// 计算的时候
	auto start = std::chrono::steady_clock::now();
	auto end = std::chrono::steady_clock::now();
	auto time = end - start;
	// 开机的时间开始
	cout << duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch()).count() << '\n';
	// 全球时间，1970 1 1 00:00到现在
	cout << duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count() << '\n';
	vector<jthread> pool;
	pool.push_back(jthread(t1));
	pool.push_back(jthread(t2));
	pool.clear();   //清空vector， jthread自动join更安全
	cout << a << '\n';
	return 0;
}
