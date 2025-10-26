#include <cstdio>
#include <cstdlib>  // C语言 rand()函数
#include <ctime>    // srand(time(NULL))
#include <random>   // std::mt19937,mt是人名简写，要2的19937次方才出现重复的数字
#include <algorithm>
#include <iostream>
#include <numeric>

// xorshift32算法
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

// 最好的wangshash，处理并行非常棒
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
std::iota(vec.begin(),vec.end(), 0); // 给的0到99的数，绝对不重复，然后开启洗牌算法
std::mt19937 rng; // wangshash rng;
std::shuffle(vec.begin(),vec.end(), rng)
*/

int main() {
	/*
	std::random_device rnd; // 从cpu中获取的随机数，随机性更高，但是效率低些
	std::mt19937 rng(rnd());//效率低
	//==================
	std::mt19937 rng(time(NULL));   // 加上随机数种子，如果太快值还是一样，伪随机的
	std::uniform_int_distribution<int> uni(0, 100); // 指定随机数生成范围
	size_t num = uni(rng);  // 平均下来O(1)复杂度
	*/
	std::vector<float> gailv = { 0.6f, 0.25f, 0.05f, 0.1f };
	std::vector<float> gailv_scanned;   // = {0.6, 0.85, 0.9, 1.0}
	// 给每个gailv的数组元素生成概率
	std::inclusive_scan(gailv.begin(), gailv.end(), std::back_inserter(gailv_scanned));
	for (size_t i = 0; i < gailv_scanned.size(); i++) {
		printf("%f\n", gailv_scanned[i]);
	}
	std::vector<std::string> lian = { "mushi", "she", "enlosi", "wuya" };
	std::mt19937 rng{ std::random_device{}() };
	std::uniform_real_distribution<float> unf(0.0f, 1.0f);  // float x = rand() / (float)RAND_MAX;以前写法
	auto genlian = [&]() -> std::string {
		float f = unf(rng);
		// 二分法
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
