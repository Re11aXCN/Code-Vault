    int firstMissingPositive(std::vector<int>& nums) {
        #pragma clang loop unroll_count(8)
        for(int num : nums) {
            while(num > 0 && num <= nums.size() && nums[num - 1] != num) {
                std::swap(num, nums[num - 1]);
            }
        }
        #pragma clang loop unroll_count(8)
        for(int i = 0; i < nums.size(); ++i) {
            if (int mapIdx = i + 1; nums[i] != mapIdx) return mapIdx;
        }
        return nums.size() + 1;
    }
/*
 * @lc app=leetcode.cn id=41 lang=cpp
 *
 * [41] 缺失的第一个正数
 */

// @lc code=start
#include <vector>
using namespace std;
class Solution {
public:
// 3 4 -1 1
// -1 4 3 1
// -1 1 3 4
// 1 -1 3 4
    /// <description>
    /// 基于 原地哈希（In-place Hashing） 的核心思想，
    /// 目标是利用数组本身的空间作为哈希表，将每个正整数 x 放置在索引 x-1 的位置
    /// 为什么这样设计
    ///   映射关系：如果数组中包含 1 到 n 的所有正整数，则遍历后每个位置 i 的值应为 i + 1。
    ///   缺失值：如果某个位置 i 的值不是 i + 1，则 i + 1 就是缺失的最小正数。 
    ///   边界情况：如果所有位置都正确，说明缺失的是 n + 1。
    /// </description>
    int firstMissingPositive(vector<int>& nums) {
        int length = nums.size();

        // 第一次遍历：将每个正整数放到正确的位置
        for (int i = 0; i < length; ++i) {
            // 持续交换直到当前元素无法放置（负数、零或超出范围）
            int& curr = nums[i];
            while (curr > 0 && curr <= length && nums[curr - 1] != curr) {
                std::swap(curr, nums[curr - 1]);
            }
        }

        // 第二次遍历：寻找第一个位置不匹配的正整数
        #pragma GCC unroll 4
        for (int i = 0; i < length; ++i) {
            if (nums[i] != i + 1) {
                return i + 1;
            }
        }

        // 如果所有位置都正确，返回 n+1
        return length + 1;
    }
};
// @lc code=end
        /*
        * 代码未实现，只给思路，但是这个思路可能考虑问题不全面
        * 只记录正数
        // 思想是一次遍历nums，记录一个连续的①range范围，出现数字比left小，就更新范围为left-1到right+1
        // 出现数字比①right大，再记录一个新的②range范围
        // 如果再次出现right比①大，且比②left小，更新②
        // 如果再次出现left比①小，更新②为①后，更新①为left-1到right+1
        // 最后比较①和②，
        // ①left不是0，缺失1；
        // ①left是0，比较①的right和②的left；
        //   如果②的left等于①的right，缺失②的right；
        //   如果②的left小于①的right，缺失①的right；
        int range_left = 0, range_right = 0, first_missing_v = INT_MAX;
        bool init_range = false;
        int part_left = 0, part_right = 0;
        for (int i = 0; i < nums.size(); ++i) {
            if (nums[i] <= 0) continue;
            if (nums[i] > range_left && nums[i] < range_right) continue;
            if (!init_range) {
                first_missing_v = nums[i] - 1;
                init_range = true;
                range_left = nums[i] - 1;
                range_right = nums[i] == INT_MAX ? nums[i] - 1 : nums[i] + 1;
            }
            else {
                if (nums[i] == range_left) {
                    --range_left;
                }
                else if (nums[i] == range_right) {
                    ++range_right;
                }
                else if (nums[i] < range_left) {
                    range_left = nums[i] - 1;
                    range_right = nums[i] + 1;
                    //range_left = nums[i] - 2;
                }
                else if (nums[i] > range_right) {
                    first_missing_v = range_right;
                    if (part_left < nums[i]) {
                        part_left = nums[i] - 1;
                        part_right = nums[i] + 1;
                    }
                }
            }

        }
        if (first_missing_v == 0) first_missing_v = range_right;
        else if (range_left == first_missing_v) first_missing_v = 1;
        else if (range_left <= first_missing_v && first_missing_v <= range_right) {
            if (range_left == 0) {
                if(part_left != 0)
                    return range_right == part_left ? range_right : part_right;
                else
                    return range_right;
            }
            else {
                return 1;
            }
        }

        return init_range ? first_missing_v : 1;
        */
int main()
{
    Solution s;
    //7, 9, 5, 1, 3, 0
    vector<int> v{ 100,5,-1,6 };
    int r = s.firstMissingPositive(v);
    return 0;
}
