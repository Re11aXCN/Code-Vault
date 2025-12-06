#pragma once

#include <array>
#include <cstddef>
#include <cmath>
// Integer Sequences of Mathematical Functions
// https://oeis.org/
namespace isomath { 
    namespace detail {
        struct Matrix2D {
            std::array<std::size_t, 4> data;// will be plain array

            constexpr Matrix2D() : data{ } {}

            constexpr Matrix2D(std::size_t a, std::size_t b,
                std::size_t c, std::size_t d)
                : data{ a, b, c, d } {
            }

            constexpr Matrix2D(const Matrix2D& other) : data{ other.data } {}
            constexpr Matrix2D& operator=(const Matrix2D& other) {
                data = other.data;
                return *this;
            }
            constexpr Matrix2D(Matrix2D&& other) noexcept : data{ other.data } {}
            constexpr Matrix2D& operator=(Matrix2D&& other) noexcept {
                data = other.data;
                return *this;
            }

            constexpr bool operator==(const Matrix2D& other) const {
                return data == other.data;
            }

            constexpr Matrix2D operator+(const Matrix2D& other) const {
                return Matrix2D(data[0] + other.data[0], data[1] + other.data[1], data[2] + other.data[2],data[3] + other.data[3]);
            }

            constexpr Matrix2D operator+(const std::size_t& scalar) const {
                return Matrix2D(data[0] + scalar, data[1] + scalar, data[2] + scalar, data[3] + scalar);
            }

            constexpr Matrix2D operator-(const Matrix2D& other) const {
                return Matrix2D(data[0] - other.data[0], data[1] - other.data[1], data[2] - other.data[2], data[3] - other.data[3]);
            }

            constexpr Matrix2D operator-(const std::size_t& scalar) const {
                return Matrix2D(data[0] - scalar, data[1] - scalar, data[2] - scalar, data[3] - scalar);
            }

            constexpr Matrix2D operator*(const std::size_t& scalar) const {
                return Matrix2D(data[0] * scalar, data[1] * scalar, data[2] * scalar, data[3] * scalar);
            }

            constexpr auto operator*(const Matrix2D& other) const -> Matrix2D {
                return Matrix2D(
                    data[0] * other.data[0] + data[1] * other.data[2],
                    data[0] * other.data[1] + data[1] * other.data[3],
                    data[2] * other.data[0] + data[3] * other.data[2],
                    data[2] * other.data[1] + data[3] * other.data[3]
                );
            }

            constexpr Matrix2D& operator*=(const Matrix2D& other) {
                /*
                不能这样写，如果是自身赋值，，temp *= temp会导致计算结果错误，因为temp被修改了，也就是other.data被修改了
                auto [a, b, c, d] = data;
                data[0] = a * other.data[0] + b * other.data[2];
                data[1] = a * other.data[1] + b * other.data[3];
                data[2] = c * other.data[0] + d * other.data[2];
                data[3] = c * other.data[1] + d * other.data[3];
                */
                //return *this = *this * other;
                auto [a, b, c, d] = data;
                auto [e, f, g, h] = other.data;

                data[0] = a * e + b * g;
                data[1] = a * f + b * h;
                data[2] = c * e + d * g;
                data[3] = c * f + d * h;

                return *this;
            }

            static constexpr auto matrixPower(const Matrix2D& base, std::size_t exponent) -> Matrix2D {
                Matrix2D result;
                Matrix2D temp = base;
                std::size_t n = exponent;

                while (n > 0) {
                    if (n & 1) result *= temp;
                    temp *= temp;
                    n >>= 1;
                }
                return result;
            }
        };
    }

    namespace recursive {
        inline constexpr auto [[nodiscard]] fibonacci(std::size_t n) -> std::size_t {
            if (n == 0)return 0;
            if (n == 1) return 1;
            return fibonacci(n - 1) + fibonacci(n - 2);
        }

        inline constexpr auto [[nodiscard]] catalan(std::size_t n) -> std::size_t {
            if (n == 0) return 1;

            std::size_t result = 0;
            for (std::size_t i = 0; i < n; ++i) {
                result += catalan(i) * catalan(n - 1 - i);
            }
            return result;
        }
    }

    namespace iteration {
        inline constexpr auto [[nodiscard]] fibonacci(std::size_t n) -> std::size_t {
            if (n == 0) return 0;
            if (n == 1) return 1;
            std::size_t a = 0, b = 1, c;
            for (int i = 2; i <= n; ++i) {
                c = a + b;
                a = b;
                b = c;
            }
            return b;
        }
        inline constexpr auto [[nodiscard]] catalan(std::size_t n) -> std::size_t {
            if (n == 0) return 1;

            std::size_t result = 1;
            // 使用迭代公式: C(n) = (2n)! / (n!(n+1)!)
            // 等价于: C(n) = ∏(i=1 to n) (4i-2)/(i+1)
            for (std::size_t i = 1; i <= n; ++i) {
                result = result * (4 * i - 2) / (i + 1);
            }

            return result;
        }
    }

    inline constexpr auto [[nodiscard]] fibonacci(std::size_t n) -> std::size_t {
        using namespace detail;
        if (n == 0) return 0;

        Matrix2D fibMatrix(1, 1, 1, 0);
        Matrix2D result = Matrix2D::matrixPower(fibMatrix, n - 1);

        return result.data[0];
    }

    inline constexpr auto [[nodiscard]] catalan(std::size_t n) -> std::size_t {
        return iteration::catalan(n);
    }

    /*
// 针对浮点运算的优化
__attribute__((optimize("-O3 -funsafe-math-optimizations"))) 

// 允许倒数使用（用乘法代替除法）
__attribute__((optimize("-O3 -freciprocal-math"))) 

// 单精度浮点优化  
__attribute__((optimize("-O3 -fsingle-precision-constant")))

// 更激进的数学优化
__attribute__((optimize("-O3 -ffast-math")))
    */
    std::size_t catalan_tgamma(std::size_t n) {
        if (n == 0) return 1;

        // 使用伽马函数计算组合数 C(2n, n)
        double comb = std::tgamma(2 * n + 1) / (std::tgamma(n + 1) * std::tgamma(n + 1));

        // 卡特兰数公式: C(n) = C(2n, n) / (n + 1)
        return comb / (n + 1.0) + 0.1;
    }

    // 只能计算前15个精确数
    std::size_t catalan_tgammaf(std::size_t n) {
        if (n == 0) return 1;
        float comb = std::tgammaf(2 * n + 1) / (std::tgammaf(n + 1) * std::tgammaf(n + 1));
        return comb / (n + 1.0f) + 0.1f;
    }

    // faster than catalan_tgamma
    std::size_t catalan_lgamma(std::size_t n) {
        if (n == 0) return 1;

        // 使用对数伽马函数避免大数溢出
        double log_comb = std::lgamma(2 * n + 1) - 2 * std::lgamma(n + 1);

        // 卡特兰数公式: C(n) = C(2n, n) / (n + 1)
        return std::exp(log_comb + 1e-10) / (n + 1.0);
    }
    // 只能计算前12个精确数
    std::size_t catalan_lgammaf(std::size_t n) {
        if (n == 0) return 1;
        float log_comb = std::lgammaf(2 * n + 1) - 2 * std::lgammaf(n + 1);
        return std::expf(log_comb + 1e-5f) / (n + 1.0f);
    }

    // see Code-Vault/.leetcode/markdown/牛顿迭代法.md
    double sqrt_newton(double x, double tolerance) {
        if (x < 0) return NAN;
        if (x == 0) return 0;

        double guess = x / 2.0;
        double prev = 0.0;

        while (std::abs(guess - prev) > tolerance) {
            prev = guess;
            guess = 0.5 * (guess + x / guess);
        }

        return guess;
    }
}
