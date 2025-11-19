class Solution {
public:
    struct Matrix2D {
        std::array<std::array<std::size_t, 2>, 2> data;// will be plain array

        constexpr Matrix2D() : data{ {{1, 0}, {0, 1}} } {}

        constexpr Matrix2D(std::size_t a, std::size_t b,
            std::size_t c, std::size_t d)
            : data{ {{a, b}, {c, d}} } {
        }

        constexpr Matrix2D operator*(const Matrix2D& other) const {
            return Matrix2D(
                data[0][0] * other.data[0][0] + data[0][1] * other.data[1][0],
                data[0][0] * other.data[0][1] + data[0][1] * other.data[1][1],
                data[1][0] * other.data[0][0] + data[1][1] * other.data[1][0],
                data[1][0] * other.data[0][1] + data[1][1] * other.data[1][1]
            );
        }
    };

    constexpr Matrix2D matrixPower(const Matrix2D& base, std::size_t exponent) {
        Matrix2D result;
        Matrix2D temp = base;
        std::size_t n = exponent;

        while (n > 0) {
            if (n % 2 == 1) {
                result = result * temp;
            }
            temp = temp * temp;
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

    constexpr std::size_t climbStairs(std::size_t n) {
        return fibonacci(n + 1);
    }
};

class Solution {
public:
    constexpr std::size_t climbStairs(std::size_t n) {
        if (n + 1 == 0) return 0;
        if (n + 1 == 1) return 1;
        std::size_t a = 0, b = 1, c;
        for (std::size_t i = 2; i <= n + 1; ++i) {
            c = a + b;
            a = b;
            b = c;
        }
        return b;
    }
};
// 不能爬到m，及m的倍数
class Solution {
public:
    constexpr int climbStairs(int n, int m) {
        if (n <= 0) return 0;
        if (n == 1) return 1;
        
        long long a = 1; // dp[0]
        long long b = (1 % m == 0) ? 0 : 1; // dp[1]
        
        if (n == 1) return b;
        
        for (int i = 2; i <= n; ++i) {
            long long c;
            // 如果当前台阶是 m 的倍数，则不能到达
            if (i % m == 0) {
                c = 0;
            } else {
                c = a + b;
            }
            a = b;
            b = c;
        }
        
        return b;
    }
    int climbStairs(int n, int m) {
        if (n <= 0) return 0;
        if (n == 1) return 1;
        
        // dp[i] 表示爬到第 i 阶的方法数
        vector<long long> dp(n + 1, 0);
        dp[0] = 1; // 起始位置
        
        for (int i = 1; i <= n; ++i) {
            // 如果当前台阶是 m 的倍数，则不能到达
            if (i % m == 0) {
                dp[i] = 0;
                continue;
            }
            
            // 从 i-1 阶爬 1 步上来
            if (i - 1 >= 0 && (i - 1) % m != 0) {
                dp[i] += dp[i - 1];
            }
            
            // 从 i-2 阶爬 2 步上来
            if (i - 2 >= 0 && (i - 2) % m != 0) {
                dp[i] += dp[i - 2];
            }
        }
        
        return dp[n];
    }
};
/*
 * @lc app=leetcode.cn id=70 lang=cpp
 *
 * [70] 爬楼梯
 */

// @lc code=start
// 使用COMBINATION宏来选择使用排列组合方法
// 使用DP宏来选择使用动态规划方法
// 使用DP_OPTIMIZED宏来选择使用空间优化的动态规划方法
// 使用MATRIX_POWER宏来选择使用矩阵快速幂方法
//#define COMBINATION
//#define DP
//#define DP_OPTIMIZED
#define MATRIX_POWER

#include <unordered_map>
#include <vector>
#include <array>
class Solution {
public:
    int climbStairs(int n) {
#ifdef COMBINATION
        // 排列组合解法
        int two_max_count = n / 2; // 可以有2的最大个数
        int result = 0;
        
        // 枚举使用的2的个数，从0到two_max_count
        for (int count_of_2 = 0; count_of_2 <= two_max_count; ++count_of_2) {
            int count_of_1 = n - 2 * count_of_2;  // 使用的1的个数
            int total_steps = count_of_1 + count_of_2;  // 总步数
            
            // 在total_steps个位置中选择count_of_2个位置放2
            result += _combination(total_steps, count_of_2);
        }
        
        return result;
#elif defined(DP)
        // 动态规划解法
        if (n <= 2) return n;
        
        // dp[i]表示爬到第i阶楼梯的方法数
        std::vector<int> dp(n + 1, 0);
        
        // 初始化基础情况
        dp[1] = 1; // 爬到第1阶有1种方法
        dp[2] = 2; // 爬到第2阶有2种方法：1+1或2
        
        // 状态转移
        for (int i = 3; i <= n; i++) {
            // 爬到第i阶的方法数 = 爬到第(i-1)阶的方法数 + 爬到第(i-2)阶的方法数
            // 因为可以从i-1爬1阶到达i，或者从i-2爬2阶到达i
            dp[i] = dp[i - 1] + dp[i - 2];
        }
        
        return dp[n];
#elif defined(DP_OPTIMIZED)
        // 空间优化的动态规划解法
        if (n <= 2) return n;
        
        // 只使用两个变量记录前两个状态
        int prev1 = 1; // 爬到第1阶有1种方法
        int prev2 = 2; // 爬到第2阶有2种方法：1+1或2
        int current = 0;
        
        // 状态转移
        // 从第三楼开始，只有两种上楼方式，从前一层再爬一楼和从前二层再爬两楼。
        // 可以推出 f(n) = f(n -1) + f(n -2)
        for (int i = 3; i <= n; i++) {
            // 当前状态等于前两个状态之和
            current = prev1 + prev2;
            // 更新状态，为下一次迭代做准备
            prev1 = prev2;
            prev2 = current;
        }
        
        return prev2;
#elif defined(MATRIX_POWER)
        // 矩阵快速幂解法
        if (n <= 2) return n;
        
        // 定义2x2矩阵及其运算
        using Matrix = std::array<std::array<long long, 2>, 2>;
        
        // 矩阵乘法
        auto multiply = [](const Matrix& a, const Matrix& b) -> Matrix {
            Matrix c = {{{0, 0}, {0, 0}}};
            for (int i = 0; i < 2; ++i) {
                for (int j = 0; j < 2; ++j) {
                    for (int k = 0; k < 2; ++k) {
                        c[i][j] += a[i][k] * b[k][j];
                    }
                }
            }
            return c;
        };
        
        // 矩阵快速幂
        auto matrix_pow = [&multiply](Matrix a, int n) -> Matrix {
            Matrix res = {{{1, 0}, {0, 1}}}; // 单位矩阵
            while (n > 0) {
                if (n & 1) { // 如果n是奇数
                    res = multiply(res, a);
                }
                a = multiply(a, a);
                n >>= 1; // n = n / 2
            }
            return res;
        };
        
        // 构建转移矩阵 [[1, 1], [1, 0]]
        // 该矩阵表示状态转移方程：f(n) = f(n-1) + f(n-2)
        Matrix base = {{{1, 1}, {1, 0}}};
        
        // 计算矩阵的(n-2)次幂
        Matrix result = matrix_pow(base, n - 2);
        
        // 计算第n个斐波那契数
        // [f(n), f(n-1)] = [[1, 1], [1, 0]]^(n-2) * [f(2), f(1)]
        // 其中f(2)=2, f(1)=1
        return static_cast<int>(result[0][0] * 2 + result[0][1] * 1);
#endif
    }
private:
#ifdef COMBINATION
    // 排列组合方法的辅助函数
    #define CACHE
    #ifdef CACHE
        std::unordered_map<int, std::vector<unsigned long long>> _cache;
    #endif
    unsigned long long _combination(int n, int k) {
    #ifdef CACHE
        if (k < 0 || k > n) return 0;
        k = std::min(k, n - k);  // 关键优化点：C(n,k) = C(n,n-k)

        // 检查缓存是否存在
        if (_cache.find(n) == _cache.end()) {
            // 为当前n预分配缓存空间（仅存k_max+1个元素）
            int k_max = n / 2;
            _cache[n] = std::vector<unsigned long long>(k_max + 1, 0);
        }

        auto& n_cache = _cache[n];
        
        // 直接返回已缓存结果
        if (k < n_cache.size() && n_cache[k] != 0) {
            return n_cache[k];
        }

        // 计算并缓存结果
        unsigned long long result = 1;
        for (int i = 0; i < k; ++i) {
            result *= (n - i);
            result /= (i + 1);
        }
        
        // 缓存计算结果
        if (k < n_cache.size()) {
            n_cache[k] = result;
        }

        return result;
    #else
        if (k < 0 || k > n) return 0;    // 处理非法输入
        k = std::min(k, n - k);          // 取较小值优化计算次数
        unsigned long long result = 1;
        for (int i = 0; i < k; ++i) {    // 只循环k次
            result *= (n - i);           // 分子部分：n * (n-1) * ... * (n-k+1)
            result /= (i + 1);           // 分母部分：1 * 2 * ... * k
        }
        return result;
    #endif
    }
#endif
};
// @lc code=end