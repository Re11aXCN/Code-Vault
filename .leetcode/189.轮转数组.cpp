/*
 * @lc app=leetcode.cn id=189 lang=cpp
 *
 * [189] 轮转数组
 */

// @lc code=start
#include <vector>
using namespace std;
        // 1 2 3 4 5 6 7 换6
        // 6 2 3 4 5 1 7 换7
        // 6 7 3 4 5 1 2 换1
        // 6 7 1 4 5 3 2 换2
        // 6 7 1 2 5 3 4 换3
        // 6 7 1 2 3 5 4 换4
        // 6 7 1 2 3 4 5

// 错
        //  4 2 3 1 5 6 7
        //  4 5 3 1 2 6 7
        //  4 5 6 1 2 3 7
        //  4 5 6 7 2 3 1
        //  4 5 6 7 1 3 2
        //  4 5 6 7 1 2 3
// 错
        //  4 2 3 1 5 6 7 8
        //  4 5 3 1 2 6 7 8
        //  4 5 6 1 2 3 7 8
        //  4 5 6 7 2 3 1 8
        //  4 5 6 7 8 3 1 2
        //  4 5 6 7 8 2 1 3
        //  4 5 6 7 8 2 3 1
// 大于一半应该反向
        // 1 2 3 4 5 6 7 8
        // 1 2 8 4 5 6 7 3
        // 1 7 8 4 5 6 2 3
        // 6 7 8 4 5 1 2 3
        // 6 7 5 4 8 1 2 3
        // 6 4 5 7 8 1 2 3
        // 5 4 6 7 8 1 2 3
        // 4 5 6 7 8 1 2 3
        
        //  1 2 3 4 5 6 7 8 9 10 11
        //  6 2 3 4 5 1 7 8 9 10 11
        //  6 7 3 4 5 1 2 8 9 10 11
        //  6 7 8 4 5 1 2 3 9 10 11
        //  6 7 8 9 5 1 2 3 4 10 11
        //  6 7 8 9 10 1 2 3 4 5 11
        //  6 7 8 9 10 11 2 3 4 5 1
        //  6 7 8 9 10 11 1 3 4 5 2
        //  6 7 8 9 10 11 1 2 4 5 3
        //  6 7 8 9 10 11 1 2 3 5 4  // 第十次交换
        //  6 7 8 9 10 11 2 3 3 4 5m
        // 1 2 3 4 5 6 7 8
        // 1 2 8 4 5 6 7 3
        // 1 7 8 4 5 6 2 3
        // 6 7 8 4 5 1 2 3
        // 6 7 5 4 8 1 2 3
        // 6 4 5 7 8 1 2 3
        // 5 4 6 7 8 1 2 3
        // 4 5 6 7 8 1 2 3

        // 1 2 3 4 5 6 7 8
        // 6 2 3 4 5 1 7 8
        // 6 7 3 4 5 1 2 8
        // 6 7 8 4 5 1 2 3
        // 6 7 8 1 5 4 2 3
        // 6 7 8 1 2 4 5 3
        // 6 7 8 1 2 3 5 4
        // 6 7 8 1 2 3 4 5
        // 
        // 1 2 3 4 5 6 7 8
        // 7 2 3 4 5 6 1 8
        // 7 8 3 4 5 6 1 2
        // 7 8 1 4 5 6 3 2
        // 7 8 1 2 5 6 3 4
        // 7 8 1 2 3 6 5 4
        // 7 8 1 2 3 4 5 6



        // 类似冒泡排序，时间复杂度是On平方
       //k=3
        1 2 3 4 5 6 7 // 移动1，后向前
        1 2 3 7 5 6 4
        1 2 6 7 5 3 4

        1 5 6 7 2 3 4 // 移动2
        5 1 6 7 2 3 4
        5 6 1 7 2 3 4
        5 6 7 1 2 3 4

//k=1
        1 2 3 4 5 6 7 // 移动1
        1 2 3 4 5 7 6
        1 2 3 4 7 5 6
        ...
        7 1 2 3 4 5 6

        无 // 移动2
//k=2
        1 2 3 4 5 6 7
        1 2 3 4 7 6 5
        1 2 3 6 7 4 5
class Solution {
public:
    void rotate(vector<int>& nums, int k) {
        const size_t length = nums.size();
        if (length == 0 || (k %= length) == 0) return; // k是length倍数返回
#ifdef SPACE_O_N
        auto begin_it = nums.cbegin();
        vector<int> result;
        result.reserve(length);
        result.insert(result.cbegin(), begin_it + length - k, nums.cend());
        result.insert(result.cend(), begin_it, begin_it  + length - k);
        nums = move(result);
#else
        /* 
        // 这个逻辑进行交换位置，但是有些问题，
        // 循环逻辑错误：尝试手动控制交换位置时，未能正确覆盖所有需要移动的元素
        // 未考虑最大公约数：当 k 与 n 不互质时，需要分多个循环链处理
        //
        // 标准库实现的就是交换逻辑，直接使用标准库的算法
        if (k > (length / 2)) {
            int swap_start =  length - k - 1;
            for(int i = nums.size() - 1; i >= 1; --i) {;
                swap(nums[i], nums[swap_start--]);
                if (swap_start == -1) swap_start = length - k - 1;
            }
        }
        else {
            int swap_start =  length - k;
            for(int i = 0; i < nums.size() - 1; ++i) {
                swap(nums[i], nums[swap_start++]);
                if(swap_start == length) swap_start = length - k;
            }
        }
        */
        /*
        整体反转 → [7,6,5,4,3,2,1]
        反转前3个 → [5,6,7,4,3,2,1]
        反转后4个 → [5,6,7,1,2,3,4]
        */
        // 三次反转
        //reverse(nums.begin(), nums.end());          // 1. 整体反转
        //reverse(nums.begin(), nums.begin() + k);    // 2. 前k个反转
        //reverse(nums.begin() + k, nums.end());      // 3. 剩余部分反转

        // 向右旋转k步等价于将后k个元素移到前面
        std::rotate(
            nums.begin(),        // 起始位置
            nums.end() - k,      // 新的起始点（原数组倒数第k个元素）
            nums.end()           // 结束位置
        );
#endif
    }
};
// @lc code=end
int main() {
    Solution s;
    vector<int> nums = {1,2,3,4,5,6,7,8};
    s.rotate(nums, 3);
    return 0;
}

