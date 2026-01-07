#include "avx_filter.hpp"

namespace avx {

// 查找表实现
AVXFilter::MaskLookupTable::MaskLookupTable()
{
  for (int mask_bits = 0; mask_bits < 256; ++mask_bits) {
    int positions [8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

    // 计算符合条件的元素位置
    for (int src_pos = 0, dst_pos = 0; src_pos < 8; ++src_pos) {
      if (mask_bits & (1 << src_pos)) { positions [dst_pos++] = src_pos; }
    }

    // 存储排列索引
    permutation [mask_bits] =
        _mm256_loadu_si256(reinterpret_cast<const __m256i*>(positions));

    // 计算符合条件的元素数量
    const int match_count = _mm_popcnt_u32(static_cast<unsigned>(mask_bits));

    // 生成存储掩码（只有前match_count个通道为-1）
    mask [mask_bits] =
        _mm256_setr_epi32(match_count > 0 ? -1 : 0, match_count > 1 ? -1 : 0,
                          match_count > 2 ? -1 : 0, match_count > 3 ? -1 : 0,
                          match_count > 4 ? -1 : 0, match_count > 5 ? -1 : 0,
                          match_count > 6 ? -1 : 0, match_count > 7 ? -1 : 0);
  }
}

AVXFilter::PlainMaskLookupTable::PlainMaskLookupTable()
{
  for (int mask_bits = 0; mask_bits < 256; ++mask_bits) {
    int positions [8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

    for (int src_pos = 0, dst_pos = 0; src_pos < 8; ++src_pos) {
      if (mask_bits & (1 << src_pos)) { positions [dst_pos++] = src_pos; }
    }

    __m256i perm =
        _mm256_loadu_si256(reinterpret_cast<const __m256i*>(positions));
    _mm256_store_si256(masklut + mask_bits * 2, perm);

    const int match_count = _mm_popcnt_u32(static_cast<unsigned>(mask_bits));

    __m256i mask =
        _mm256_setr_epi32(match_count > 0 ? -1 : 0, match_count > 1 ? -1 : 0,
                          match_count > 2 ? -1 : 0, match_count > 3 ? -1 : 0,
                          match_count > 4 ? -1 : 0, match_count > 5 ? -1 : 0,
                          match_count > 6 ? -1 : 0, match_count > 7 ? -1 : 0);

    _mm256_store_si256(masklut + mask_bits * 2 + 1, mask);
  }
}

const AVXFilter::MaskLookupTable& AVXFilter::getLookupTable()
{
  static const MaskLookupTable instance;
  return instance;
}

const AVXFilter::PlainMaskLookupTable& AVXFilter::getPlainLookupTable()
{
  static const PlainMaskLookupTable instance;
  return instance;
}

// 核心过滤实现
template<int ComparisonCode>
size_t AVXFilter::filterImpl(const float* input, size_t count, float threshold,
                             float* output)
{
  const __m256       threshold_vector = _mm256_set1_ps(threshold);
  const float* const input_end        = input + count;
  const float* const original_output  = output;

  const auto& lookup_table = getLookupTable();

  // 向量化处理：每次处理16个元素
  while (input + 16 <= input_end) {
    // 加载并比较前8个元素
    __m256 first_chunk = _mm256_loadu_ps(input);
    __m256 first_mask =
        _mm256_cmp_ps(first_chunk, threshold_vector, ComparisonCode);

    // 加载并比较后8个元素
    __m256 second_chunk = _mm256_loadu_ps(input + 8);
    input += 16;
    __m256 second_mask =
        _mm256_cmp_ps(second_chunk, threshold_vector, ComparisonCode);

    // 生成位掩码
    const unsigned first_mask_bits  = _mm256_movemask_ps(first_mask);  // 0-255
    const unsigned second_mask_bits = _mm256_movemask_ps(second_mask);

    // 处理第一个8元素块 - 使用正确的索引
    __m256i permutation =
        _mm256_load_si256(&lookup_table.permutation [first_mask_bits]);
    __m256i store_mask =
        _mm256_load_si256(&lookup_table.mask [first_mask_bits]);

    first_chunk = _mm256_permutevar8x32_ps(first_chunk, permutation);
    _mm256_maskstore_ps(output, store_mask, first_chunk);
    output += _mm_popcnt_u32(first_mask_bits);

    // 处理第二个8元素块
    permutation =
        _mm256_load_si256(&lookup_table.permutation [second_mask_bits]);
    store_mask = _mm256_load_si256(&lookup_table.mask [second_mask_bits]);

    second_chunk = _mm256_permutevar8x32_ps(second_chunk, permutation);
    _mm256_maskstore_ps(output, store_mask, second_chunk);
    output += _mm_popcnt_u32(second_mask_bits);
  }
#if 0
        // 向量化处理：每次处理16个元素
        while (input + 16 <= input_end) {
            // 加载并比较前8个元素
            const __m256 first_chunk = _mm256_loadu_ps(input);
            const __m256 first_mask = _mm256_cmp_ps(first_chunk, threshold_vector, ComparisonCode);

            // 加载并比较后8个元素
            const __m256 second_chunk = _mm256_loadu_ps(input + 8);
            input += 16;
            const __m256 second_mask = _mm256_cmp_ps(second_chunk, threshold_vector, ComparisonCode);

            // 生成位掩码并查找对应的排列和存储掩码
            const size_t first_mask_bits = static_cast<size_t>(_mm256_movemask_ps(first_mask)) << 6;
            const size_t second_mask_bits = static_cast<size_t>(_mm256_movemask_ps(second_mask)) << 6;

            // 处理第一个8元素块
            const __m256i* first_lookup_ptr = lookup_table.permutation + (first_mask_bits >> 5);
            const __m256i first_permutation = _mm256_load_si256(first_lookup_ptr);
            const __m256i first_store_mask = _mm256_load_si256(first_lookup_ptr + 1);

            __m256 first_filtered = _mm256_permutevar8x32_ps(first_chunk, first_permutation);
            _mm256_maskstore_ps(output, first_store_mask, first_filtered);
            output += _mm_popcnt_u32(static_cast<unsigned>(first_mask_bits));

            // 处理第二个8元素块
            const __m256i* second_lookup_ptr = lookup_table.permutation + (second_mask_bits >> 5);
            const __m256i second_permutation = _mm256_load_si256(second_lookup_ptr);
            const __m256i second_store_mask = _mm256_load_si256(second_lookup_ptr + 1);

            __m256 second_filtered = _mm256_permutevar8x32_ps(second_chunk, second_permutation);
            _mm256_maskstore_ps(output, second_store_mask, second_filtered);
            output += _mm_popcnt_u32(static_cast<unsigned>(second_mask_bits));
        }
#endif  // 0
        // 标量处理剩余元素
  for (; input < input_end; ++input) {
    const __m128 element = _mm_load_ss(input);
    const __m128 comparison =
        _mm_cmp_ss(element, _mm_set_ss(threshold), ComparisonCode);
    const int result_mask = _mm_extract_ps(comparison, 0);

    if (result_mask) { _mm_store_ss(output++, element); }
  }

  return output - original_output;
}

// 核心过滤实现
template<int ComparisonCode>
size_t AVXFilter::filterPlainImpl(const float* input, size_t count,
                                  float threshold, float* output)
{
  const __m256       threshold_vector = _mm256_set1_ps(threshold);
  const float* const input_end        = input + count;
  const float* const original_output  = output;

  const auto& lookup_table = getPlainLookupTable();
  // 向量化处理：每次处理16个元素
  while (input + 16 <= input_end) {
    // 加载并比较前8个元素
    __m256 first_chunk = _mm256_loadu_ps(input);
    __m256 first_mask =
        _mm256_cmp_ps(first_chunk, threshold_vector, ComparisonCode);

    // 加载并比较后8个元素
    __m256 second_chunk = _mm256_loadu_ps(input + 8);
    input += 16;
    __m256 second_mask =
        _mm256_cmp_ps(second_chunk, threshold_vector, ComparisonCode);

    // 生成位掩码并查找对应的排列和存储掩码
    size_t first_mask_bits = static_cast<size_t>(_mm256_movemask_ps(first_mask))
        << 6;
    size_t second_mask_bits =
        static_cast<size_t>(_mm256_movemask_ps(second_mask)) << 6;

    // 处理第一个8元素块
    const __m256i* lookup_ptr  = lookup_table.masklut + (first_mask_bits >> 5);
    __m256i        permutation = _mm256_load_si256(lookup_ptr);
    __m256i        store_mask  = _mm256_load_si256(lookup_ptr + 1);

    first_chunk = _mm256_permutevar8x32_ps(first_chunk, permutation);
    _mm256_maskstore_ps(output, store_mask, first_chunk);
    output += _mm_popcnt_u32(static_cast<unsigned>(first_mask_bits));

    // 处理第二个8元素块
    lookup_ptr  = lookup_table.masklut + (second_mask_bits >> 5);
    permutation = _mm256_load_si256(lookup_ptr);
    store_mask  = _mm256_load_si256(lookup_ptr + 1);

    second_chunk = _mm256_permutevar8x32_ps(second_chunk, permutation);
    _mm256_maskstore_ps(output, store_mask, second_chunk);
    output += _mm_popcnt_u32(static_cast<unsigned>(second_mask_bits));
  }

  // 标量处理剩余元素
  for (; input < input_end; ++input) {
    const __m128 element = _mm_load_ss(input);
    const __m128 comparison =
        _mm_cmp_ss(element, _mm_set_ss(threshold), ComparisonCode);
    const int result_mask = _mm_extract_ps(comparison, 0);

    if (result_mask) { _mm_store_ss(output++, element); }
  }

  return output - original_output;
}

// 模板特化实例化
template size_t AVXFilter::filterImpl<_CMP_GT_OQ>(const float*, size_t, float,
                                                  float*);
template size_t AVXFilter::filterImpl<_CMP_LT_OQ>(const float*, size_t, float,
                                                  float*);
template size_t AVXFilter::filterImpl<_CMP_EQ_OQ>(const float*, size_t, float,
                                                  float*);
template size_t AVXFilter::filterImpl<_CMP_GE_OQ>(const float*, size_t, float,
                                                  float*);
template size_t AVXFilter::filterImpl<_CMP_LE_OQ>(const float*, size_t, float,
                                                  float*);

template size_t AVXFilter::filterPlainImpl<_CMP_GT_OQ>(const float*, size_t,
                                                       float, float*);
template size_t AVXFilter::filterPlainImpl<_CMP_LT_OQ>(const float*, size_t,
                                                       float, float*);
template size_t AVXFilter::filterPlainImpl<_CMP_EQ_OQ>(const float*, size_t,
                                                       float, float*);
template size_t AVXFilter::filterPlainImpl<_CMP_GE_OQ>(const float*, size_t,
                                                       float, float*);
template size_t AVXFilter::filterPlainImpl<_CMP_LE_OQ>(const float*, size_t,
                                                       float, float*);

// 公共接口实现
template<AVXFilter::ComparisonOperator Operator>
size_t AVXFilter::filter(const float* input, size_t count, float threshold,
                         float* output)
{
  return filterImpl<static_cast<int>(Operator)>(input, count, threshold,
                                                output);
}

template<AVXFilter::ComparisonOperator Operator>
size_t AVXFilter::filterPlain(const float* input, size_t count, float threshold,
                              float* output)
{
  return filterPlainImpl<static_cast<int>(Operator)>(input, count, threshold,
                                                     output);
}

size_t AVXFilter::filterGreaterThan(const float* input, size_t count,
                                    float threshold, float* output)
{
  return filter<ComparisonOperator::GreaterThan>(input, count, threshold,
                                                 output);
}

size_t AVXFilter::filterLessThan(const float* input, size_t count,
                                 float threshold, float* output)
{
  return filter<ComparisonOperator::LessThan>(input, count, threshold, output);
}

size_t AVXFilter::filterPlainLessThan(const float* input, size_t count,
                                      float threshold, float* output)
{
  return filterPlain<ComparisonOperator::LessThan>(input, count, threshold,
                                                   output);
}

size_t AVXFilter::filterEqual(const float* input, size_t count, float threshold,
                              float* output)
{
  return filter<ComparisonOperator::Equal>(input, count, threshold, output);
}

}  // namespace avx