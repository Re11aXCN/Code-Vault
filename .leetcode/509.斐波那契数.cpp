/*
 * @lc app=leetcode.cn id=509 lang=cpp
 *
 * [509] 斐波那契数
 */

// @lc code=start
class Solution {
public:
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

        constexpr Matrix2D operator*(const Matrix2D& other) const {
            return Matrix2D(
                data[0][0] * other.data[0][0] + data[0][1] * other.data[1][0],
                data[0][0] * other.data[0][1] + data[0][1] * other.data[1][1],
                data[1][0] * other.data[0][0] + data[1][1] * other.data[1][0],
                data[1][0] * other.data[0][1] + data[1][1] * other.data[1][1]
            );
        }

        constexpr Matrix2D& operator*=(const Matrix2D& other) {
            return *this = *this * other;
        }
    };
    constexpr Matrix2D matrixPower(const Matrix2D& base, std::size_t exponent) {
        Matrix2D result;
        Matrix2D temp = base;
        std::size_t n = exponent;

        while (n > 0) {
            if (n & 1) {
                result *= temp;
            }
            temp *= temp;
            n >>= 1;
        }
        return result;
    }

    constexpr std::size_t fibonacci(std::size_t n) {
        if (n == 0) return 0;

        Matrix2D fibMatrix(1, 1, 1, 0);
        Matrix2D result = matrixPower(fibMatrix, n - 1);

        return result.data[0][0];
    }
    constexpr std::size_t fib(std::size_t n) {
        return fibonacci(n);
    }
};
// @lc code=end

