#pragma once
#include <immintrin.h>
#include <tbb/concurrent_vector.h>
#include <tbb/enumerable_thread_specific.h>
#include <tbb/parallel_for.h>

#include <cstddef>
#include <memory_resource>
#include <random>

#include "../stdex/memory/no_initialized_pod.hpp"
#include "../stdex/profiling/ticktock.hpp"

namespace avx {
/**
 * @brief AVX向量化过滤器，用于高效过滤数组元素
 *
 * 使用AVX2指令集和预计算的查找表实现高性能向量化过滤
 */
class AVXFilter
{
private:

  struct MaskLookupTable
  {
    __m256i permutation [256];  // 排列索引表
    __m256i mask [256];         // 存储掩码表

    MaskLookupTable();
  };

  struct PlainMaskLookupTable
  {
    __m256i masklut [512];

    PlainMaskLookupTable();
  };

  static const MaskLookupTable&      getLookupTable();
  static const PlainMaskLookupTable& getPlainLookupTable();

public:

  enum class ComparisonOperator
  {
    GreaterThan  = _CMP_GT_OQ,
    LessThan     = _CMP_LT_OQ,
    Equal        = _CMP_EQ_OQ,
    GreaterEqual = _CMP_GE_OQ,
    LessEqual    = _CMP_LE_OQ
  };

  /**
   * @brief 过滤数组中的元素
   *
   * @tparam Operator 比较操作符
   * @param input 输入数组
   * @param count 元素数量
   * @param threshold 比较阈值
   * @param output 输出数组（必须有足够空间）
   * @return size_t 输出元素数量
   */
  template<ComparisonOperator Operator>
  static size_t filter(const float* input, size_t count, float threshold,
                       float* output);

  template<ComparisonOperator Operator>
  static size_t filterPlain(const float* input, size_t count, float threshold,
                            float* output);

  /**
   * @brief 过滤大于阈值的元素
   */
  static size_t filterGreaterThan(const float* input, size_t count,
                                  float threshold, float* output);

  /**
   * @brief 过滤小于阈值的元素
   */
  static size_t filterLessThan(const float* input, size_t count,
                               float threshold, float* output);
  static size_t filterPlainLessThan(const float* input, size_t count,
                                    float threshold, float* output);

  /**
   * @brief 过滤等于阈值的元素
   */
  static size_t filterEqual(const float* input, size_t count, float threshold,
                            float* output);

private:

  template<int ComparisonCode>
  static size_t filterImpl(const float* input, size_t count, float threshold,
                           float* output);

  template<int ComparisonCode>
  static size_t filterPlainImpl(const float* input, size_t count,
                                float threshold, float* output);
};
}  // namespace avx

static void avx_filter_test(const std::vector<float>& scores)
{
  TICK(avx_filter);

  // 开始过滤出 60 分以下的学生，进行批评教育
  tbb::concurrent_vector<float> bad_scores;
  tbb::enumerable_thread_specific<std::pmr::unsynchronized_pool_resource>
      pool_ets;
  tbb::parallel_for(tbb::blocked_range<size_t>(0, scores.size(), 65536 * 32),
                    [&](tbb::blocked_range<size_t> r) {
#if 1
    auto&                                            pool = pool_ets.local();
    std::pmr::vector<stdex::NoInitializedPod<float>> local_bad_scores { &pool };
#else
    std::vector<NoInitializedPod<float>> local_bad_scores;
#endif
    local_bad_scores.resize(r.size());
#if 1
    size_t n = avx::AVXFilter::filterLessThan((float*)scores.data() + r.begin(),
                                              r.size(), 60,
                                              (float*)local_bad_scores.data());
#else
    size_t n = 0;
    for (size_t i = r.begin(); i != r.end(); ++i) {
      float score = scores [i];
      if (score < 60) { local_bad_scores [n++] = score; }
    }
#endif
    local_bad_scores.resize(n);
    std::copy(local_bad_scores.begin(), local_bad_scores.end(),
              bad_scores.grow_by(local_bad_scores.size()));
  });
  TOCK(avx_filter);

  // 验证过滤结果是否正确
  bool   has_invalid_score = false;
  float  invalid_score     = 0.0f;
  size_t invalid_index     = 0;

  // 方法1: 使用并行查找是否有大于等于60的分数
  tbb::parallel_for(tbb::blocked_range<size_t>(0, bad_scores.size()),
                    [&](const tbb::blocked_range<size_t>& r) {
    for (size_t i = r.begin(); i != r.end(); ++i) {
      if (bad_scores [i] >= 60.0f) {
        // 发现无效分数，记录信息
#pragma omp critical
        {
          has_invalid_score = true;
          invalid_score     = bad_scores [i];
          invalid_index     = i;
        }
        break;  // 找到一个就退出这个线程的循环
      }
    }
  });

  // 方法2: 也可以使用标准算法（单线程）
  /*
  auto it = std::find_if(bad_scores.begin(), bad_scores.end(),
                        [](float score) { return score >= 60.0f; });
  if (it != bad_scores.end()) {
      has_invalid_score = true;
      invalid_score = *it;
      invalid_index = std::distance(bad_scores.begin(), it);
  }
  */

  if (has_invalid_score) {
    std::cout << "❌ 验证失败: 在 bad_scores 中发现无效分数 " << invalid_score
              << " >= 60, 位置: " << invalid_index << std::endl;
    std::cout << "bad_scores 大小: " << bad_scores.size() << std::endl;

    // 可选: 检查附近的几个值
    size_t start = (invalid_index > 5) ? invalid_index - 5 : 0;
    size_t end   = (invalid_index + 5 < bad_scores.size()) ? invalid_index + 5
                                                           : bad_scores.size();
    std::cout << "附近的值: ";
    for (size_t i = start; i <= end; ++i) {
      std::cout << bad_scores [i] << " ";
    }
    std::cout << std::endl;
  } else {
    std::cout << "✅ 验证通过: 所有 " << bad_scores.size()
              << " 个分数都正确低于60分" << std::endl;

    // 额外验证: 检查原始数据中应该被过滤的分数是否真的都被过滤了
    size_t expected_bad_count = 0;
    for (size_t i = 0; i < scores.size(); ++i) {
      if (scores [i] < 60.0f) { expected_bad_count++; }
    }

    if (bad_scores.size() == expected_bad_count) {
      std::cout << "✅ 数量验证通过: 过滤出的分数数量 " << bad_scores.size()
                << " 与预期 " << expected_bad_count << " 一致" << std::endl;
    } else {
      std::cout << "⚠️  数量不一致: 过滤出 " << bad_scores.size()
                << " 个分数，但预期有 " << expected_bad_count << " 个"
                << std::endl;
    }
  }
}

static void avx_filter_plain_test(const std::vector<float>& scores)
{
  TICK(avx_filter);

  // 开始过滤出 60 分以下的学生，进行批评教育
  tbb::concurrent_vector<float> bad_scores;
  tbb::enumerable_thread_specific<std::pmr::unsynchronized_pool_resource>
      pool_ets;
  tbb::parallel_for(tbb::blocked_range<size_t>(0, scores.size(), 65536 * 32),
                    [&](tbb::blocked_range<size_t> r) {
#if 1
    auto&                                            pool = pool_ets.local();
    std::pmr::vector<stdex::NoInitializedPod<float>> local_bad_scores { &pool };
#else
    std::vector<NoInitializedPod<float>> local_bad_scores;
#endif
    local_bad_scores.resize(r.size());
#if 1
    size_t n = avx::AVXFilter::filterPlainLessThan(
        (float*)scores.data() + r.begin(), r.size(), 60,
        (float*)local_bad_scores.data());
#else
    size_t n = 0;
    for (size_t i = r.begin(); i != r.end(); ++i) {
      float score = scores [i];
      if (score < 60) { local_bad_scores [n++] = score; }
    }
#endif
    local_bad_scores.resize(n);
    std::copy(local_bad_scores.begin(), local_bad_scores.end(),
              bad_scores.grow_by(local_bad_scores.size()));
  });
  TOCK(avx_filter);

  // 验证过滤结果是否正确
  bool   has_invalid_score = false;
  float  invalid_score     = 0.0f;
  size_t invalid_index     = 0;

  // 方法1: 使用并行查找是否有大于等于60的分数
  tbb::parallel_for(tbb::blocked_range<size_t>(0, bad_scores.size()),
                    [&](const tbb::blocked_range<size_t>& r) {
    for (size_t i = r.begin(); i != r.end(); ++i) {
      if (bad_scores [i] >= 60.0f) {
        // 发现无效分数，记录信息
#pragma omp critical
        {
          has_invalid_score = true;
          invalid_score     = bad_scores [i];
          invalid_index     = i;
        }
        break;  // 找到一个就退出这个线程的循环
      }
    }
  });

  // 方法2: 也可以使用标准算法（单线程）
  /*
  auto it = std::find_if(bad_scores.begin(), bad_scores.end(),
                        [](float score) { return score >= 60.0f; });
  if (it != bad_scores.end()) {
      has_invalid_score = true;
      invalid_score = *it;
      invalid_index = std::distance(bad_scores.begin(), it);
  }
  */

  if (has_invalid_score) {
    std::cout << "❌ 验证失败: 在 bad_scores 中发现无效分数 " << invalid_score
              << " >= 60, 位置: " << invalid_index << std::endl;
    std::cout << "bad_scores 大小: " << bad_scores.size() << std::endl;

    // 可选: 检查附近的几个值
    size_t start = (invalid_index > 5) ? invalid_index - 5 : 0;
    size_t end   = (invalid_index + 5 < bad_scores.size()) ? invalid_index + 5
                                                           : bad_scores.size();
    std::cout << "附近的值: ";
    for (size_t i = start; i <= end; ++i) {
      std::cout << bad_scores [i] << " ";
    }
    std::cout << std::endl;
  } else {
    std::cout << "✅ 验证通过: 所有 " << bad_scores.size()
              << " 个分数都正确低于60分" << std::endl;

    // 额外验证: 检查原始数据中应该被过滤的分数是否真的都被过滤了
    size_t expected_bad_count = 0;
    for (size_t i = 0; i < scores.size(); ++i) {
      if (scores [i] < 60.0f) { expected_bad_count++; }
    }

    if (bad_scores.size() == expected_bad_count) {
      std::cout << "✅ 数量验证通过: 过滤出的分数数量 " << bad_scores.size()
                << " 与预期 " << expected_bad_count << " 一致" << std::endl;
    } else {
      std::cout << "⚠️  数量不一致: 过滤出 " << bad_scores.size()
                << " 个分数，但预期有 " << expected_bad_count << " 个"
                << std::endl;
    }
  }
}

/*
使用两个数组的效率快于使用一个数组的效率，原因是：
单个数组需要进行计算移位运算获取索引然后访问
两个数组直接通过索引访问
*/
static void avx_test()
{
  std::vector<float> scores(65536 * 4096);
  // 随机填充学生成绩数据（0～100）
  tbb::parallel_for(tbb::blocked_range<size_t>(0, scores.size()),
                    [&](tbb::blocked_range<size_t> r) {
    std::mt19937                          rng(r.begin());
    std::uniform_real_distribution<float> uni(0, 100);
    for (size_t i = r.begin(); i != r.end(); ++i) {
      scores [i] = uni(rng);
    }
  });
  avx_filter_test(scores);

  std::cout << std::endl;

  avx_filter_plain_test(scores);
}