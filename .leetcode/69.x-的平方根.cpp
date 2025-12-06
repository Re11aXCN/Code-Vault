#include <immintrin.h>
/*
 * @lc app=leetcode.cn id=69 lang=cpp
 *
 * [69] x 的平方根 
 */

// @lc code=start
class Solution {
public:
    double sqrt_exp_log2(double x) {
        if (x < 0) return NAN;
        if (x == 0) return 0.0;
        if (x == 1) return 1.0;
        
        return std::exp2(0.5 * std::log2(x)) + 1e-12;
    }
    double sqrt_newton(double x) {
        if (x < 0) return NAN;
        if (x == 0) return 0;
        
        double guess = x / 2.0;
        double prev = 0.0;
        
        while (std::abs(guess - prev) > 1e-10) {
            prev = guess;
            guess = 0.5 * (guess + x / guess);
        }
        
        return guess;
    }
    #if defined(__clang__) || defined(__GNUC__)
    double sqrt_inner(double x) {
        return __builtin_sqrt(x);
    }
    #endif
    void sqrt_newton_sse(float* input, float* output, int n, int iterations = 3) {
        const __m128 half = _mm_set1_ps(0.5f);
        
        for (int i = 0; i < n; i += 4) {
            __m128 x = _mm_load_ps(&input[i]);
            
            // 初始猜测：使用快速近似
            __m128 guess = _mm_mul_ps(x, _mm_set1_ps(0.5f));
            
            // 牛顿迭代：guess = 0.5 * (guess + x / guess)
            for (int iter = 0; iter < iterations; iter++) {
                __m128 x_div_guess = _mm_div_ps(x, guess);  // x / guess
                __m128 sum = _mm_add_ps(guess, x_div_guess);  // guess + x/guess
                guess = _mm_mul_ps(half, sum);  // 0.5 * (guess + x/guess)
            }
            
            _mm_store_ps(&output[i], guess);
        }
    }
    float sqrt_rsqrt_ss(float n) {
        // 1. 将标量float加载到SSE寄存器
        __m128 v = _mm_set_ss(n);
        // 2. 计算倒数平方根的近似值
        __m128 rsqrt_val = _mm_rsqrt_ss(v);
        // 3. 在寄存器中直接相乘：sqrt(n) ≈ n * (1/sqrt(n))
        __m128 sqrt_approx = _mm_mul_ss(v, rsqrt_val);
        // 4. 将结果寄存器的低32位转换回标量float[citation:3]
        return _mm_cvtss_f32(sqrt_approx);
    }
    float fast_sqrt_with_one_refinement(float n) {
        // 1. 加载输入值到SSE寄存器
        __m128 x = _mm_set_ss(n);
        // 2. 计算初始的倒数平方根近似值 (精度较低)
        __m128 y0 = _mm_rsqrt_ss(x);
        
        // --- 开始一次牛顿迭代以提升精度 ---
        // 牛顿迭代公式: y1 = y0 * (1.5 - 0.5 * x * y0 * y0)
        // 其中 y0 是 1/sqrt(x) 的初始近似值
        
        // 3. 计算 x * y0 * y0
        __m128 x_times_y0_squared = _mm_mul_ss(x, _mm_mul_ss(y0, y0));
        
        // 4. 准备常数：0.5 和 1.5
        // _mm_set_ss 创建只有一个有效元素的向量，其余三个元素会被忽略
        __m128 half = _mm_set_ss(0.5f);
        __m128 one_half = _mm_set_ss(1.5f);
        
        // 5. 计算括号内的部分: (1.5 - 0.5 * x * y0 * y0)
        __m128 inner = _mm_sub_ss(one_half, _mm_mul_ss(half, x_times_y0_squared));
        
        // 6. 完成迭代: y1 = y0 * inner
        // y1 现在是精度更高的 1/sqrt(x) 近似值
        __m128 y1 = _mm_mul_ss(y0, inner);
        
        // 7. 转换为平方根: sqrt(x) = x * (1/sqrt(x))
        __m128 sqrt_result = _mm_mul_ss(x, y1);
        
        // 8. 将结果转换回标量float并返回
        return _mm_cvtss_f32(sqrt_result);
    }
    double sqrt_sqrt_ss(double n) {
        __m128 rsqrt_val = _mm_set_sd(n);
        rsqrt_val = _mm_sqrt_sd(_mm_setzero_pd(), rsqrt_val);
        return _mm_cvtsd_f64(rsqrt_val);
    }
    int mySqrt(int x) {
        return sqrt_inner(x);
    }
};
// @lc code=end

