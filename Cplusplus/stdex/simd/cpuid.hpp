#pragma once

#include <array>

#if defined(_MSC_VER)
#  include <intrin.h>
#else
#  include <cpuid.h>
#endif

namespace stdex::cpu {

struct CPUID
{
  using register_type = uint32_t;

  static void execute_cpuid(register_type leaf, register_type subleaf,
                                      std::array<register_type, 4>& regs) noexcept
  {
#if defined(_MSC_VER)
    int cpu_info [4];
    __cpuidex(cpu_info, static_cast<int>(leaf), static_cast<int>(subleaf));
    regs [0] = static_cast<register_type>(cpu_info [0]);
    regs [1] = static_cast<register_type>(cpu_info [1]);
    regs [2] = static_cast<register_type>(cpu_info [2]);
    regs [3] = static_cast<register_type>(cpu_info [3]);
#else
    __cpuid_count(static_cast<int>(leaf), static_cast<int>(subleaf), regs [0],
                  regs [1], regs [2], regs [3]);
#endif
  }

  constexpr CPUID() noexcept : regs_ { 0, 0, 0, 0 } { }

  explicit CPUID(register_type leaf, register_type subleaf = 0) noexcept
  {
    execute_cpuid(leaf, subleaf, regs_);
  }

  [[nodiscard]] constexpr register_type eax() const noexcept { return regs_ [0]; }

  [[nodiscard]] constexpr register_type ebx() const noexcept { return regs_ [1]; }

  [[nodiscard]] constexpr register_type ecx() const noexcept { return regs_ [2]; }

  [[nodiscard]] constexpr register_type edx() const noexcept { return regs_ [3]; }

  [[nodiscard]] constexpr bool test_bit(register_type reg,
                                        register_type bit) const noexcept
  {
    return (regs_ [reg] & (register_type { 1 } << bit)) != 0;
  }

  [[nodiscard]] constexpr const std::array<register_type, 4>&
  registers() const noexcept
  {
    return regs_;
  }

private:

  std::array<register_type, 4> regs_;
};

inline std::array<uint32_t, 4> cpuid(uint32_t leaf, uint32_t subleaf = 0) noexcept
{
  std::array<uint32_t, 4> regs {};
  CPUID::execute_cpuid(leaf, subleaf, regs);
  return regs;
}

}  // namespace stdex::cpu
