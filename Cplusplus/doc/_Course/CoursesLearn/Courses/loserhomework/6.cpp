#include <format>
#include <vector>
#include <functional>
#include <stdexcept>
#include <iostream>
#include <string>
#include <atomic>

int main() {
	// c++17���Բ�����п���
	auto a = std::atomic<int>(5);
	// auto b = a; // error����ֹ��ͨ��load()���п�����ɾ���˿�������
	return 0;
}
