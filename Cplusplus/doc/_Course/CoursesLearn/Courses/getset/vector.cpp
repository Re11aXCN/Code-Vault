#include <print>
#include <cstddef>

struct Vector {
private:
	int* m_data;
	size_t m_size;

public:
	Vector() : m_data(new int[4]), m_size(4) {}

	// 内部处理了，使得size和data重新变为合法状态
	// 调用之前/之后会使得成员变量变为非法状态的应该设置为private
	void setSize(size_t newSize) {
		m_size = newSize;
		delete[] m_data;
		m_data = new int[newSize];
	}

	int* data() const {
		return m_data;
	}

	size_t size() const {
		return m_size;
	}
};

int main() {
	Vector v;
	v.setSize(14);
	v.setSize(11);
	return 0;
}
