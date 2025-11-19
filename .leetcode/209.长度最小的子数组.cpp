/*
 * @lc app=leetcode.cn id=209 lang=cpp
 *
 * [209] 长度最小的子数组
 */

// @lc code=start
class Solution {
public:
    int minSubArrayLen(int target, vector<int>& nums) {
        int sum{0}, min_res{INT_MAX};
        for (int left = 0, right = 0; right < nums.size(); ++right) {
            //第一次进入for循环是空子串，不满足状态
            //right加入窗口
            sum += nums[right];
            while (sum >= target)
            {
                //根据情况收集答案
                //Left移出窗口
                if(auto curr_res = right - left + 1; min_res > curr_res) min_res = curr_res;
                sum -= nums[left++];
            }
            //不满足状态（根据情况收集答案）
        }
        return min_res == INT_MAX ? 0 : min_res;
    }
};
// @lc code=end

