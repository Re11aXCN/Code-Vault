int findDuplicate(vector<int>& nums) {
    int size = nums.size();
    
    for (int i = 0; i < size; ++i) {
        // 如果当前位置的数字不在正确的位置
        while (nums[i] != i + 1) {
            int correct_index = nums[i] - 1;
            
            // 如果目标位置已经放着正确的数字，说明找到了重复
            if (nums[i] == nums[correct_index]) {
                return nums[i];
            }
            
            // 否则交换到正确位置
            std::swap(nums[i], nums[correct_index]);
        }
    }
    
    return -1;  // 理论上不会执行到这里
}
/*
 * @lc app=leetcode.cn id=287 lang=cpp
 *
 * [287] 寻找重复数
 */

// @lc code=start
// 计数排序，时间 2O(N) 空间O(N)

// 快慢指针
/*
映射，数组下标是 节点 
     数组的值是 next
// 必须从下标0开始，从其他位置可能无法找到答案
     3 1 3 4 2
     0 1 2 3 4

     3->4->2->3

     1 3 4 2 2
     0 1 2 3 4
     1->3->2->4->2
*/

class Solution {
public:
    int findDuplicate(std::vector<int>& nums) {
        int slow{ 0 }, fast{ 0 };

        // 判断环是否存在
        #pragma GCC unroll 16
        do {
            slow = nums[slow];
            fast = nums[nums[fast]];
        } while(slow != fast);

        slow = 0;
        // 找形成环的位置，即重复元素
        #pragma GCC unroll 16
        while(slow != fast)
        {
            slow = nums[slow];
            fast = nums[fast];
        }
        return slow;
    }
};
// @lc code=end

