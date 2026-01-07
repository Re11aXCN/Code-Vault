#include <immintrin.h>

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

// 原有的雷神之锤相关函数保持不变
static float log2_sqrt_quake(float x)
{
  // 计算 log₂(sqrt(x)) = 0.5 * log₂(x)

  // 将浮点数按位解释为整数
  uint32_t i;
  std::memcpy(&i, &x, sizeof(i));

  // 提取指数部分 (8位) 和尾数部分 (23位)
  uint32_t exponent = (i >> 23) & 0xFF;
  uint32_t mantissa = i & 0x7F'FF'FF;

  // 如果输入是0或负数，返回错误值（实际情况中应该处理边界条件）
  if (x <= 0.0f) return -1000.0f;  // 错误值

  // 关键魔法：利用浮点数的对数性质
  // log₂(x) ≈ exponent - 127 + mantissa/2²³
  // 所以 log₂(sqrt(x)) = 0.5 * log₂(x) ≈ 0.5 * (exponent - 127) + 0.5 *
  // mantissa/2²³

  // 使用整数运算近似计算
  // 将指数部分减127然后右移1位（除以2）
  int32_t exp_part = (static_cast<int32_t>(exponent) - 127) >> 1;

  // 尾数部分也右移1位，但要考虑进位
  uint32_t mant_part = mantissa >> 1;

  // 组合结果：这里我们构造一个近似的浮点数
  // 注意：这只是一个近似值，需要后续处理

  // 更简单的方法：直接操作原始位模式
  uint32_t result_bits = i;

  // 关键步骤：将指数部分减半（相当于开平方的对数）
  // 通过右移1位实现，但要加上适当的偏移
  result_bits = (result_bits >> 1) + 0x1F'40'00'00;  // 魔法常数

  // 将结果转换回浮点数
  float result;
  std::memcpy(&result, &result_bits, sizeof(result));

  return result;
}

static float log2_sqrt_quake_precise(float x)
{
  // 第一步：快速近似
  uint32_t i;
  std::memcpy(&i, &x, sizeof(i));

  // 魔法常数：基于浮点数格式的经验值
  // 这个常数通过最小化误差得到
  uint32_t result_bits = (i >> 1) + 0x1F'40'00'00;

  float approx;
  std::memcpy(&approx, &result_bits, sizeof(approx));

  // 第二步：使用一次牛顿迭代提高精度
  // 我们要求解 y = 0.5 * log₂(x)
  // 设 f(y) = 2^y - sqrt(x) = 0
  // 牛顿迭代：y_{n+1} = y_n - f(y_n)/f'(y_n)
  // f'(y) = ln(2) * 2^y
  // 所以：y_{n+1} = y_n - (2^{y_n} - sqrt(x)) / (ln(2) * 2^{y_n})
  // 简化：y_{n+1} = y_n - (1 - sqrt(x)/2^{y_n}) / ln(2)

  const float ln2 = 0.69314718056f;

  // 计算 2^approx
  float two_power_approx = std::exp2(approx);

  // 计算 sqrt(x)（这里用标准库，实际可以用雷神之锤的平方根倒数）
  float sqrt_x = std::sqrt(x);

  // 牛顿迭代
  approx = approx - (1.0f - sqrt_x / two_power_approx) / ln2;

  return approx;
}

static float log2_sqrt_pure_integer_style(float x)
{
  // 完全避免浮点除法的版本

  uint32_t i;
  std::memcpy(&i, &x, sizeof(i));

  // 提取浮点数的各个部分
  int32_t  exponent = ((i >> 23) & 0xFF) - 127;  // 有符号指数
  uint32_t mantissa = i & 0x7F'FF'FF;

  // 计算 log₂(sqrt(x)) = 0.5 * (exponent + log₂(1 + m/2²³))
  // 其中 m 是尾数

  // 近似：log₂(1 + t) ≈ t + 0.0430 * t * (1 - t)
  // 其中 t = m/2²³

  // 使用整数运算避免浮点除法
  uint64_t t_scaled = static_cast<uint64_t>(mantissa) << 17;  // t * 2^40

  // 计算 t * (1 - t) 的近似
  uint64_t t_one_minus_t =
      (t_scaled * (0x1'00'00'00'00'00ULL - t_scaled)) >> 40;

  // log₂(1 + t) ≈ t + 0.0430 * t*(1-t)
  uint64_t log2_1pt_scaled =
      t_scaled + ((t_one_minus_t * 0xB2ULL) >> 8);  // 0.0430 ≈ 0xB2/2^12

  // 最终结果 = 0.5 * exponent + 0.5 * log₂(1 + t)
  // 使用整数运算避免浮点除法
  int32_t result_int =
      (exponent << 11) + static_cast<int32_t>(log2_1pt_scaled >> 29);

  // 转换为浮点数
  float result = static_cast<float>(result_int) / 2048.0f;  // 2^11

  return result;
}

// 新的函数：将log2(sqrt(x))转换为实际的平方根值
static float quake_log_to_sqrt(float x, float log2_sqrt_x)
{
  // sqrt(x) = 2^(log2(sqrt(x)))
  return std::exp2(log2_sqrt_x);
}

// 经典的雷神之锤平方根倒数算法
static float quake_inv_sqrt(float x)
{
  float xhalf = 0.5f * x;
  int   i     = *(int*)&x;                     // 将浮点数按位解释为整数
  i           = 0x5f'37'59'df - (i >> 1);      // 魔法步骤
  float y     = *(float*)&i;                   // 将整数按位解释回浮点数
  y           = y * (1.5f - (xhalf * y * y));  // 一次牛顿迭代
  // y = y * (1.5f - (xhalf * y * y)); // 第二次牛顿迭代（可选）
  return y;
}

// 基于雷神之锤平方根倒数的平方根计算
static float quake_sqrt(float x)
{
  if (x <= 0.0f) return 0.0f;
  float inv_sqrt = quake_inv_sqrt(x);
  return x * inv_sqrt;  // sqrt(x) = x * (1/sqrt(x))
}

class SqrtBenchmark
{
private:

  std::vector<float>             data;
  std::vector<float>             result;
  inline static constexpr size_t size       = 1024;
  int                            iterations = 10000;

public:

  SqrtBenchmark() : data(size), result(size)
  {
    // 初始化测试数据，避免0和负数
    for (size_t i = 0; i < size; ++i) {
      data [i] = 1.0f + static_cast<float>(i) / size;  // 1.0 ~ 2.0
    }
  }

  // 原有的测试方法...

  // 方法1: 使用std::sqrt逐个计算
  double test_std_sqrt()
  {
    auto start = std::chrono::high_resolution_clock::now();

    for (int iter = 0; iter < iterations; ++iter) {
      for (size_t i = 0; i < size; ++i) {
        result [i] = std::sqrt(data [i]);
      }
    }

    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end - start).count();
  }
#ifdef __linux__
  // 方法2: 使用__builtin_sqrt逐个计算
  double test_builtin_sqrt()
  {
    auto start = std::chrono::high_resolution_clock::now();

    for (int iter = 0; iter < iterations; ++iter) {
      for (size_t i = 0; i < size; ++i) {
        result [i] = __builtin_sqrt(data [i]);
      }
    }

    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end - start).count();
  }
#endif  //
  // 方法3: 使用SSE指令集 (_mm_sqrt_ps)
  double test_sse_sqrt()
  {
    // 确保数据是16字节对齐的
    alignas(16) std::vector<float> aligned_data(size);
    alignas(16) std::vector<float> aligned_result(size);
    std::copy(data.begin(), data.end(), aligned_data.begin());

    auto start = std::chrono::high_resolution_clock::now();

    for (int iter = 0; iter < iterations; ++iter) {
      for (size_t i = 0; i < size; i += 4) {
        __m128 vec      = _mm_load_ps(&aligned_data [i]);
        __m128 sqrt_vec = _mm_sqrt_ps(vec);
        _mm_store_ps(&aligned_result [i], sqrt_vec);
      }
    }

    std::copy(aligned_result.begin(), aligned_result.end(), result.begin());
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end - start).count();
  }

  // 方法4: 使用AVX指令集 (_mm256_sqrt_ps)
  double test_avx_sqrt()
  {
    // 确保数据是32字节对齐的
    alignas(32) std::vector<float> aligned_data(size);
    alignas(32) std::vector<float> aligned_result(size);
    std::copy(data.begin(), data.end(), aligned_data.begin());

    auto start = std::chrono::high_resolution_clock::now();

    for (int iter = 0; iter < iterations; ++iter) {
      for (size_t i = 0; i < size; i += 8) {
        __m256 vec      = _mm256_load_ps(&aligned_data [i]);
        __m256 sqrt_vec = _mm256_sqrt_ps(vec);
        _mm256_store_ps(&aligned_result [i], sqrt_vec);
      }
    }

    std::copy(aligned_result.begin(), aligned_result.end(), result.begin());
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end - start).count();
  }

  // 方法5: 使用SSE指令集，但用_mm_loadu_ps处理非对齐数据
  double test_sse_unaligned_sqrt()
  {
    auto start = std::chrono::high_resolution_clock::now();

    for (int iter = 0; iter < iterations; ++iter) {
      for (size_t i = 0; i < size; i += 4) {
        __m128 vec      = _mm_loadu_ps(&data [i]);
        __m128 sqrt_vec = _mm_sqrt_ps(vec);
        _mm_storeu_ps(&result [i], sqrt_vec);
      }
    }

    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end - start).count();
  }

  // 方法6: 使用AVX指令集，但用_mm256_loadu_ps处理非对齐数据
  double test_avx_unaligned_sqrt()
  {
    auto start = std::chrono::high_resolution_clock::now();

    for (int iter = 0; iter < iterations; ++iter) {
      for (size_t i = 0; i < size; i += 8) {
        __m256 vec      = _mm256_loadu_ps(&data [i]);
        __m256 sqrt_vec = _mm256_sqrt_ps(vec);
        _mm256_storeu_ps(&result [i], sqrt_vec);
      }
    }

    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end - start).count();
  }

  // 新增的方法：

  // 方法7: 使用雷神之锤平方根倒数算法
  double test_quake_sqrt()
  {
    auto start = std::chrono::high_resolution_clock::now();

    for (int iter = 0; iter < iterations; ++iter) {
      for (size_t i = 0; i < size; ++i) {
        result [i] = quake_sqrt(data [i]);
      }
    }

    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end - start).count();
  }

  // 方法8: 使用log2_sqrt_quake + exp2
  double test_log2_quake_sqrt()
  {
    auto start = std::chrono::high_resolution_clock::now();

    for (int iter = 0; iter < iterations; ++iter) {
      for (size_t i = 0; i < size; ++i) {
        float log_val = log2_sqrt_quake(data [i]);
        result [i]    = std::exp2(log_val);
      }
    }

    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end - start).count();
  }

  // 方法9: 使用log2_sqrt_quake_precise + exp2
  double test_log2_quake_precise_sqrt()
  {
    auto start = std::chrono::high_resolution_clock::now();

    for (int iter = 0; iter < iterations; ++iter) {
      for (size_t i = 0; i < size; ++i) {
        float log_val = log2_sqrt_quake_precise(data [i]);
        result [i]    = std::exp2(log_val);
      }
    }

    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end - start).count();
  }

  // 方法10: 使用log2_sqrt_pure_integer_style + exp2
  double test_log2_pure_integer_sqrt()
  {
    auto start = std::chrono::high_resolution_clock::now();

    for (int iter = 0; iter < iterations; ++iter) {
      for (size_t i = 0; i < size; ++i) {
        float log_val = log2_sqrt_pure_integer_style(data [i]);
        result [i]    = std::exp2(log_val);
      }
    }

    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end - start).count();
  }

  // 方法11: 使用pow(x, 0.5)
  double test_pow_sqrt()
  {
    auto start = std::chrono::high_resolution_clock::now();

    for (int iter = 0; iter < iterations; ++iter) {
      for (size_t i = 0; i < size; ++i) {
        result [i] = std::pow(data [i], 0.5f);
      }
    }

    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end - start).count();
  }

  // 验证结果正确性 - 对于近似算法使用更大的容差
  bool validate_results(const std::vector<float>& reference,
                        bool                      is_approximate = false)
  {
    const float epsilon =
        is_approximate ? 1e-3f : 1e-6f;  // 近似算法使用更大的容差
    for (size_t i = 0; i < size; ++i) {
      if (std::abs(result [i] - reference [i]) > epsilon) {
        std::cout << "Validation failed at index " << i << ": expected "
                  << reference [i] << ", got " << result [i]
                  << ", diff = " << std::abs(result [i] - reference [i])
                  << std::endl;
        return false;
      }
    }
    return true;
  }

  void run_benchmark()
  {
    // 先计算一个参考结果用于验证
    std::vector<float> reference(size);
    for (size_t i = 0; i < size; ++i) {
      reference [i] = std::sqrt(data [i]);
    }

    std::cout << "性能测试结果 (1024个float, " << iterations << "次迭代):\n";
    std::cout << "==========================================\n";

    // 测试各种方法
    double time_std = test_std_sqrt();
    std::cout << "std::sqrt:           " << time_std << " 秒, "
              << "正确性: " << (validate_results(reference) ? "✓" : "✗")
              << "\n";
#ifdef __linux__
    double time_builtin = test_builtin_sqrt();
    std::cout << "__builtin_sqrt:      " << time_builtin << " 秒, "
              << "正确性: " << (validate_results(reference) ? "✓" : "✗")
              << "\n";
#endif  //

    double time_sse_aligned = test_sse_sqrt();
    std::cout << "SSE(_mm_sqrt_ps):    " << time_sse_aligned << " 秒, "
              << "正确性: " << (validate_results(reference) ? "✓" : "✗")
              << "\n";

    double time_sse_unaligned = test_sse_unaligned_sqrt();
    std::cout << "SSE(非对齐):        " << time_sse_unaligned << " 秒, "
              << "正确性: " << (validate_results(reference) ? "✓" : "✗")
              << "\n";

    double time_avx_aligned = test_avx_sqrt();
    std::cout << "AVX(_mm256_sqrt_ps): " << time_avx_aligned << " 秒, "
              << "正确性: " << (validate_results(reference) ? "✓" : "✗")
              << "\n";

    double time_avx_unaligned = test_avx_unaligned_sqrt();
    std::cout << "AVX(非对齐):        " << time_avx_unaligned << " 秒, "
              << "正确性: " << (validate_results(reference) ? "✓" : "✗")
              << "\n";

    // 新增的测试方法
    double time_pow = test_pow_sqrt();
    std::cout << "std::pow(x,0.5):    " << time_pow << " 秒, "
              << "正确性: " << (validate_results(reference) ? "✓" : "✗")
              << "\n";

    double time_quake = test_quake_sqrt();
    std::cout << "雷神之锤sqrt:       " << time_quake << " 秒, "
              << "正确性: " << (validate_results(reference, true) ? "✓" : "✗")
              << "\n";

    double time_log2_quake = test_log2_quake_sqrt();
    std::cout << "log2_quake+exp2:    " << time_log2_quake << " 秒, "
              << "正确性: " << (validate_results(reference, true) ? "✓" : "✗")
              << "\n";

    double time_log2_quake_precise = test_log2_quake_precise_sqrt();
    std::cout << "log2_quake_precise: " << time_log2_quake_precise << " 秒, "
              << "正确性: " << (validate_results(reference, true) ? "✓" : "✗")
              << "\n";

    double time_log2_pure_integer = test_log2_pure_integer_sqrt();
    std::cout << "log2_pure_integer:  " << time_log2_pure_integer << " 秒, "
              << "正确性: " << (validate_results(reference, true) ? "✓" : "✗")
              << "\n";

    std::cout << "==========================================\n";

    // 计算相对性能
    std::cout << "\n相对性能 (std::sqrt = 1.0):\n";
    std::cout << "std::sqrt:           1.00x\n";
#ifdef __linux__
    std::cout << "__builtin_sqrt:      " << (time_std / time_builtin) << "x\n";
#endif  //
    std::cout << "SSE(对齐):          " << (time_std / time_sse_aligned)
              << "x\n";
    std::cout << "SSE(非对齐):        " << (time_std / time_sse_unaligned)
              << "x\n";
    std::cout << "AVX(对齐):          " << (time_std / time_avx_aligned)
              << "x\n";
    std::cout << "AVX(非对齐):        " << (time_std / time_avx_unaligned)
              << "x\n";
    std::cout << "std::pow(x,0.5):    " << (time_std / time_pow) << "x\n";
    std::cout << "雷神之锤sqrt:       " << (time_std / time_quake) << "x\n";
    std::cout << "log2_quake+exp2:    " << (time_std / time_log2_quake)
              << "x\n";
    std::cout << "log2_quake_precise: " << (time_std / time_log2_quake_precise)
              << "x\n";
    std::cout << "log2_pure_integer:  " << (time_std / time_log2_pure_integer)
              << "x\n";
  }
};

/*
int main() {
    SqrtBenchmark benchmark;
    benchmark.run_benchmark();
    return 0;
}
*/

/*
* gcc -std=c++20 -O3 -mavx2
性能测试结果 (1024个float, 10000次迭代):
==========================================
std::sqrt:           0.0122837 秒, 正确性: ✓
__builtin_sqrt:      0.0121587 秒, 正确性: ✓
SSE(_mm_sqrt_ps):    0.00351034 秒, 正确性: ✓
SSE(非对齐):        0.00356184 秒, 正确性: ✓
AVX(_mm256_sqrt_ps): 0.00313857 秒, 正确性: ✓
AVX(非对齐):        0.00282225 秒, 正确性: ✓
std::pow(x,0.5):    0.0906588 秒, 正确性: ✓
雷神之锤sqrt:       0.0185715 秒, 正确性: Validation failed at index 0: expected
1, got 0.998307, diff = 0.00169283 ✗ log2_quake+exp2:    0.0509443 秒, 正确性:
Validation failed at index 0: expected 1, got 1.41421, diff = 0.414214 ✗
log2_quake_precise: 0.13048 秒, 正确性: Validation failed at index 0: expected
1, got 1.05515, diff = 0.0551473 ✗ log2_pure_integer:  0.080767 秒, 正确性:
Validation failed at index 6: expected 1.00293, got 1.00407, diff = 0.00114429 ✗
==========================================

相对性能 (std::sqrt = 1.0):
std::sqrt:           1.00x
__builtin_sqrt:      1.01027x
SSE(对齐):          3.49927x
SSE(非对齐):        3.44868x
AVX(对齐):          3.91377x
AVX(非对齐):        4.35243x
std::pow(x,0.5):    0.135493x
雷神之锤sqrt:       0.661424x
log2_quake+exp2:    0.241119x
log2_quake_precise: 0.0941417x
log2_pure_integer:  0.152088x

*/
