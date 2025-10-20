#include "header.h"
#include <boost/container/small_vector.hpp>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif
using std::string;
using std::vector;
using std::queue;
#include <iostream>
#include <string>
#include <tuple>
#include <concepts>

// 基础概念定义
template <class T>
concept common_trait = requires(T t) {
    { t.origin } -> std::same_as<int&>;
    { t.size } -> std::same_as<double&>;
};

template <class T>
concept drawable_trait = common_trait<T> && std::invocable<T, std::string&>;

// 基础Impl结构
struct ContainerProps {
    int origin = 0;
    double size = 0.0;
};

// 第一个Impl实现 - 简单平移
struct SurfaceImpl : public ContainerProps {
    constexpr explicit SurfaceImpl(double size, int origin = 0)
        : ContainerProps{ origin, size } {
    }

    template<typename... Ts>
    void make_layout(std::tuple<Ts...>& drawable) {
        std::apply([this](auto&... d) { ((this->make(d)), ...); }, drawable);
    }

    void make(drawable_trait auto& drawable) {
        drawable.origin += this->origin;  // 简单平移
    }
};

// 第二个Impl实现 - 居中对齐
struct BoxImpl : public ContainerProps {
    constexpr explicit BoxImpl(double size, int origin = 0)
        : ContainerProps{ origin, size } {
    }

    template<typename... Ts>
    void make_layout(std::tuple<Ts...>& drawable) {
        std::apply([this](auto&... d) { ((this->make(d)), ...); }, drawable);
    }

    void make(drawable_trait auto& drawable) {
        // 居中对齐逻辑
        double offset = (this->size - drawable.size) / 2.0;
        drawable.origin = this->origin + static_cast<int>(offset);
    }
};

// Container定义
template <class Impl, drawable_trait... Ts>
struct Container : public Impl {
    std::tuple<std::decay_t<Ts>...> drawable;

    constexpr explicit Container(const Impl& impl, Ts&&... drawable)
        : Impl{ impl }
        , drawable{ std::make_tuple(std::forward<Ts>(drawable)...) } {
    }

    auto operator()(std::string& painter)
        requires(std::invocable<Ts, std::string&> && ...)
    {
        render(painter);
    }

    auto render(std::string& painter) noexcept {
        // 应用布局
        this->make_layout(drawable);

        // 渲染所有drawable
        auto render_func = [&](auto&... d) {
            (d(painter), ...);
            };
        std::apply(render_func, drawable);
    }
};

// 推导指引
template<typename... Ts>
Container(const SurfaceImpl& impl, Ts&&... args) -> Container<SurfaceImpl, Ts...>;

template<typename... Ts>
Container(const BoxImpl& impl, Ts&&... args) -> Container<BoxImpl, Ts...>;

// 测试用的简单drawable类型
struct SimpleDrawable {
    int origin = 0;
    double size = 0.0;
    std::string name;

    void operator()(std::string& painter) {
        std::cout << name << " drawn at origin: " << origin
            << ", size: " << size << " with painter: " << painter << std::endl;
    }
};

// 验证common_trait和drawable_trait
static_assert(common_trait<SimpleDrawable>);
static_assert(drawable_trait<SimpleDrawable>);

int main() {
    std::string painter = "main_painter";

    // 创建基础drawable对象
    SimpleDrawable child1{ 0, 20.0, "Child1" };
    SimpleDrawable child2{ 0, 30.0, "Child2" };
    SimpleDrawable child3{ 0, 25.0, "Child3" };

    std::cout << "=== 基础Container测试 ===" << std::endl;

    // 测试SurfaceImpl Container
    SurfaceImpl surfaceImpl(100.0, 10);
    Container surfaceContainer(surfaceImpl, child1, child2);
    surfaceContainer(painter);

    std::cout << "\n=== BoxImpl Container测试 ===" << std::endl;

    // 测试BoxImpl Container  
    BoxImpl boxImpl(200.0, 50);
    Container boxContainer(boxImpl, child3);
    boxContainer(painter);

    std::cout << "\n=== 嵌套Container测试 ===" << std::endl;

    // 嵌套测试：Box容器包含Surface容器
    SimpleDrawable child4{ 0, 15.0, "Child4" };

    // 创建内层Surface容器
    SurfaceImpl innerImpl(80.0, 5);
    Container innerContainer(innerImpl, child1, child2);

    // 创建外层Box容器，包含内层容器和另一个child
    BoxImpl outerImpl(300.0, 100);
    Container outerContainer(outerImpl, innerContainer, child4);

    outerContainer(painter);

    std::cout << "\n=== 多层嵌套测试 ===" << std::endl;

    // 三层嵌套测试
    SimpleDrawable child5{ 0, 10.0, "Child5" };

    // 最内层
    SurfaceImpl deepestImpl(60.0, 2);
    Container deepestContainer(deepestImpl, child5);

    // 中间层
    BoxImpl middleImpl(150.0, 20);
    Container middleContainer(middleImpl, deepestContainer, child3);

    // 最外层
    SurfaceImpl topImpl(400.0, 0);
    Container topContainer(topImpl, middleContainer, child4);

    topContainer(painter);

    return 0;
}
//int main() {
//#ifdef _WIN32
//    SetConsoleOutputCP(CP_UTF8); // 输出 UTF-8 编码
//#endif
//    std::cout.setf(std::ios_base::boolalpha);
//    auto benchmark = [](std::string_view name, auto&& func) {
//        auto start = std::chrono::high_resolution_clock::now();
//        func();
//        auto end = std::chrono::high_resolution_clock::now();
//        std::println("{} took {}us", name, std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
//        };
//    auto generate_test_data = [](size_t n, int min_val, int max_val) {
//        if(n <= 1) throw std::invalid_argument("n must be greater than 1");
//        std::random_device rd;
//        std::mt19937 gen(rd());
//        std::uniform_int_distribution<int> dist(min_val, max_val);
//
//        std::vector<int> data(n);
//        for (size_t i = 0; i < n; ++i) {
//            data[i] = dist(gen);
//        }
//        return data;
//        };
//
//    return 0;
//}
