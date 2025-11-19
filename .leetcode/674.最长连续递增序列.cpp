/*
 * @lc app=leetcode.cn id=674 lang=cpp
 *
 * [674] 最长连续递增序列
 */

// @lc code=start
class Solution {
public:
    int findLengthOfLCIS(vector<int>& nums) {
        int maxLen{ 1 }, currLen{ 1 };

        for(int i = 1; i < nums.size(); ++i) {
            if (nums[i] > nums[i - 1]) {
                ++currLen;
                if (currLen > maxLen) maxLen = currLen;
            }
            else {
                currLen = 1;
            }
        }
        return maxLen;
    }
};
// @lc code=end

