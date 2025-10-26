#pragma once

#include <cstdint>
#include <random>

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