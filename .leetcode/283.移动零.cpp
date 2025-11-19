/*
 * @lc app=leetcode.cn id=283 lang=cpp
 *
 * [283] 移动零
 */

 // @lc code=start
#include <vector>
using namespace std;
class Solution {
public:
     /*
      * 双指针，将所有0移动到末尾，不改变其他元素相对位置
      * 时间复杂度O(n)，空间复杂度O(1)
      */
    /*  步骤
     * 1、第一次循环，将所有非0的数前移，同时需要统计非0的个数，能够保证所有元素相对位置，且没有0
     * 2、第二次循环，根据非0的个数，作为循环起始位置，将剩余部分填充0
     * 3、复杂度：第一次O(n)，第二次O(0个数)
     */
    void moveZeroes(vector<int>& nums) {
        int index = 0;
        for (auto it = nums.begin(); it != nums.end(); ++it)
        {
            if(*it != 0) nums[index++] = *it; 
        }
        for(; index < nums.size(); ++index)
        {
            nums[index] = 0;
        }
    }
    void moveZeroes(std::vector<int>& nums) {
        int notZeroLength{ 0 };
        #pragma GCC unroll 4
        for(int num : nums)
        {
            if(num == 0) continue;
            nums[notZeroLength] = num;
            ++notZeroLength;
        }
        std::fill_n(nums.rbegin(), nums.size() - notZeroLength, 0);
    }
};
// @lc code=end
#ifdef _DEBUG
int main()
{
    Solution obj;
    vector<int> nums{ 0,1,0,3,12 };
    obj.moveZeroes(nums);
    return 0;
}
#endif
