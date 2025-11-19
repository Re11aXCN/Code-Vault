/*
 * @lc app=leetcode.cn id=45 lang=cpp
 *
 * [45] 跳跃游戏 II
 */

// @lc code=start
class Solution {
public:
//[7,0,9,6,9,6,1,7,9,0,1,2,9,0,3]
//边界（含）内一定只跳一次，不管你max_reach是否已经大于了size-1，
//到达边界的时候再进行更新current_end为max_reach，
//如果超出了size-1，说明找到最小跳跃次数，
//否则继续跳跃重复边界范围内只会跳1次
    int jump(vector<int>& nums) {
        int jumps = 0;          // 跳跃次数
        int current_end = 0;    // 当前跳跃能到达的最远位置
        int max_reach = 0;      // 全局能到达的最远位置
        
        // 遍历到倒数第二个元素即可（避免多跳一次）
        for (int i = 0; i < nums.size() - 1; ++i) {
            max_reach = max(max_reach, i + nums[i]);  // 更新全局最远位置
            
            // 当遍历到当前跳跃的边界时，必须进行一次跳跃
            if (i == current_end) {
                jumps++;                     // 增加跳跃次数
                current_end = max_reach;     // 更新跳跃边界
                if (current_end >= nums.size() - 1) break; // 提前终止
            }
        }
        return jumps;
    }
};
// @lc code=end

