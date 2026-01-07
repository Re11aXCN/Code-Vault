#pragma once

#include <concepts>
#include <exception>
#include <functional>
#include <type_traits>
#include <utility>

namespace stdex {
namespace detail {

// Concept: Check if type is invocable and does not throw exceptions
template<typename Func, typename... Arguments>
concept NothrowInvocable = std::invocable<Func, Arguments...> &&
    noexcept(std::invoke(std::declval<Func>(), std::declval<Arguments>()...));

// Concept: Check if type is a valid exit function
template<typename ExitFunc>
concept ExitFunction = requires (ExitFunc& exit_function) {
  requires std::invocable<ExitFunc&> ||
      std::invocable<std::remove_reference_t<ExitFunc>&> ||
      std::invocable<ExitFunc> ||
      std::invocable<std::remove_reference_t<ExitFunc>>;

  { std::invoke(exit_function) } -> std::same_as<void>;
};

// Always - Only occurs with an exception - As long as no exceptions occur
enum class ScopeGuardType
{
  Exit,
  Fail,
  Success
};

template<typename ExitFunc, ScopeGuardType ScopeGuardTypeValue>
class [[nodiscard]] ScopeGuardBase
{
  struct Dummy
  { };

  using ExceptionCountType =
      std::conditional_t<ScopeGuardTypeValue != ScopeGuardType::Exit, int,
                         Dummy>;

public:

  template<typename Function>
  requires ExitFunction<Function>
  explicit ScopeGuardBase(Function&& function) noexcept(
      std::is_nothrow_constructible_v<ExitFunc, Function>)
      : m_ExitFunction(std::forward<Function>(function)), m_Active(true)
  {
    if constexpr (ScopeGuardTypeValue != ScopeGuardType::Exit)
      m_ExceptionCount = std::uncaught_exceptions();
  }

  ScopeGuardBase(ScopeGuardBase&& other) noexcept(
      std::is_nothrow_move_constructible_v<ExitFunc>)
      : m_ExitFunction(std::move(other.m_ExitFunction))
      , m_Active(std::exchange(other.m_Active, false))
  {
    if constexpr (ScopeGuardTypeValue != ScopeGuardType::Exit)
      m_ExceptionCount = other.m_ExceptionCount;
  }

  ScopeGuardBase(const ScopeGuardBase&)             = delete;
  ScopeGuardBase& operator= (const ScopeGuardBase&) = delete;

  ScopeGuardBase& operator= (ScopeGuardBase&& other) noexcept(
      std::is_nothrow_move_assignable_v<ExitFunc> &&
      std::is_nothrow_move_constructible_v<ExitFunc>)
  {
    if (this != &other) {
      if (m_Active) execute_if_needed();

      m_ExitFunction = std::move(other.m_ExitFunction);
      m_Active       = std::exchange(other.m_Active, false);
      if constexpr (ScopeGuardTypeValue != ScopeGuardType::Exit)
        m_ExceptionCount = other.m_ExceptionCount;
    }
    return *this;
  }

  ~ScopeGuardBase() noexcept(noexcept(std::invoke(m_ExitFunction)))
  {
    if (m_Active) execute_if_needed();
  }

  void release() noexcept { m_Active = false; }

  [[nodiscard]] bool is_active() const noexcept { return m_Active; }

protected:

  void execute_if_needed()
  {
    if constexpr (ScopeGuardTypeValue == ScopeGuardType::Exit) {
      std::invoke(m_ExitFunction);
    } else if constexpr (ScopeGuardTypeValue == ScopeGuardType::Fail) {
      if (std::uncaught_exceptions() > m_ExceptionCount) {
        std::invoke(m_ExitFunction);
      }
    } else if constexpr (ScopeGuardTypeValue == ScopeGuardType::Success) {
      if (std::uncaught_exceptions() == m_ExceptionCount) {
        std::invoke(m_ExitFunction);
      }
    }
  }

private:

  ExitFunc                                 m_ExitFunction;
  bool                                     m_Active;
  [[no_unique_address]] ExceptionCountType m_ExceptionCount;
};
}  // namespace detail

// ============================================================================
// ScopeExit - Executes regardless of whether an exception occurs
// ============================================================================
template<detail::ExitFunction ExitFunc>
class [[nodiscard]] ScopeExit
    : public detail::ScopeGuardBase<ExitFunc, detail::ScopeGuardType::Exit>
{
  using Base = detail::ScopeGuardBase<ExitFunc, detail::ScopeGuardType::Exit>;

public:

  using Base::Base;
};

template<typename ExitFunc>
ScopeExit(ExitFunc) -> ScopeExit<ExitFunc>;

// ============================================================================
// ScopeFail - Executes only when an exception occurs
// ============================================================================
template<detail::ExitFunction ExitFunc>
class [[nodiscard]] ScopeFail
    : public detail::ScopeGuardBase<ExitFunc, detail::ScopeGuardType::Fail>
{
  using Base = detail::ScopeGuardBase<ExitFunc, detail::ScopeGuardType::Fail>;

public:

  using Base::Base;
};

template<typename ExitFunc>
ScopeFail(ExitFunc) -> ScopeFail<ExitFunc>;

// ============================================================================
// ScopeSuccess - Executes only on normal exit
// ============================================================================
template<detail::ExitFunction ExitFunc>
class [[nodiscard]] ScopeSuccess
    : public detail::ScopeGuardBase<ExitFunc, detail::ScopeGuardType::Success>
{
  using Base =
      detail::ScopeGuardBase<ExitFunc, detail::ScopeGuardType::Success>;

public:

  using Base::Base;
};

template<typename ExitFunc>
ScopeSuccess(ExitFunc) -> ScopeSuccess<ExitFunc>;

// ============================================================================
// Extended tool: ScopedResource
// ============================================================================
template<typename Resource, typename Deleter = std::default_delete<Resource>>
class [[nodiscard]] ScopedResource
{
public:

  ScopedResource(Resource* ptr, Deleter deleter = Deleter {}) noexcept
      : m_Ptr(ptr), m_Deleter(std::move(deleter))
  {
  }

  ScopedResource(std::unique_ptr<Resource, Deleter>&& unique_ptr) noexcept
      : m_Ptr(unique_ptr.release())
      , m_Deleter(std::move(unique_ptr.get_deleter()))
  {
  }

  ~ScopedResource()
  {
    if (m_Ptr) m_Deleter(m_Ptr);
  }

  ScopedResource(const ScopedResource&)             = delete;
  ScopedResource& operator= (const ScopedResource&) = delete;

  ScopedResource(ScopedResource&& other) noexcept
      : m_Ptr(std::exchange(other.m_Ptr, nullptr))
      , m_Deleter(std::move(other.m_Deleter))
  {
  }

  ScopedResource& operator= (ScopedResource&& other) noexcept
  {
    if (this != &other) {
      reset();
      m_Ptr     = std::exchange(other.m_Ptr, nullptr);
      m_Deleter = std::move(other.m_Deleter);
    }
    return *this;
  }

  // Resource access
  [[nodiscard]] Resource* get() const noexcept { return m_Ptr; }

  [[nodiscard]] Resource* operator->() const noexcept { return m_Ptr; }

  [[nodiscard]] Resource& operator* () const noexcept { return *m_Ptr; }

  [[nodiscard]] Resource* release() noexcept
  {
    return std::exchange(m_Ptr, nullptr);
  }

  void reset(Resource* new_ptr = nullptr) noexcept
  {
    if (m_Ptr) m_Deleter(m_Ptr);
    m_Ptr = new_ptr;
  }

  // Boolean conversion
  explicit operator bool() const noexcept { return m_Ptr != nullptr; }

  // Get deleter
  [[nodiscard]] Deleter& get_deleter() noexcept { return m_Deleter; }

  [[nodiscard]] const Deleter& get_deleter() const noexcept
  {
    return m_Deleter;
  }

private:

  Resource*                     m_Ptr;
  [[no_unique_address]] Deleter m_Deleter;
};

// ============================================================================
// Extended tool: ScopeGuard (generic RAII wrapper)
// ============================================================================
template<typename InitFunc, typename CleanupFunc>
class [[nodiscard]] ScopeGuard
{
public:

  ScopeGuard(InitFunc&& init, CleanupFunc&& cleanup)
      : m_Cleanup(std::forward<CleanupFunc>(cleanup))
  {
    std::invoke(init);
  }

  ~ScopeGuard()
  {
    if (m_Active) std::invoke(m_Cleanup);
  }

  ScopeGuard(const ScopeGuard&)             = delete;
  ScopeGuard& operator= (const ScopeGuard&) = delete;

  ScopeGuard(ScopeGuard&& other) noexcept
      : m_Cleanup(std::move(other.m_Cleanup))
      , m_Active(std::exchange(other.m_Active, false))
  {
  }

  ScopeGuard& operator= (ScopeGuard&& other) noexcept
  {
    if (this != &other) {
      if (m_Active) std::invoke(m_Cleanup);

      m_Cleanup = std::move(other.m_Cleanup);
      m_Active  = std::exchange(other.m_Active, false);
    }
    return *this;
  }

  void release() noexcept { m_Active = false; }

  [[nodiscard]] bool is_active() const noexcept { return m_Active; }

private:

  CleanupFunc m_Cleanup;
  bool        m_Active = true;
};

// ============================================================================
// Factory functions (for ease of use)
// ============================================================================
template<typename ExitFunc>
[[nodiscard]] auto make_scope_exit(ExitFunc&& exit_function)
{
  return ScopeExit<std::decay_t<ExitFunc>>(
      std::forward<ExitFunc>(exit_function));
}

template<typename ExitFunc>
[[nodiscard]] auto make_scope_fail(ExitFunc&& exit_function)
{
  return ScopeFail<std::decay_t<ExitFunc>>(
      std::forward<ExitFunc>(exit_function));
}

template<typename ExitFunc>
[[nodiscard]] auto make_scope_success(ExitFunc&& exit_function)
{
  return ScopeSuccess<std::decay_t<ExitFunc>>(
      std::forward<ExitFunc>(exit_function));
}

template<typename InitFunc, typename CleanupFunc>
[[nodiscard]] auto make_scope_guard(InitFunc&& init, CleanupFunc&& cleanup)
{
  return ScopeGuard<std::decay_t<InitFunc>, std::decay_t<CleanupFunc>>(
      std::forward<InitFunc>(init), std::forward<CleanupFunc>(cleanup));
}

// Helper macros: generate unique variable names
#define STDEX_SCOPE_CONCAT_IMPL(x, y)   x##y
#define STDEX_SCOPE_CONCAT(x, y)        STDEX_SCOPE_CONCAT_IMPL(x, y)
#define STDEX_SCOPE_ANONYMOUS_VAR(name) STDEX_SCOPE_CONCAT(name, __LINE__)

// Macro definitions, note: macros change scope, use with caution
#define STDEX_SCOPE_EXIT(...)                                 \
  auto STDEX_SCOPE_ANONYMOUS_VAR(scope_exit_) =               \
      stdex::make_scope_exit([&]() noexcept { __VA_ARGS__; })

#define STDEX_SCOPE_FAIL(...)                                 \
  auto STDEX_SCOPE_ANONYMOUS_VAR(scope_fail_) =               \
      stdex::make_scope_fail([&]() noexcept { __VA_ARGS__; })

#define STDEX_SCOPE_SUCCESS(...)                                 \
  auto STDEX_SCOPE_ANONYMOUS_VAR(scope_success_) =               \
      stdex::make_scope_success([&]() noexcept { __VA_ARGS__; })
}  // namespace stdex

// ============================================================================
// Usage Examples
// ============================================================================
/*
#include <iostream>
#include <fstream>
#include <memory>
#include <stdexcept>

int main() {
    try {
        // 1. ScopeExit - Always executes
        {
            std::cout << "1. ScopeExit example:\n";
            auto file = std::fopen("test.txt", "w");
            if (!file) return 0;

            auto guard = stdex::make_scope_exit([&]() noexcept {
                std::cout << "Closing file (ScopeExit)\n";
                std::fclose(file);
            });

            std::fprintf(file, "Hello, ScopeExit!\n");
            // File will be closed when scope ends, regardless of exceptions
        }

        // 2. ScopeFail - Executes only on exception
        {
            std::cout << "\n2. ScopeFail example:\n";
            bool cleanup_called = false;

            try {
                auto guard = stdex::make_scope_fail([&]() noexcept {
                    cleanup_called = true;
                    std::cout << "Cleanup on exception (ScopeFail)\n";
                });

                throw std::runtime_error("Test exception");
            } catch (...) {
                std::cout << "Exception caught. Cleanup called: "
                          << std::boolalpha << cleanup_called << "\n";
            }
        }

        // 3. ScopeSuccess - Executes only on normal exit
        {
            std::cout << "\n3. ScopeSuccess example:\n";
            bool success_called = false;

            try {
                auto guard = stdex::make_scope_success([&]() noexcept {
                    success_called = true;
                    std::cout << "Success path (ScopeSuccess)\n";
                });

                // Normal execution, no exception thrown
            } catch (...) {
                // Won't reach here
            }

            std::cout << "Success handler called: "
                      << std::boolalpha << success_called << "\n";
        }

        // 4. Using macros (similar to C#/Java finally)
        {
            std::cout << "\n4. Macro example:\n";
            STDEX_SCOPE_EXIT(
                std::cout << "This will always execute (like finally)\n";
            );

            STDEX_SCOPE_FAIL(
                std::cout << "This only executes on exception\n";
            );

            STDEX_SCOPE_SUCCESS(
                std::cout << "This only executes on normal exit\n";
            );
        }

        // 5. ScopedResource
        {
            std::cout << "\n5. ScopedResource example:\n";

            // Custom deleter
            auto deleter = [](FILE* file) {
                if (file) {
                    std::cout << "Custom file deleter\n";
                    std::fclose(file);
                }
            };

            stdex::ScopedResource<FILE, decltype(deleter)>
                file(std::fopen("test.txt", "w"), deleter);

            if (file) {
                std::fprintf(file.get(), "Hello from ScopedResource!\n");
            }
            // File automatically closed
        }

        // 6. ScopeGuard (generic RAII)
        {
            std::cout << "\n6. ScopeGuard example:\n";

            int* array = new int[100];

            auto guard = stdex::make_scope_guard(
                [&]() { std::cout << "Array initialized\n"; },
                [&]() {
                    std::cout << "Cleaning up array\n";
                    delete[] array;
                }
            );

            // Use array...
            array[0] = 42;
        }
    }
    catch (const std::exception& exception) {
        std::cerr << "Error: " << exception.what() << "\n";
    }
    return 0;
}
*/