#include <immintrin.h>
/*
 * @lc app=leetcode.cn id=69 lang=cpp
 *
 * [69] x 的平方根 
 */

// @lc code=start
class Solution {
public:
    int mySqrt(int x) {
        double res = static_cast<double>(x);
        __m128d sqrt_v = _mm_set_sd(res);
        sqrt_v = _mm_sqrt_sd(sqrt_v, sqrt_v);
        _mm_store_sd(&res, sqrt_v);
        return static_cast<int>(res);

        /*return __builtin_sqrt(x);*/
    }
};
// @lc code=end

