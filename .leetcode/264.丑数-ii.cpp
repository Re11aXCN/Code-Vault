/*
 * @lc app=leetcode.cn id=264 lang=cpp
 *
 * [264] 丑数 II
 */

// @lc code=start
class Solution {
    inline static constexpr unsigned CACHE_SIZE = 1690;
public:
    int nthUglyNumber(int n) {
        static std::array<int, CACHE_SIZE> ugly;
        ugly[0] = 1;
        int i2 = 0, i3 = 0, i5 = 0;
        #pragma clang loop unroll_count(8)
        for (int i = 1; i < n; ++i) {
            int next2 = ugly[i2] * 2;
            int next3 = ugly[i3] * 3;
            int next5 = ugly[i5] * 5;

            int min_val = std::min({next2, next3, next5});

            ugly[i] = min_val;

            if (min_val == next2) ++i2;
            if (min_val == next3) ++i3;
            if (min_val == next5) ++i5;
        }
        return ugly[n - 1];
    }

};
// @lc code=end

