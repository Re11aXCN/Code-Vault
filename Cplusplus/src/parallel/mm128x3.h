#pragma once

#include <emmintrin.h>

struct __m128x3
{
  __m128 val [3];
};

static __m128x3 _mm128x3_load_ps(float const* p)
{
  __m128x3 v;
  v.val [0] = _mm_load_ps(p);
  v.val [1] = _mm_load_ps(p + 4);
  v.val [2] = _mm_load_ps(p + 8);
  return v;
}

static __m128x3 _mm128x3_loadu_ps(float const* p)
{
  __m128x3 v;
  v.val [0] = _mm_loadu_ps(p);
  v.val [1] = _mm_loadu_ps(p + 4);
  v.val [2] = _mm_loadu_ps(p + 8);
  return v;
}

static void _mm128x3_store_ps(float* p, __m128x3 v)
{
  _mm_store_ps(p, v.val [0]);
  _mm_store_ps(p + 4, v.val [1]);
  _mm_store_ps(p + 8, v.val [2]);
}

static void _mm128x3_storeu_ps(float* p, __m128x3 v)
{
  _mm_storeu_ps(p, v.val [0]);
  _mm_storeu_ps(p + 4, v.val [1]);
  _mm_storeu_ps(p + 8, v.val [2]);
}

static void _mm128x3_stream_ps(float* p, __m128x3 v)
{
  _mm_stream_ps(p, v.val [0]);
  _mm_stream_ps(p + 4, v.val [1]);
  _mm_stream_ps(p + 8, v.val [2]);
}

static __m128x3 _mm128x3_transpose_ps(__m128x3 v)
{
  __m128 tmp0 = _mm_castsi128_ps(_mm_shuffle_epi32(
      _mm_castps_si128(v.val [0]), _MM_SHUFFLE(0, 3, 1, 2)));  // a0,a3,a1,a2
  __m128 tmp1 = _mm_castsi128_ps(_mm_shuffle_epi32(
      _mm_castps_si128(v.val [1]), _MM_SHUFFLE(2, 3, 0, 1)));  // b2,b3,b0,b1
  __m128 tmp2 = _mm_castsi128_ps(_mm_shuffle_epi32(
      _mm_castps_si128(v.val [2]), _MM_SHUFFLE(1, 2, 0, 3)));  // c1,c2,c0,c3
  __m128 tmp3 = _mm_unpacklo_ps(tmp1, tmp2);                   // b2,c1,b3,c2
  v.val [0]   = _mm_movelh_ps(tmp0, tmp3);                     // a0,a3,b2,c1
  tmp0        = _mm_unpackhi_ps(tmp0, tmp1);                   // a1,b0,a2,b1
  v.val [1]   = _mm_castsi128_ps(_mm_shuffle_epi32(
      _mm_castps_si128(tmp0), _MM_SHUFFLE(2, 3, 0, 1)));  // a2,b1,a1,b0
  v.val [1]   = _mm_movehl_ps(tmp3, v.val [1]);             // a1,b0,b3,c2
  v.val [2]   = _mm_movehl_ps(tmp2, tmp0);                  // a2,b1,c0,c3
  return v;
}

static __m128x3 _mm128x3_add_ps(__m128x3 v1, __m128x3 v2)
{
  __m128x3 v;
  v.val [0] = _mm_add_ps(v1.val [0], v2.val [0]);
  v.val [1] = _mm_add_ps(v1.val [1], v2.val [1]);
  v.val [2] = _mm_add_ps(v1.val [2], v2.val [2]);
  return v;
}

static __m128x3 _mm128x3_sub_ps(__m128x3 v1, __m128x3 v2)
{
  __m128x3 v;
  v.val [0] = _mm_sub_ps(v1.val [0], v2.val [0]);
  v.val [1] = _mm_sub_ps(v1.val [1], v2.val [1]);
  v.val [2] = _mm_sub_ps(v1.val [2], v2.val [2]);
  return v;
}

static __m128x3 _mm128x3_mul_ps(__m128x3 v1, __m128x3 v2)
{
  __m128x3 v;
  v.val [0] = _mm_mul_ps(v1.val [0], v2.val [0]);
  v.val [1] = _mm_mul_ps(v1.val [1], v2.val [1]);
  v.val [2] = _mm_mul_ps(v1.val [2], v2.val [2]);
  return v;
}

static void func3(float* dst, float* src, int n)
{
  n &= ~3;
  for (int i = 0; i < n; i += 4) {
    __m128 r    = _mm_load_ps(src + i);
    __m128 g    = _mm_load_ps(src + n + i);
    __m128 b    = _mm_load_ps(src + n * 2 + i);
    __m128 tmp0 = r * 0.299f + g * 0.587f + b * 0.114f;
    _mm_store_ps(dst + i, tmp0);
  }
}

static void func2(float* dst, float* src, int n)
{
  n &= ~3;
  for (int i = 0; i < n; i += 4) {
    __m128x3 v  = _mm128x3_transpose_ps(_mm128x3_load_ps(src + i * 3));
    __m128 tmp0 = v.val [0] * 0.299f + v.val [1] * 0.587f + v.val [2] * 0.114f;
    _mm_store_ps(dst + i, tmp0);
  }
}

static void func1(float* dst, float* src, int n)
{
  n &= ~3;
  for (int i = 0; i < n; i++) {
    float r = src [i * 3];
    float g = src [i * 3 + 1];
    float b = src [i * 3 + 2];
    dst [i] = r * 0.299f + g * 0.587f + b * 0.114f;
  }
}

void mm128x3_test()
{
  int   n   = 1 << 13;
  auto* src = new float [n * 3] {};
  auto* dst = new float [n] {};
  func1(dst, src, n);
  func2(dst, src, n);
  func3(dst, src, n);
  TICK(func1);
  func1(dst, src, n);
  TOCK(func1);
  TICK(func2);
  func2(dst, src, n);
  TOCK(func2);
  TICK(func3);
  func3(dst, src, n);
  TOCK(func3);
}