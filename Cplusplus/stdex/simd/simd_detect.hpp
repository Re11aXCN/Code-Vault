#pragma once

#include <array>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include <version>

#include "cpuid.hpp"

namespace stdex::simd {

enum class Architecture : uint8_t
{
  unknown = 0,
  x86     = 1,
  x86_64  = 2,
  arm     = 3,
  arm64   = 4
};

enum class InstructionSet : uint8_t
{
  none    = 0,
  mmx     = 1,
  sse     = 2,
  sse2    = 3,
  sse3    = 4,
  ssse3   = 5,
  sse4_1  = 6,
  sse4_2  = 7,
  avx     = 8,
  avx2    = 9,
  avx512f = 10,
  avx512dq = 11,
  avx512ifma = 12,
  avx512pf = 13,
  avx512er = 14,
  avx512cd = 15,
  avx512bw = 16,
  avx512vl = 17,
  avx512vbmi = 18,
  neon    = 19,
  neon64  = 20,
  sve     = 21,
  sve2    = 22
};

constexpr std::string_view to_string(Architecture arch) noexcept
{
  switch (arch) {
  case Architecture::x86    : return "x86";
  case Architecture::x86_64 : return "x86_64";
  case Architecture::arm    : return "ARM";
  case Architecture::arm64  : return "ARM64";
  default                   : return "Unknown";
  }
}

constexpr std::string_view to_string(InstructionSet iset) noexcept
{
  switch (iset) {
  case InstructionSet::mmx     : return "MMX";
  case InstructionSet::sse     : return "SSE";
  case InstructionSet::sse2    : return "SSE2";
  case InstructionSet::sse3    : return "SSE3";
  case InstructionSet::ssse3   : return "SSSE3";
  case InstructionSet::sse4_1  : return "SSE4.1";
  case InstructionSet::sse4_2  : return "SSE4.2";
  case InstructionSet::avx     : return "AVX";
  case InstructionSet::avx2    : return "AVX2";
  case InstructionSet::avx512f : return "AVX512F";
  case InstructionSet::avx512dq : return "AVX512DQ";
  case InstructionSet::avx512ifma : return "AVX512IFMA";
  case InstructionSet::avx512pf : return "AVX512PF";
  case InstructionSet::avx512er : return "AVX512ER";
  case InstructionSet::avx512cd : return "AVX512CD";
  case InstructionSet::avx512bw : return "AVX512BW";
  case InstructionSet::avx512vl : return "AVX512VL";
  case InstructionSet::avx512vbmi : return "AVX512VBMI";
  case InstructionSet::neon    : return "NEON";
  case InstructionSet::neon64  : return "NEON64";
  case InstructionSet::sve     : return "SVE";
  case InstructionSet::sve2    : return "SVE2";
  default                     : return "None";
  }
}

constexpr bool is_x86_architecture(Architecture arch) noexcept
{
  return arch == Architecture::x86 || arch == Architecture::x86_64;
}

constexpr bool is_arm_architecture(Architecture arch) noexcept
{
  return arch == Architecture::arm || arch == Architecture::arm64;
}

constexpr bool is_x86_instruction_set(InstructionSet iset) noexcept
{
  return iset == InstructionSet::mmx ||
         iset == InstructionSet::sse ||
         iset == InstructionSet::sse2 ||
         iset == InstructionSet::sse3 ||
         iset == InstructionSet::ssse3 ||
         iset == InstructionSet::sse4_1 ||
         iset == InstructionSet::sse4_2 ||
         iset == InstructionSet::avx ||
         iset == InstructionSet::avx2 ||
         iset == InstructionSet::avx512f ||
         iset == InstructionSet::avx512dq ||
         iset == InstructionSet::avx512ifma ||
         iset == InstructionSet::avx512pf ||
         iset == InstructionSet::avx512er ||
         iset == InstructionSet::avx512cd ||
         iset == InstructionSet::avx512bw ||
         iset == InstructionSet::avx512vl ||
         iset == InstructionSet::avx512vbmi;
}

constexpr bool is_arm_instruction_set(InstructionSet iset) noexcept
{
  return iset == InstructionSet::neon ||
         iset == InstructionSet::neon64 ||
         iset == InstructionSet::sve ||
         iset == InstructionSet::sve2;
}

namespace detail {

enum class TypeIndex : uint8_t
{
  uint32  = 0,
  int32   = 1,
  uint64  = 2,
  int64   = 3,
  float32 = 4,
  float64 = 5
};

template<typename T>
struct type_traits;

template<>
struct type_traits<uint32_t>
{
  static constexpr TypeIndex index = TypeIndex::uint32;
};

template<>
struct type_traits<int32_t>
{
  static constexpr TypeIndex index = TypeIndex::int32;
};

template<>
struct type_traits<uint64_t>
{
  static constexpr TypeIndex index = TypeIndex::uint64;
};

template<>
struct type_traits<int64_t>
{
  static constexpr TypeIndex index = TypeIndex::int64;
};

template<>
struct type_traits<float>
{
  static constexpr TypeIndex index = TypeIndex::float32;
};

template<>
struct type_traits<double>
{
  static constexpr TypeIndex index = TypeIndex::float64;
};

struct InstructionSetInfo
{
  InstructionSet         iset;
  std::string_view       name;
  uint8_t                priority;
  std::array<uint8_t, 6> widths;
};

inline constexpr std::array<InstructionSetInfo, 22> instruction_set_registry = {
  { { InstructionSet::mmx, "MMX", 1, { 4, 4, 2, 2, 4, 2 } },
    { InstructionSet::sse, "SSE", 2, { 4, 4, 2, 2, 4, 2 } },
    { InstructionSet::sse2, "SSE2", 3, { 4, 4, 2, 2, 4, 2 } },
    { InstructionSet::sse3, "SSE3", 4, { 4, 4, 2, 2, 4, 2 } },
    { InstructionSet::ssse3, "SSSE3", 5, { 4, 4, 2, 2, 4, 2 } },
    { InstructionSet::sse4_1, "SSE4.1", 6, { 4, 4, 2, 2, 4, 2 } },
    { InstructionSet::sse4_2, "SSE4.2", 7, { 4, 4, 2, 2, 4, 2 } },
    { InstructionSet::avx, "AVX", 8, { 8, 8, 4, 4, 8, 4 } },
    { InstructionSet::avx2, "AVX2", 9, { 8, 8, 4, 4, 8, 4 } },
    { InstructionSet::avx512f, "AVX512F", 10, { 16, 16, 8, 8, 16, 8 } },
    { InstructionSet::avx512dq, "AVX512DQ", 11, { 16, 16, 8, 8, 16, 8 } },
    { InstructionSet::avx512ifma, "AVX512IFMA", 12, { 16, 16, 8, 8, 16, 8 } },
    { InstructionSet::avx512pf, "AVX512PF", 13, { 16, 16, 8, 8, 16, 8 } },
    { InstructionSet::avx512er, "AVX512ER", 14, { 16, 16, 8, 8, 16, 8 } },
    { InstructionSet::avx512cd, "AVX512CD", 15, { 16, 16, 8, 8, 16, 8 } },
    { InstructionSet::avx512bw, "AVX512BW", 16, { 16, 16, 8, 8, 16, 8 } },
    { InstructionSet::avx512vl, "AVX512VL", 17, { 16, 16, 8, 8, 16, 8 } },
    { InstructionSet::avx512vbmi, "AVX512VBMI", 18, { 16, 16, 8, 8, 16, 8 } },
    { InstructionSet::neon, "NEON", 1, { 4, 4, 2, 2, 4, 2 } },
    { InstructionSet::neon64, "NEON64", 2, { 4, 4, 2, 2, 4, 2 } },
    { InstructionSet::sve, "SVE", 3, { 4, 4, 2, 2, 4, 2 } },
    { InstructionSet::sve2, "SVE2", 4, { 4, 4, 2, 2, 4, 2 } } }
};

inline constexpr Architecture detect_architecture() noexcept
{
#if defined(_M_X64) || defined(__x86_64__)
  return Architecture::x86_64;
#elif defined(_M_IX86) || defined(__i386__)
  return Architecture::x86;
#elif defined(_M_ARM64) || defined(__aarch64__)
  return Architecture::arm64;
#elif defined(_M_ARM) || defined(__arm__)
  return Architecture::arm;
#else
  return Architecture::unknown;
#endif
}

inline InstructionSet detect_x86_simd() noexcept
{
#if defined(_M_X64) || defined(__x86_64__) || defined(_M_IX86) || \
    defined(__i386__)
  cpu::CPUID cpu_info(1, 0);

  bool mmx    = cpu_info.test_bit(3, 23);
  bool sse    = cpu_info.test_bit(3, 25);
  bool sse2   = cpu_info.test_bit(3, 26);
  bool sse3   = cpu_info.test_bit(2, 0);
  bool ssse3  = cpu_info.test_bit(2, 9);
  bool sse4_1 = cpu_info.test_bit(2, 19);
  bool sse4_2 = cpu_info.test_bit(2, 20);
  bool avx    = cpu_info.test_bit(2, 28);

  cpu::CPUID cpu_info7(7, 0);
  bool avx2    = cpu_info7.test_bit(1, 5);
  bool avx512f = cpu_info7.test_bit(1, 16);
  bool avx512dq = cpu_info7.test_bit(1, 17);
  bool avx512ifma = cpu_info7.test_bit(1, 21);
  bool avx512pf = cpu_info7.test_bit(1, 26);
  bool avx512er = cpu_info7.test_bit(1, 27);
  bool avx512cd = cpu_info7.test_bit(1, 28);
  bool avx512bw = cpu_info7.test_bit(1, 30);
  bool avx512vl = cpu_info7.test_bit(1, 31);
  bool avx512vbmi = cpu_info7.test_bit(2, 1);

  if (avx512vbmi) return InstructionSet::avx512vbmi;
  if (avx512vl) return InstructionSet::avx512vl;
  if (avx512bw) return InstructionSet::avx512bw;
  if (avx512cd) return InstructionSet::avx512cd;
  if (avx512er) return InstructionSet::avx512er;
  if (avx512pf) return InstructionSet::avx512pf;
  if (avx512ifma) return InstructionSet::avx512ifma;
  if (avx512dq) return InstructionSet::avx512dq;
  if (avx512f) return InstructionSet::avx512f;
  if (avx2) return InstructionSet::avx2;
  if (avx) return InstructionSet::avx;
  if (sse4_2) return InstructionSet::sse4_2;
  if (sse4_1) return InstructionSet::sse4_1;
  if (ssse3) return InstructionSet::ssse3;
  if (sse3) return InstructionSet::sse3;
  if (sse) return InstructionSet::sse;
  if (mmx) return InstructionSet::mmx;
#endif
  return InstructionSet::none;
}

inline InstructionSet detect_arm_simd() noexcept
{
#if defined(_M_ARM64) || defined(__aarch64__)
  // 检测 SVE2 支持
  cpu::CPUID cpu_info(0, 0);
  if (cpu_info.eax() >= 0x10) {
    cpu::CPUID sve_info(0x10, 0);
    bool sve2 = sve_info.test_bit(1, 1);
    if (sve2) return InstructionSet::sve2;
    bool sve = sve_info.test_bit(1, 0);
    if (sve) return InstructionSet::sve;
  }
  return InstructionSet::neon64;
#elif defined(_M_ARM) || defined(__arm__)
  return InstructionSet::neon;
#endif
  return InstructionSet::none;
}

struct SystemInfo
{
  Architecture   architecture;
  InstructionSet instruction_set;

  constexpr SystemInfo() noexcept
      : architecture(detect_architecture())
      , instruction_set([&]() constexpr noexcept {
        if (is_x86_architecture(architecture)) {
          return detect_x86_simd();
        } else if (is_arm_architecture(architecture)) {
          return detect_arm_simd();
        }
        return InstructionSet::none;
      }())
  {
  }
};

inline const SystemInfo& get_system_info() noexcept
{
  static const SystemInfo info {};
  return info;
}

}  // namespace detail

inline Architecture get_architecture() noexcept
{
  return detail::get_system_info().architecture;
}

inline InstructionSet get_instruction_set() noexcept
{
  return detail::get_system_info().instruction_set;
}

inline bool has_simd_support() noexcept
{
  return get_instruction_set() != InstructionSet::none;
}

inline bool has_mmx() noexcept
{
  InstructionSet iset = get_instruction_set();
  return is_x86_architecture(get_architecture()) && iset == InstructionSet::mmx;
}

inline bool has_sse() noexcept
{
  InstructionSet iset = get_instruction_set();
  return is_x86_architecture(get_architecture()) && iset >= InstructionSet::sse;
}

inline bool has_sse2() noexcept
{
  InstructionSet iset = get_instruction_set();
  return is_x86_architecture(get_architecture()) && iset >= InstructionSet::sse2;
}

inline bool has_sse3() noexcept
{
  InstructionSet iset = get_instruction_set();
  return is_x86_architecture(get_architecture()) && iset >= InstructionSet::sse3;
}

inline bool has_ssse3() noexcept
{
  InstructionSet iset = get_instruction_set();
  return is_x86_architecture(get_architecture()) && iset >= InstructionSet::ssse3;
}

inline bool has_sse4_1() noexcept
{
  InstructionSet iset = get_instruction_set();
  return is_x86_architecture(get_architecture()) && iset >= InstructionSet::sse4_1;
}

inline bool has_sse4_2() noexcept
{
  InstructionSet iset = get_instruction_set();
  return is_x86_architecture(get_architecture()) && iset >= InstructionSet::sse4_2;
}

inline bool has_avx() noexcept
{
  InstructionSet iset = get_instruction_set();
  return is_x86_architecture(get_architecture()) && iset >= InstructionSet::avx;
}

inline bool has_avx2() noexcept
{
  InstructionSet iset = get_instruction_set();
  return is_x86_architecture(get_architecture()) && iset >= InstructionSet::avx2;
}

inline bool has_avx512() noexcept
{
  InstructionSet iset = get_instruction_set();
  return is_x86_architecture(get_architecture()) && iset >= InstructionSet::avx512f;
}

inline bool has_avx512f() noexcept
{
  InstructionSet iset = get_instruction_set();
  return is_x86_architecture(get_architecture()) && iset >= InstructionSet::avx512f;
}

inline bool has_avx512dq() noexcept
{
  InstructionSet iset = get_instruction_set();
  return is_x86_architecture(get_architecture()) && iset >= InstructionSet::avx512dq;
}

inline bool has_avx512ifma() noexcept
{
  InstructionSet iset = get_instruction_set();
  return is_x86_architecture(get_architecture()) && iset >= InstructionSet::avx512ifma;
}

inline bool has_avx512pf() noexcept
{
  InstructionSet iset = get_instruction_set();
  return is_x86_architecture(get_architecture()) && iset >= InstructionSet::avx512pf;
}

inline bool has_avx512er() noexcept
{
  InstructionSet iset = get_instruction_set();
  return is_x86_architecture(get_architecture()) && iset >= InstructionSet::avx512er;
}

inline bool has_avx512cd() noexcept
{
  InstructionSet iset = get_instruction_set();
  return is_x86_architecture(get_architecture()) && iset >= InstructionSet::avx512cd;
}

inline bool has_avx512bw() noexcept
{
  InstructionSet iset = get_instruction_set();
  return is_x86_architecture(get_architecture()) && iset >= InstructionSet::avx512bw;
}

inline bool has_avx512vl() noexcept
{
  InstructionSet iset = get_instruction_set();
  return is_x86_architecture(get_architecture()) && iset >= InstructionSet::avx512vl;
}

inline bool has_avx512vbmi() noexcept
{
  InstructionSet iset = get_instruction_set();
  return is_x86_architecture(get_architecture()) && iset >= InstructionSet::avx512vbmi;
}

inline bool has_neon() noexcept
{
  InstructionSet iset = get_instruction_set();
  return is_arm_architecture(get_architecture()) && 
         (iset == InstructionSet::neon || iset == InstructionSet::neon64);
}

inline bool has_neon64() noexcept
{
  InstructionSet iset = get_instruction_set();
  return is_arm_architecture(get_architecture()) && iset == InstructionSet::neon64;
}

inline bool has_sve() noexcept
{
  InstructionSet iset = get_instruction_set();
  return is_arm_architecture(get_architecture()) && iset >= InstructionSet::sve;
}

inline bool has_sve2() noexcept
{
  InstructionSet iset = get_instruction_set();
  return is_arm_architecture(get_architecture()) && iset >= InstructionSet::sve2;
}

template<typename T>
inline size_t get_simd_width() noexcept
{
  InstructionSet iset = get_instruction_set();

  if (iset == InstructionSet::none) return 1;

  for (const auto& info : detail::instruction_set_registry) {
    if (info.iset == iset) {
      uint8_t idx = static_cast<uint8_t>(detail::type_traits<T>::index);
      return static_cast<size_t>(info.widths [idx]);
    }
  }

  return 1;
}

template<typename T>
constexpr size_t get_simd_width(InstructionSet iset) noexcept
{
  if (iset == InstructionSet::none) return 1;

  for (const auto& info : detail::instruction_set_registry) {
    if (info.iset == iset) {
      uint8_t idx = static_cast<uint8_t>(detail::type_traits<T>::index);
      return static_cast<size_t>(info.widths [idx]);
    }
  }

  return 1;
}

namespace compile_time {

inline constexpr Architecture architecture = detail::detect_architecture();

#if defined(__MMX__)
constexpr bool has_mmx = true;
#else
constexpr bool has_mmx = false;
#endif

#if defined(__SSE__)
constexpr bool has_sse = true;
#else
constexpr bool has_sse = false;
#endif

#if defined(__SSE2__)
constexpr bool has_sse2 = true;
#else
constexpr bool has_sse2 = false;
#endif

#if defined(__SSE3__)
constexpr bool has_sse3 = true;
#else
constexpr bool has_sse3 = false;
#endif

#if defined(__SSSE3__)
constexpr bool has_ssse3 = true;
#else
constexpr bool has_ssse3 = false;
#endif

#if defined(__SSE4_1__)
constexpr bool has_sse4_1 = true;
#else
constexpr bool has_sse4_1 = false;
#endif

#if defined(__SSE4_2__)
constexpr bool has_sse4_2 = true;
#else
constexpr bool has_sse4_2 = false;
#endif

#if defined(__AVX__)
constexpr bool has_avx = true;
#else
constexpr bool has_avx = false;
#endif

#if defined(__AVX2__)
constexpr bool has_avx2 = true;
#else
constexpr bool has_avx2 = false;
#endif

#if defined(__AVX512F__)
constexpr bool has_avx512f = true;
#else
constexpr bool has_avx512f = false;
#endif

#if defined(__AVX512DQ__)
constexpr bool has_avx512dq = true;
#else
constexpr bool has_avx512dq = false;
#endif

#if defined(__AVX512IFMA__)
constexpr bool has_avx512ifma = true;
#else
constexpr bool has_avx512ifma = false;
#endif

#if defined(__AVX512PF__)
constexpr bool has_avx512pf = true;
#else
constexpr bool has_avx512pf = false;
#endif

#if defined(__AVX512ER__)
constexpr bool has_avx512er = true;
#else
constexpr bool has_avx512er = false;
#endif

#if defined(__AVX512CD__)
constexpr bool has_avx512cd = true;
#else
constexpr bool has_avx512cd = false;
#endif

#if defined(__AVX512BW__)
constexpr bool has_avx512bw = true;
#else
constexpr bool has_avx512bw = false;
#endif

#if defined(__AVX512VL__)
constexpr bool has_avx512vl = true;
#else
constexpr bool has_avx512vl = false;
#endif

#if defined(__AVX512VBMI__)
constexpr bool has_avx512vbmi = true;
#else
constexpr bool has_avx512vbmi = false;
#endif

#if defined(__ARM_NEON)
constexpr bool has_neon = true;
#else
constexpr bool has_neon = false;
#endif

#if defined(__ARM_FEATURE_SVE)
constexpr bool has_sve = true;
#else
constexpr bool has_sve = false;
#endif

#if defined(__ARM_FEATURE_SVE2)
constexpr bool has_sve2 = true;
#else
constexpr bool has_sve2 = false;
#endif

}  // namespace compile_time

}  // namespace stdex::simd
