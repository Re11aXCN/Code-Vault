#include <array>
#include <type_traits>
namespace stdex {
    template <typename T, std::size_t N, std::size_t... Is>
    constexpr auto make_array_impl(T&& value, std::index_sequence<Is...>) {
        // remove cvref from T
        // 逗号运算符：返回最后一个表达式的结果（std::forward<T>(value)）
        return std::array<std::decay_t<T>, N>{ (static_cast<void>(Is), std::forward<T>(value))... };
        /*
         return std::array<std::decay_t<T>, N>{ 
            (void)Is...,  // 展开并丢弃所有Is
            std::forward<T>(value)  // 最后一个值用作初始化
        };
        // like this:
        return std::array<int, 3>{
            (static_cast<void>(0), std::forward<T>(value)), // 类似宏的"挤兑"
            (static_cast<void>(1), std::forward<T>(value)),
            (static_cast<void>(2), std::forward<T>(value))
        };
        */
    }

    template <typename T, std::size_t N>
    [[nodiscard]] constexpr auto make_array(T&& value) {
        return make_array_impl<T, N>(std::forward<T>(value), std::make_index_sequence<N>());
    }

    template<typename T>
    [[nodiscard]] constexpr T& as_mutable(T const& t) noexcept {
        return const_cast<T&>(t);
    }

    template<typename T> requires (!std::is_lvalue_reference_v<T>)
    [[nodiscard]]  constexpr T& as_mutable(T&& t) noexcept {
        return const_cast<T&>(t);
    }

}