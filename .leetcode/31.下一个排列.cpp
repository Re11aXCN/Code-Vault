    void nextPermutation(vector<int>& nums) {
        auto first = nums.begin();
        auto last = nums.end();
        auto back = last;
        if (first == last || first == --back) return;

        while(true) {
            auto back_next = back;
            if(*--back < *back_next) {
                auto prev = last;
                do { --prev; } while (*back >= *prev);

                swap(*back, *prev);
                reverse(back_next, last);
                return;
            }
            if (back == first) {
                reverse(first, last);
                return;
            }
        }
    }
/*
 * @lc app=leetcode.cn id=31 lang=cpp
 *
 * [31] 下一个排列
 */

// @lc code=start

/*
    全排列
    找下一个排列，按照比大小肯定是比当前大的一点的
    我们逆序查找降序的第一个值，如果都是降序反转，
    找到后，我们再逆序找到比这个值大的第一个元素交换位置，然后反转
*/
void nextPermutation(std::vector<int>& nums) {
        int length = nums.size() , i = length - 2;
        #pragma GCC unroll 8
        while(i > -1 && nums[i] >= nums[i + 1]) --i;

        if(i > -1){
            #pragma GCC unroll 8
            for(int j = length - 1; j > i; --j)
            {
                if(nums[j] > nums[i]) {
                    std::swap(nums[j], nums[i]);
                    break;
                }
            }
        }
        std::reverse(nums.begin() + i + 1, nums.end());
    }
class Solution {
public:
    void nextPermutation(vector<int>& nums) {
        // 5 3 7 1
        // i = 1
        // j = 2
        // 5 7 3 1
        // 5 7 1 3 reverse
        int n = nums.size();
        int i = n - 2;

        // 步骤1：从右向左寻找第一个递减的元素nums[i]
        while (i >= 0 && nums[i] >= nums[i + 1]) {
            --i;
        }

        if (i >= 0) { // 如果找到可交换的位置
            int j = n - 1;
            // 步骤2：在右侧寻找比nums[i]大的最小元素nums[j]
            while (j > i && nums[j] <= nums[i]) {
                --j;
            }
            // 步骤3：交换i和j位置的元素
            swap(nums[i], nums[j]);
        }

        // 步骤4：反转i+1后的元素，使其变为升序（最小化后续部分）
        reverse(nums.begin() + i + 1, nums.end());
    }
};
// @lc code=end

