#pragma once

#include <utility>
#include <type_traits>
namespace stdex {

// Wrapper for POD types that skips default initialization
// Useful for performance optimization when you will immediately initialize the
// value
//
// Usage:
//   NoInitializedPod<int> uninitialized_int;
//   uninitialized_int = 42;  // No wasted initialization
//   uninitialized_int.emplace(100);  // In-place construction
//
// Note: Only safe for POD types. For non-POD types, constructors will be
// called.
template<typename Type>
struct NoInitializedPod
{
  static_assert(
      std::conjunction_v<std::is_trivially_copyable<Type>,
                         std::is_trivially_default_constructible<Type>,
                         std::is_trivially_destructible<Type>>,
      "Type must satisfy requirements for uninitialized storage");

private:

  Type m_Value;

public:

  // Default constructor - does NOT zero-initialize for POD types
  NoInitializedPod() noexcept { }

  NoInitializedPod(const NoInitializedPod& other) : m_Value(other.m_Value) { }

  NoInitializedPod(NoInitializedPod&& other) noexcept
      : m_Value(std::move(other.m_Value))
  {
  }

  NoInitializedPod& operator= (const NoInitializedPod& other)
  {
    m_Value = other.m_Value;
    return *this;
  }

  NoInitializedPod& operator= (NoInitializedPod&& other) noexcept
  {
    m_Value = std::move(other.m_Value);
    return *this;
  }

  ~NoInitializedPod() = default;

  NoInitializedPod(const Type& value) : m_Value(value) { }

  NoInitializedPod(Type&& value) noexcept : m_Value(std::move(value)) { }

  NoInitializedPod& operator= (const Type& value)
  {
    m_Value = value;
    return *this;
  }

  NoInitializedPod& operator= (Type&& value) noexcept
  {
    m_Value = std::move(value);
    return *this;
  }

  // Implicit conversion operators
  operator Type&() { return m_Value; }

  operator const Type&() const { return m_Value; }

  // Explicit access
  Type& get() { return m_Value; }

  const Type& get() const { return m_Value; }

  // Manual destruction
  void destroy() { m_Value.~Type(); }

  // In-place construction with arguments
  template<typename... Arguments>
  NoInitializedPod& emplace(Arguments&&... arguments)
  {
    ::new (&m_Value) Type(std::forward<Arguments>(arguments)...);
    return *this;
  }
};

}  // namespace stdex
