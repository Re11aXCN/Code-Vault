/*
 * @lc app=leetcode.cn id=169 lang=cpp
 *
 * [169] 多数元素
 */

// @lc code=start
class Solution {
public:
// 方法一：hashmap统计
// 方法二：排序返回一半的位置

// 关键信息是 超过一半以上的数字重复，抵消找数字
// 方法三：摩尔投票法，其核心思想是通过抵消不同的元素来找到出现次数超过一半的元素
/*
方法思路
    初始化候选元素和计数器：遍历数组时维护当前候选元素和其出现次数。
    抵消机制：
        当计数器为0时，选择当前元素作为新候选。
        遇到相同元素则计数器加1，不同元素则计数器减1。
    最终候选即为答案：由于题目保证存在多数元素，最终候选即为所求。
*/
    int majorityElement(vector<int>& nums) {
        int candidate{ 0 }, count{ 0 };

        #pragma GCC unroll 16
        for(int num : nums) {
            if(count == 0) candidate = num, count = 1;
            else if(candidate == num) ++count;
            else --count;
        }
        return candidate;
    }
};
// @lc code=end

