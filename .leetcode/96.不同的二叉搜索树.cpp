#include <vector>
/*
 * @lc app=leetcode.cn id=96 lang=cpp
 *
 * [96] 不同的二叉搜索树
 */

// @lc code=start
class Solution {
public:
    std::size_t numTrees(std::size_t n) {
        std::vector<std::size_t> dp(n + 1);
        dp[0] = 1;
        
        for (std::size_t i = 1; i <= n; ++i) {
            for (std::size_t j = 0; j < i; ++j) {
                dp[i] += dp[j] * dp[i - 1 - j];
            }
        }
        
        return dp.back();
    }
};
// @lc code=end
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
inline constexpr auto [[nodiscard]] catalan(std::size_t n) -> std::size_t {
    if (n == 0) return 1;

    std::size_t result = 0;
    for (std::size_t i = 0; i < n; ++i) {
        result += catalan(i) * catalan(n - 1 - i);
    }
    return result;
}
