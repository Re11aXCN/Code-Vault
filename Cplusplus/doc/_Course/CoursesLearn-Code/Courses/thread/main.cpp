#include "mtqueue.hpp"
#include <functional>
#include <iostream>
#include <thread>
#include <termios.h>
#include <vector>
#include <sstream>

using namespace std;
//�ܽ᣺û������ƿ��mt_queue�ķ����Ժ������Ժ�ά���Զ�����ѡ����������

/*
ʹ��С����ʦ��װ��mtqueue.hpp����
int counter = 1;
void compute(int beg, int end){
	 for (int i = beg; i < end; ++i) {
		counter += i; // �ڲ��߼���counter.store(counter.load()+i); ��ԭ�ӣ�Ҫôͨ������֤��������һ�㣩
		// Ҫô�������·���counter_thread��Ϣ����
	}
}

int main(){
	vector<jthread> pool;
	 for (int i = 0; i < 10000; i += 1000) {
		pool.push_back(jthread(compute, i, i + 1000));
	}
	pool.clear();
	cout << "counter: " << counter << "\n";
}
*/
/*
mt_queue<optional<int>> counter_queue;
// std::barrier finish(11);//10��compute����1��finish_thread
mt_queue<monostate> finish_queue;

//mt_queue<monostate> sem_queue(10); // ���ź�����

void counter_thread() {
	int counter = 0;
	while (true) {
		if (auto i = counter_queue.pop()) {
			counter += i.value();
		}
		else
			cout << "counter_thread �յ���Ϣ����\n";
		break;
	}
	cout << "����couter: " << counter << "\n";
}


void compute(int beg, int end) {
	for (int i = beg; i < end; ++i) {
		counter_queue.push(i);
		cout << "compute" << beg << "~" << end << "����������ȴ��󲿶ӻ�ʦ\n";
		finish_queue.push(monostate());
	}
}

void finish_thread() {
	cout << "finish_thread�ȴ���ʦ\n";
	for (int i = 0; i < 10; i++) {
		(void)finish_queue.pop();
	}
	cout << "finish_thread��ʦ�ɹ�\n";
	counter_queue.push(nullopt);
}
*/

struct CounterState {
	int counter = 0;
	string msg = "";
	bool finish = false;
};

void counter_thread(mt_queue<function<void(CounterState&)>>& counter_queue, mt_queue<int>& result_queue) {
	CounterState state;
	while (!state.finish) {
		auto task = counter_queue.pop();
		task(state);
	}
	ostringstream ss;
	ss << "msg: " << state.msg << '\n';
	ss << "counter: " << state.counter << '\n';
	cout << ss.str();
	result_queue.push(state.counter);
}

void compute(mt_queue<function<void(CounterState&)>>& counter_queue, int beg, int end) {
	for (int i = beg; i < end; ++i) {
		counter_queue.push([i](CounterState& state) {
			state.counter += i;
			});
	}
	counter_queue.push([](CounterState& state) {
		state.msg += "OK";
		});
}

int main() {
	mt_queue<function<void(CounterState&)>> counter_queue;
	mt_queue<int> result_queue;

	vector<jthread> compute_pool;
	vector<jthread> counter_pool;
	for (int i = 0; i < 10000; i += 1000) {
		compute_pool.push_back(jthread(compute, std::ref(counter_queue), i, i + 1000));
	}
	counter_pool.push_back(jthread(counter_thread, std::ref(counter_queue), std::ref(result_queue)));
	counter_pool.push_back(jthread(counter_thread, std::ref(counter_queue), std::ref(result_queue)));
	counter_pool.push_back(jthread(counter_thread, std::ref(counter_queue), std::ref(result_queue)));
	for (auto&& t : compute_pool) {
		t.join();
	}
	counter_queue.push([](CounterState& state) {
		state.finish = true;
		});
	counter_queue.push([](CounterState& state) {
		state.finish = true;
		});
	counter_queue.push([](CounterState& state) {
		state.finish = true;
		});
	for (auto&& t : counter_pool) {
		t.join();
	}
	int result = 0;
	for (int i = 0; i < 3; i++) {
		result += result_queue.pop();
	}
	cout << result << endl;
	return 0;
}
