#include <cstdio>
#include <cstdlib>  // C���� rand()����
#include <ctime>    // srand(time(NULL))
#include <random>   // std::mt19937,mt��������д��Ҫ2��19937�η��ų����ظ�������
#include <algorithm>
#include <iostream>
#include <numeric>

// xorshift32�㷨
struct xorshift32 {
	uint32_t a;

	explicit xorshift32(size_t seed = 0) : a(static_cast<uint32_t>(seed + 1)) {
	}

	using result_type = uint32_t;

	constexpr uint32_t operator()() noexcept {
		uint32_t x = a;
		x ^= x << 13;
		x ^= x >> 17;
		x ^= x << 5;
		return a = x;
	}

	static constexpr uint32_t min() noexcept {
		return 1;
	}

	static constexpr uint32_t max() noexcept {
		return UINT32_MAX;
	}
};

/*
xorshift32 rng(std::random_device{}());
std::uniform_int_distribution<int> uni(0, 100);
 size_t num = uni(rng);

 std::vector<std::string> choices = {"apple", "peach", "cherry"};
 std::uniform_int_distribution<size_t> unic(0, choices.size() - 1);

*/

// ��õ�wangshash�������зǳ���
struct wangshash {
	uint32_t a;

	explicit wangshash(size_t seed = 0) : a(static_cast<uint32_t>(seed)) {
	}

	using result_type = uint32_t;

	constexpr uint32_t operator()() noexcept {
		uint32_t x = a;
		x = (x ^ 61) ^ (x >> 16);
		x *= 9;
		x = x ^ (x >> 4);
		x *= 0x27d4eb2d;
		x = x ^ (x >> 15);
		return a = x;
	}

	static constexpr uint32_t min() noexcept {
		return 0;
	}

	static constexpr uint32_t max() noexcept {
		return UINT32_MAX;
	}
};

/*
std::vector<int> vec(100);
std::generate(vec.begin(),vec.end(),[rng = wangshash(0), uni = std::uniform_int_distribution<int>(0,100)]() mutable {return uni(rng)};

std::vector<int> vec;
vec.reserve(100);
std::generate_n(std::back_inserter(vec),100,[rng = wangshash(0), uni = std::uniform_int_distribution<int>(0,100)]() mutable {return uni(rng)};

std::vector<int> vec(100);
std::iota(vec.begin(),vec.end(), 0); // ����0��99���������Բ��ظ���Ȼ����ϴ���㷨
std::mt19937 rng; // wangshash rng;
std::shuffle(vec.begin(),vec.end(), rng)
*/

int main() {
	/*
	std::random_device rnd; // ��cpu�л�ȡ�������������Ը��ߣ�����Ч�ʵ�Щ
	std::mt19937 rng(rnd());//Ч�ʵ�
	//==================
	std::mt19937 rng(time(NULL));   // ������������ӣ����̫��ֵ����һ����α�����
	std::uniform_int_distribution<int> uni(0, 100); // ָ����������ɷ�Χ
	size_t num = uni(rng);  // ƽ������O(1)���Ӷ�
	*/
	std::vector<float> gailv = { 0.6f, 0.25f, 0.05f, 0.1f };
	std::vector<float> gailv_scanned;   // = {0.6, 0.85, 0.9, 1.0}
	// ��ÿ��gailv������Ԫ�����ɸ���
	std::inclusive_scan(gailv.begin(), gailv.end(), std::back_inserter(gailv_scanned));
	for (size_t i = 0; i < gailv_scanned.size(); i++) {
		printf("%f\n", gailv_scanned[i]);
	}
	std::vector<std::string> lian = { "mushi", "she", "enlosi", "wuya" };
	std::mt19937 rng{ std::random_device{}() };
	std::uniform_real_distribution<float> unf(0.0f, 1.0f);  // float x = rand() / (float)RAND_MAX;��ǰд��
	auto genlian = [&]() -> std::string {
		float f = unf(rng);
		// ���ַ�
		auto it = std::lower_bound(gailv_scanned.begin(), gailv_scanned.end(), f);
		if (it == gailv_scanned.end()) return "";
		return lian[it - gailv_scanned.begin()];
		};
	std::cout << genlian() << '\n';
	std::cout << genlian() << '\n';
	std::cout << genlian() << '\n';
	std::cout << genlian() << '\n';
	std::cout << genlian() << '\n';
	std::cout << genlian() << '\n';
	std::cout << genlian() << '\n';
	std::cout << genlian() << '\n';
	std::cout << genlian() << '\n';
	std::cout << genlian() << '\n';
	std::cout << genlian() << '\n';
	std::cout << genlian() << '\n';
	std::cout << genlian() << '\n';
	std::cout << genlian() << '\n';
	std::cout << genlian() << '\n';
	return 0;
}
