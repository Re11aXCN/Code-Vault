/*
 * @lc app=leetcode.cn id=300 lang=cpp
 *
 * [300] 最长递增子序列
 */

// @lc code=start
#include <vector>
#include <algorithm>
using namespace std;

// 默认使用动态规划解法
#define GREEDY

class Solution {
public:
// 其实就是暴力算法优化版，两次for，统计每个数字的最长子序列长度，用一个数组存储起来，取最大值
    int lengthOfLIS(vector<int>& nums) {
#ifdef DP
        // 动态规划解法 - 时间复杂度O(n²)，空间复杂度O(n)
        int n = nums.size();
        if (n == 0) return 0;
        
        // dp[i]表示以nums[i]结尾的最长递增子序列的长度
        vector<int> dp(n, 1);
        int maxLen = 1;
        
        for (int i = 1; i < n; ++i) {
            //考虑从数组开始到当前位置的所有可能的子序列
            for (int j = 0; j < i; ++j) {
                //可以将 nums[i] 接在以 nums[j] 结尾的子序列后面，形成一个更长的递增子序列
                if (nums[i] > nums[j]) {
                    dp[i] = max(dp[i], dp[j] + 1);
                }
            }
            maxLen = max(maxLen, dp[i]);
        }
        
        return maxLen;
#elif defined GREEDY
        // 贪心+二分查找解法 - 时间复杂度O(n log n)，空间复杂度O(n)
        // tails[i]表示长度为i+1的递增子序列的最小结尾元素
        vector<int> tails;
        
        // 遍历每个元素
        for (int num : nums) {
            // 使用二分查找找到第一个大于等于num的位置
            auto it = lower_bound(tails.begin(), tails.end(), num);
            
            // 如果没有找到大于等于num的元素，说明num比所有tails元素都大
            // 将num添加到tails末尾，表示找到了一个更长的递增子序列
            if (it == tails.end()) {
                tails.push_back(num);
            } 
            // 否则，更新该位置的值为num
            // 这意味着我们找到了一个更小的结尾元素，可以形成相同长度的递增子序列
            else {
                *it = num;
            }
        }
        
        // tails的长度就是最长递增子序列的长度
        return tails.size();
#endif
    }
};
// @lc code=end

// 第二次写，错误
class Solution {
public:
// 10 9 2 8 3 4 5 9 101 18  
// [1,3,6,7,9,4,10,5,6] 不对 只能是 1 3 4 5 6正确是1 3 6 7 9 10
// 思路没错，就是采用一个窗口维持大小，利用二分查找逻辑进行查找
// 错误在于更新策略，两个判断条件都是连续的，然后因为pop_back更新问题
    int lengthOfLIS(vector<int>& nums) {
        std::vector<int> window;
        std::unordered_set<int> filter;
        window.push_back(nums.front());
        filter.insert(nums.front());
        for (auto curr = nums.begin() + 1; curr != nums.end(); ++curr) {
            if (*(curr - 1) > *curr) {
                if(filter.contains(*curr)) continue;
                auto find = std::lower_bound(window.begin(), window.end(), *curr);
                if (find == window.end()) continue;
                int dist = std::distance(find, window.end());
                while (dist > 0) {
                    window.pop_back();
                    --dist;
                }
                window.push_back(*curr);
            }
            else if (*(curr - 1) < *curr) {
                window.push_back(*curr);
                filter.insert(*curr);
            }
        }
        return window.size();
    }
};
int main(){
    Solution s;
    vector<int> nums = {10,9,2,5,3,7,101,18};
    auto res = s.lengthOfLIS(nums);
    return 0;
}