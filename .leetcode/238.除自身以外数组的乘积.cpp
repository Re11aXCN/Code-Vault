    std::vector<int> productExceptSelf(std::vector<int>& nums) {
        /* 最慢
        std::vector<int> preMultiSum; preMultiSum.reserve(nums.size());
        preMultiSum.emplace_back(nums.front());
        #pragma clang loop vectorize(enable) unroll_count(8)
        for(int i = 1; i < nums.size(); ++i) {
            preMultiSum.emplace_back(nums[i] * preMultiSum[i - 1]); // 如果换为nums[i - 1]则位居其次，但是不符合题意，因为换了之后是计算相邻的，而不是累计的
        }

        // 是nums[i] * nums[i - 1]的标准库写法 如果计算相邻最快
        /* 计算的是相邻的乘积，而不是累计的乘积，不适用
        std::adjacent_difference(nums.begin(), nums.end(), 
                        preMultiSum.begin(), std::multiplies<>{});
        // 等价于
        preMultiSum.front() = nums.front();
        std::transform(std::next(nums.begin()), nums.end(),
            nums.begin(), std::next(preMultiSum.begin()),
            [](int a, int b) { return a * b; });
        */
        /* 其次
        std::vector<int> preMultiSum(nums.begin(), nums.end());
        #pragma GCC unroll 8
        for(int i = 1; i < preMultiSum.size(); ++i) {
            preMultiSum[i] *= preMultiSum[i - 1];
        }
        // 等价于
        std::vector<int> preMultiSum(nums.size());
        std::partial_sum(nums.begin(), nums.end(), preMultiSum.begin(), 
                std::multiplies<>{});
        */
        // 最快
        std::vector<int> preMultiSum; preMultiSum.reserve(nums.size());
        //std::partial_sum(nums.begin(), nums.end(), std::back_inserter(preMultiSum), std::multiplies<>{});
        // 等价于
        std::inclusive_scan(nums.begin(), nums.end(), std::back_inserter(preMultiSum), std::multiplies<>{});

        int sufMultiSum = 1;
        #pragma GCC unroll 8
        for(int i = nums.size() - 1; i >= 1; --i) {
            preMultiSum[i] = preMultiSum[i - 1] * sufMultiSum;
            sufMultiSum *= nums[i];
        }
        preMultiSum.front() = sufMultiSum;
        return preMultiSum;
    }

// 7ms
class Solution {
public:
    std::vector<int> productExceptSelf(std::vector<int>& nums) {
        int multi{ 1 }, zeroCount{ 0 };
        #pragma GCC unroll 4
        for(int num : nums)
        {
            if(num == 0) {
                ++zeroCount;
                if (zeroCount > 1) return std::vector(nums.size(), 0); 
            }
            else {
                multi *= num;
            }
        }
        std::vector<int> res;
        res.reserve(nums.size());
        if(zeroCount == 1) {
            #pragma GCC unroll 4
            for(int num : nums)
            {
                res.emplace_back(num == 0 ? multi : 0);
            }
        }
        else {
            #pragma GCC unroll 4
            for(int num : nums)
            {
                res.emplace_back(static_cast<int>(0.01 + std::exp(std::log(std::fabs(multi)) - std::log(std::fabs(num)))) * ((multi < 0) ^ (num < 0) ? -1 : 1));
            }
        }
        return res;
    }
};

/*
 * @lc app=leetcode.cn id=238 lang=cpp
 *
 * [238] 除自身以外数组的乘积
 */

// @lc code=start
#include <vector>
using namespace std;
class Solution {
public:
    int div32(int dividend, int divisor) {
        // 判断结果符号：异或后符号位为1则结果为负
        const bool is_negative = (dividend ^ divisor) < 0;

        // 转换为无符号绝对值，特殊处理INT_MIN避免溢出
        const unsigned int unsigned_dividend =
            (dividend == INT_MIN) ? (unsigned int)INT_MIN : abs(dividend);
        const unsigned int unsigned_divisor =
            (divisor == INT_MIN) ? (unsigned int)INT_MIN : abs(divisor);

        unsigned int quotient = 0;
        unsigned int remaining = unsigned_dividend;  // 剩余被除数

        // 从最高位开始尝试减法（覆盖32位有符号范围）
        for (int shift = 30; shift >= 0; shift--) {
            // 计算 divisor * 2^shift 并检查溢出
            const unsigned int shifted_value = unsigned_divisor << shift;
            if ((shifted_value >> shift) != unsigned_divisor) {
                continue;  // 左移后溢出，跳过该位
            }

            // 若当前位可减，则累加商并减少剩余被除数
            if (remaining >= shifted_value) {
                quotient += 1U << shift;  // 更新商
                remaining -= shifted_value;  // 更新剩余值
            }

            // 提前终止：已完全除尽
            if (remaining == 0) break;
        }

        // 应用符号并返回结果
        return is_negative ? -(int)quotient : (int)quotient;
    }
    vector<int> productExceptSelf(vector<int>& nums) {
#ifdef Subtraction
        vector<pair<int, bool>> result(61, { 0,false }); //注意不是{61, {0,false}}
        int product = 1;
        bool has_zero = false;
        for (int i = 0; i < nums.size(); ++i) {
            result[nums[i] + 30].first++;
            if (nums[i] != 0) product *= nums[i];
            else has_zero = true;
        }
        if (result[30].first > 1) product = 0;
        for (int i = 0; i < nums.size(); ++i) {
            if (has_zero) {
                if (nums[i] != 0) nums[i] = 0;
                else nums[i] = product;
            }
            else {
                if (result[nums[i] + 30].second) {
                    nums[i] = result[nums[i] + 30].first;
                }
                else {
                    result[nums[i] + 30].first = div32(product, nums[i]);
                    result[nums[i] + 30].second = true;
                    nums[i] = result[nums[i] + 30].first;
                }
            }
        }

        return nums;
#else
        int n = nums.size();
        vector<int> answer(n, 1);  // 结果数组，初始化为1

        // 第一步：计算前缀积（左侧乘积）
        int prefix = 1;
        #pragma GCC unroll 16
        for (int i = 0; i < n; ++i) {
            answer[i] = prefix;    // 存储当前位置的前缀积（不包含当前元素）
            prefix *= nums[i];     // 更新前缀积，包含当前元素供下一个位置使用
        }

        // 第二步：计算后缀积（右侧乘积）并与前缀积相乘
        int suffix = 1;
        #pragma GCC unroll 16
        for (int i = n - 1; i >= 0; --i) {
            answer[i] *= suffix;   // 前缀积 * 后缀积
            suffix *= nums[i];     // 更新后缀积，包含当前元素供前一个位置使用
        }

        return answer;
#endif // DEBUG
    }
};
// @lc code=end
int main() {
    
    Solution s;
    vector<int> nums = { 0,4,0 };
    s.productExceptSelf(nums);
    return 0;
}

