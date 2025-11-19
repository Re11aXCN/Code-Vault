/*
 * @lc app=leetcode.cn id=1035 lang=cpp
 *
 * [1035] 不相交的线
 */

// @lc code=start
class Solution {
public:
    int maxUncrossedLines(vector<int>& nums1, vector<int>& nums2) {
        int nums1Len = nums1.size();
        int nums2Len = nums2.size();
        if(nums1Len > nums2Len) return maxUncrossedLines(nums2, nums1);

        std::vector<int> dp(nums2Len + 1, 0);

        for(int i = 1; i <= nums1Len; ++i) {
            int topLeft = dp[0];
            #pragma clang loop unroll_count(4)
            for(int j = 1; j <= nums2Len; ++j) {
                int top = dp[j];
                dp[j] = nums1[i - 1] == nums2[j - 1] 
                    ? topLeft + 1 
                    : std::max(dp[j - 1], top);
                topLeft = top;
            }
        }
        return dp[nums2Len];
    }
};
// @lc code=end

