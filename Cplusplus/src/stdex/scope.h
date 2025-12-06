#pragma once

#include <concepts>
#include <exception>
#include <functional>
#include <type_traits>
#include <utility>

namespace stdex {

    namespace detail {

        // 概念：检查类型是否可调用且不抛出异常
        template<typename F, typename... Args>
        concept NothrowInvocable =
            std::invocable<F, Args...> &&
            noexcept(std::invoke(std::declval<F>(), std::declval<Args>()...));

        // 概念：检查是否为有效的退出函数类型
        template<typename EF>
        concept ExitFunction = requires(EF & ef)
        {
            requires std::invocable<EF&> || std::invocable<std::remove_reference_t<EF>&> ||
        std::invocable<EF> || std::invocable<std::remove_reference_t<EF>>;

        { std::invoke(ef) } -> std::same_as<void>;
        };
        // Always - Only occurs with an exception - As long as no exceptions occur
        enum class scope_guard_type { exit, fail, success };

        template<typename EF, scope_guard_type ScopeGuardType>
        class [[nodiscard]] scope_guard_base {
            struct dummy {};
            using exception_count_t = std::conditional_t<ScopeGuardType != scope_guard_type::exit, int, dummy>;
        public:
            template<typename F> requires ExitFunction<F>
            explicit scope_guard_base(F&& func) noexcept(std::is_nothrow_constructible_v<EF, F>)
                : exit_function_(std::forward<F>(func))
                , active_(true)
            {
                if constexpr (ScopeGuardType != scope_guard_type::exit) exception_count_ = std::uncaught_exceptions();
            }

            scope_guard_base(scope_guard_base&& other) noexcept(std::is_nothrow_move_constructible_v<EF>)
                : exit_function_(std::move(other.exit_function_))
                , active_(std::exchange(other.active_, false))
            {
                if constexpr (ScopeGuardType != scope_guard_type::exit) exception_count_ = other.exception_count_;
            }

            scope_guard_base(const scope_guard_base&) = delete;
            scope_guard_base& operator=(const scope_guard_base&) = delete;

            scope_guard_base& operator=(scope_guard_base&& other) noexcept(std::is_nothrow_move_assignable_v<EF>&& std::is_nothrow_move_constructible_v<EF>)
            {
                if (this != &other) {
                    if (active_) execute_if_needed();

                    exit_function_ = std::move(other.exit_function_);
                    active_ = std::exchange(other.active_, false);
                    if constexpr (ScopeGuardType != scope_guard_type::exit) exception_count_ = other.exception_count_;
                }
                return *this;
            }

            ~scope_guard_base() noexcept(noexcept(std::invoke(exit_function_)))
            {
                if (active_) execute_if_needed();
            }

            void release() noexcept { active_ = false; }

            [[nodiscard]] bool is_active() const noexcept { return active_; }

        protected:
            void execute_if_needed()
            {
                if constexpr (ScopeGuardType == scope_guard_type::exit) {
                    std::invoke(exit_function_);
                }
                else if constexpr (ScopeGuardType == scope_guard_type::fail) {
                    if (std::uncaught_exceptions() > exception_count_) {
                        std::invoke(exit_function_);
                    }
                }
                else if constexpr (ScopeGuardType == scope_guard_type::success) {
                    if (std::uncaught_exceptions() == exception_count_) {
                        std::invoke(exit_function_);
                    }
                }
            }

        private:
            EF exit_function_;
            bool active_;
            [[no_unique_address]] exception_count_t exception_count_;
        };

    } // namespace detail

    // ============================================================================
    // scope_exit - 无论是否发生异常都执行
    // ============================================================================
    template<detail::ExitFunction EF>
    class [[nodiscard]] scope_exit : public detail::scope_guard_base<EF, detail::scope_guard_type::exit> {
        using base = detail::scope_guard_base<EF, detail::scope_guard_type::exit>;
    public:
        using base::base;
    };
    template<typename EF>
    scope_exit(EF) -> scope_exit<EF>;

    // ============================================================================
    // scope_fail - 仅在发生异常时执行
    // ============================================================================
    template<detail::ExitFunction EF>
    class [[nodiscard]] scope_fail : public detail::scope_guard_base<EF, detail::scope_guard_type::fail> {
        using base = detail::scope_guard_base<EF, detail::scope_guard_type::fail>;
    public:
        using base::base;
    };
    template<typename EF>
    scope_fail(EF) -> scope_fail<EF>;

    // ============================================================================
    // scope_success - 仅在正常退出时执行
    // ============================================================================
    template<detail::ExitFunction EF>
    class [[nodiscard]] scope_success : public detail::scope_guard_base<EF, detail::scope_guard_type::success> {
        using base = detail::scope_guard_base<EF, detail::scope_guard_type::success>;
    public:
        using base::base;
    };
    template<typename EF>
    scope_success(EF) -> scope_success<EF>;

    // ============================================================================
    // 扩展工具：scoped_resource
    // ============================================================================
    template<typename Resource, typename Deleter = std::default_delete<Resource>>
    class [[nodiscard]] scoped_resource {
    public:
        scoped_resource(Resource* ptr, Deleter deleter = Deleter{}) noexcept
            : ptr_(ptr), deleter_(std::move(deleter))
        {
        }

        scoped_resource(std::unique_ptr<Resource, Deleter>&& uptr) noexcept
            : ptr_(uptr.release()), deleter_(std::move(uptr.get_deleter()))
        {
        }

        ~scoped_resource()
        {
            if (ptr_)  deleter_(ptr_);
        }

        scoped_resource(const scoped_resource&) = delete;
        scoped_resource& operator=(const scoped_resource&) = delete;

        scoped_resource(scoped_resource&& other) noexcept
            : ptr_(std::exchange(other.ptr_, nullptr))
            , deleter_(std::move(other.deleter_))
        {
        }

        scoped_resource& operator=(scoped_resource&& other) noexcept
        {
            if (this != &other) {
                reset();
                ptr_ = std::exchange(other.ptr_, nullptr);
                deleter_ = std::move(other.deleter_);
            }
            return *this;
        }

        // 资源访问
        [[nodiscard]] Resource* get() const noexcept { return ptr_; }
        [[nodiscard]] Resource* operator->() const noexcept { return ptr_; }
        [[nodiscard]] Resource& operator*() const noexcept { return *ptr_; }

        [[nodiscard]] Resource* release() noexcept { return std::exchange(ptr_, nullptr); }

        void reset(Resource* new_ptr = nullptr) noexcept
        {
            if (ptr_) deleter_(ptr_);
            ptr_ = new_ptr;
        }

        // 布尔转换
        explicit operator bool() const noexcept { return ptr_ != nullptr; }

        // 获取删除器
        [[nodiscard]] Deleter& get_deleter() noexcept { return deleter_; }
        [[nodiscard]] const Deleter& get_deleter() const noexcept { return deleter_; }

    private:
        Resource* ptr_;
        [[no_unique_address]] Deleter deleter_;
    };

    // ============================================================================
    // 扩展工具：scope_guard（通用RAII包装器）
    // ============================================================================
    template<typename InitFunc, typename CleanupFunc>
    class [[nodiscard]] scope_guard {
    public:
        scope_guard(InitFunc&& init, CleanupFunc&& cleanup)
            : cleanup_(std::forward<CleanupFunc>(cleanup))
        {
            std::invoke(init);
        }

        ~scope_guard()
        {
            if (active_) std::invoke(cleanup_);
        }

        scope_guard(const scope_guard&) = delete;
        scope_guard& operator=(const scope_guard&) = delete;

        scope_guard(scope_guard&& other) noexcept
            : cleanup_(std::move(other.cleanup_))
            , active_(std::exchange(other.active_, false))
        {
        }

        scope_guard& operator=(scope_guard&& other) noexcept
        {
            if (this != &other) {
                if (active_) std::invoke(cleanup_);

                cleanup_ = std::move(other.cleanup_);
                active_ = std::exchange(other.active_, false);
            }
            return *this;
        }

        void release() noexcept { active_ = false; }
        [[nodiscard]] bool is_active() const noexcept { return active_; }

    private:
        CleanupFunc cleanup_;
        bool active_ = true;
    };

    // ============================================================================
    // 工厂函数（便于使用）
    // ============================================================================
    template<typename EF>
    [[nodiscard]] auto make_scope_exit(EF&& ef) {
        return scope_exit<std::decay_t<EF>>(std::forward<EF>(ef));
    }

    template<typename EF>
    [[nodiscard]] auto make_scope_fail(EF&& ef) {
        return scope_fail<std::decay_t<EF>>(std::forward<EF>(ef));
    }

    template<typename EF>
    [[nodiscard]] auto make_scope_success(EF&& ef) {
        return scope_success<std::decay_t<EF>>(std::forward<EF>(ef));
    }

    template<typename InitFunc, typename CleanupFunc>
    [[nodiscard]] auto make_scope_guard(InitFunc&& init, CleanupFunc&& cleanup)
    {
        return scope_guard<std::decay_t<InitFunc>, std::decay_t<CleanupFunc>>(
            std::forward<InitFunc>(init),
            std::forward<CleanupFunc>(cleanup)
        );
    }

    // 宏定义，注意：宏会改变作用域，使用时需要注意
#define STREX_SCOPE_EXIT(...) \
    auto STREX_ANONYMOUS_VAR(scope_exit_) = stdex::make_scope_exit([&]() noexcept { __VA_ARGS__; })

#define STREX_SCOPE_FAIL(...) \
    auto STREX_ANONYMOUS_VAR(scope_fail_) = stdex::make_scope_fail([&]() noexcept { __VA_ARGS__; })

#define STREX_SCOPE_SUCCESS(...) \
    auto STREX_ANONYMOUS_VAR(scope_success_) = stdex::make_scope_success([&]() noexcept { __VA_ARGS__; })

// 辅助宏：生成唯一的变量名
#define STREX_CONCAT_IMPL(x, y) x##y
#define STREX_CONCAT(x, y) STREX_CONCAT_IMPL(x, y)
#define STREX_ANONYMOUS_VAR(name) STREX_CONCAT(name, __LINE__)
} // namespace stdex

// ============================================================================
// 使用示例
// ============================================================================
/*
#include <iostream>
#include <fstream>
#include <memory>
#include <stdexcept>

int main() {
    try {
        // 1. scope_exit - 总是执行
        {
            std::cout << "1. scope_exit example:\n";
            auto file = std::fopen("test.txt", "w");
            if (!file) return 0;

            auto guard = stdex::make_scope_exit([&]() noexcept {
                std::cout << "Closing file (scope_exit)\n";
                std::fclose(file);
            });

            std::fprintf(file, "Hello, scope_exit!\n");
            // 文件会在作用域结束时关闭，无论是否发生异常
        }

        // 2. scope_fail - 仅在异常时执行
        {
            std::cout << "\n2. scope_fail example:\n";
            bool cleanup_called = false;

            try {
                auto guard = stdex::make_scope_fail([&]() noexcept {
                    cleanup_called = true;
                    std::cout << "Cleanup on exception (scope_fail)\n";
                });

                throw std::runtime_error("Test exception");
            } catch (...) {
                std::cout << "Exception caught. Cleanup called: "
                          << std::boolalpha << cleanup_called << "\n";
            }
        }

        // 3. scope_success - 仅在正常退出时执行
        {
            std::cout << "\n3. scope_success example:\n";
            bool success_called = false;

            try {
                auto guard = stdex::make_scope_success([&]() noexcept {
                    success_called = true;
                    std::cout << "Success path (scope_success)\n";
                });

                // 正常执行，不会抛出异常
            } catch (...) {
                // 不会进入这里
            }

            std::cout << "Success handler called: "
                      << std::boolalpha << success_called << "\n";
        }

        // 4. 使用宏（类似C#/Java的finally）
        {
            std::cout << "\n4. Macro example:\n";
            STREX_SCOPE_EXIT(
                std::cout << "This will always execute (like finally)\n";
            );

            STREX_SCOPE_FAIL(
                std::cout << "This only executes on exception\n";
            );

            STREX_SCOPE_SUCCESS(
                std::cout << "This only executes on normal exit\n";
            );
        }

        // 5. scoped_resource
        {
            std::cout << "\n5. scoped_resource example:\n";

            // 自定义删除器
            auto deleter = [](FILE* f) {
                if (f) {
                    std::cout << "Custom file deleter\n";
                    std::fclose(f);
                }
            };

            stdex::scoped_resource<FILE, decltype(deleter)>
                file(std::fopen("test.txt", "w"), deleter);

            if (file) {
                std::fprintf(file.get(), "Hello from scoped_resource!\n");
            }
            // 文件自动关闭
        }

        // 6. scope_guard（通用RAII）
        {
            std::cout << "\n6. scope_guard example:\n";

            int* arr = new int[100];

            auto guard = stdex::make_scope_guard(
                [&]() { std::cout << "Array initialized\n"; },
                [&]() {
                    std::cout << "Cleaning up array\n";
                    delete[] arr;
                }
            );

            // 使用数组...
            arr[0] = 42;
        }
    } 
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
    return 0;
}
*/