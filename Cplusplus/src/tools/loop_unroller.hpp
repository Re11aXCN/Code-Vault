#pragma once

#include <utility>
#include <type_traits>

template <size_t UnrollFactor>
class LoopUnroller {
public:
    // 基本循环展开
    template <typename F, typename... Args>
    static void execute(F&& f, Args&&... args) {
        if constexpr (UnrollFactor > 0) {
            [&] <size_t... Is>(std::index_sequence<Is...>) {
                ((void)Is, ..., std::forward<F>(f)(std::forward<Args>(args)...));
            }(std::make_index_sequence<UnrollFactor>{});
        }
    }

    // 带索引的循环展开
    template <typename F>
    static void execute_with_index(F&& f) {
        if constexpr (UnrollFactor > 0) {
            [&] <size_t... Is>(std::index_sequence<Is...>) {
                (std::forward<F>(f)(Is), ...);
            }(std::make_index_sequence<UnrollFactor>{});
        }
    }

    // 批量处理数组（需要知道数组大小）
    template <typename T, typename F>
    static void process_array(T* data, size_t size, F&& f) {
        constexpr size_t step = UnrollFactor;
        size_t i = 0;

        // 展开处理的部分
        for (; i + step <= size; i += step) {
            execute([&](auto) {
                f(data[i + decltype(Is){}]);
                });
        }

        // 处理剩余元素
        for (; i < size; ++i) {
            f(data[i]);
        }
    }
};

/*
int main() {
    // 示例1: 基本循环展开
    std::cout << "基本循环展开:\n";
    LoopUnroller<4>::execute([](auto) {
        std::cout << "Hello ";
    });
    std::cout << "\n\n";

    // 示例2: 带索引的循环展开
    std::cout << "带索引的循环展开:\n";
    LoopUnroller<4>::execute_with_index([](size_t i) {
        std::cout << "Index: " << i << " ";
    });
    std::cout << "\n\n";

    // 示例3: 数组处理
    std::vector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::cout << "数组处理:\n";
    LoopUnroller<4>::process_array(vec.data(), vec.size(), [](int val) {
        std::cout << val * 2 << " ";
    });
    std::cout << "\n\n";

    // 示例4: 使用unroll_with_index
    std::cout << "使用unroll_with_index:\n";
    unroll_with_index<5>([](size_t i) {
        std::cout << "Square[" << i << "] = " << i * i << "\n";
    });

    return 0;
}
*/