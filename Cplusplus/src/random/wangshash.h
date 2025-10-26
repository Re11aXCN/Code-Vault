#pragma once

#include <cstdint>
#include <random>

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