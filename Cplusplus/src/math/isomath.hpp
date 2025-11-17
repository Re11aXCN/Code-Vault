#pragma once

#include <array>
#include <cstddef>
// Integer Sequences of Mathematical Functions
// https://oeis.org/
namespace isomath { 
    namespace detail {
        struct Matrix2D {
            std::array<std::array<std::size_t, 2>, 2> data;// will be plain array

            constexpr Matrix2D() : data{ {{1, 0}, {0, 1}} } {}

            constexpr Matrix2D(std::size_t a, std::size_t b,
                std::size_t c, std::size_t d)
                : data{ {{a, b}, {c, d}} } {
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
                return data[0][0] == other.data[0][0] &&
                    data[0][1] == other.data[0][1] &&
                    data[1][0] == other.data[1][0] &&
                    data[1][1] == other.data[1][1];
            }

            constexpr Matrix2D operator+(const Matrix2D& other) const {
                return Matrix2D(
                    data[0][0] + other.data[0][0],
                    data[0][1] + other.data[0][1],
                    data[1][0] + other.data[1][0],
                    data[1][1] + other.data[1][1]
                );
            }

            constexpr Matrix2D operator+(const std::size_t& scalar) const {
                return Matrix2D(
                    data[0][0] + scalar,
                    data[0][1] + scalar,
                    data[1][0] + scalar,
                    data[1][1] + scalar
                );
            }

            constexpr Matrix2D operator-(const Matrix2D& other) const {
                return Matrix2D(
                    data[0][0] - other.data[0][0],
                    data[0][1] - other.data[0][1],
                    data[1][0] - other.data[1][0],
                    data[1][1] - other.data[1][1]
                );
            }

            constexpr Matrix2D operator-(const std::size_t& scalar) const {
                return Matrix2D(
                    data[0][0] - scalar,
                    data[0][1] - scalar,
                    data[1][0] - scalar,
                    data[1][1] - scalar
                );
            }

            constexpr Matrix2D operator*(const std::size_t& scalar) const {
                return Matrix2D(
                    data[0][0] * scalar,
                    data[0][1] * scalar,
                    data[1][0] * scalar,
                    data[1][1] * scalar
                );
            }

            constexpr auto operator*(const Matrix2D& other) const -> Matrix2D {
                return Matrix2D(
                    data[0][0] * other.data[0][0] + data[0][1] * other.data[1][0],
                    data[0][0] * other.data[0][1] + data[0][1] * other.data[1][1],
                    data[1][0] * other.data[0][0] + data[1][1] * other.data[1][0],
                    data[1][0] * other.data[0][1] + data[1][1] * other.data[1][1]
                );
            }

            constexpr Matrix2D& operator*=(const Matrix2D& other) {
                //return *this = *this * other;
                auto [a, b, c, d] = data;
                auto [e, f, g, h] = other.data;

                data[0][0] = a * e + b * g;
                data[0][1] = a * f + b * h;
                data[1][0] = c * e + d * g;
                data[1][1] = c * f + d * h;

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

    inline constexpr auto [[nodiscard]] fibonacci(std::size_t n) -> std::size_t {
        using namespace detail;
        if (n == 0) return 0;

        Matrix2D fibMatrix(1, 1, 1, 0);
        Matrix2D result = Matrix2D::matrixPower(fibMatrix, n - 1);

        return result.data[0][0];
    }

    inline constexpr auto [[nodiscard]] catalan(std::size_t n) -> std::size_t {
        return iteration::catalan(n);
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
}
