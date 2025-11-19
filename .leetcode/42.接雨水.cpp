/*
 * @lc app=leetcode.cn id=42 lang=cpp
 *
 * [42] 接雨水
 */
#include <stack>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
// @lc code=start
class Solution {
public:
    int trap(vector<int>& height) {
        if (height.size() < 3) return 0;

        int blackRectAreaSum{ 0 }, whiteRectAreaSum{ 0 }, maxHeight{ 0 };
        auto left = height.begin(), right = height.end() - 1, maxHeightPos = height.begin();
        for (; left < right; ++left, --right)
        {
            if (*left > maxHeight && *left >= *right) {
                maxHeight = *left;
                maxHeightPos = left;
            }
            else if (*right > maxHeight && *right > *left) {
                maxHeight = *right;
                maxHeightPos = right;
            }
            blackRectAreaSum += *left + *right;
        }
        if (left == right) { // height.size() & 1
            if (*left > maxHeight) maxHeight = *left, maxHeightPos = left;
            blackRectAreaSum += *left;
        }

        left = height.begin(), right = height.end() - 1;
        int currVal = *left;
        while (left <= maxHeightPos)
        {
            if (currVal < *left)
            {
                // 高 * 低
                whiteRectAreaSum += (*left - currVal) * std::distance(height.begin(), left);
                // 更新当前 黑色矩形 进入下一轮计算白色矩形和循环
                currVal = *left;
            }
            ++left;
        }
        currVal = *right;
        while (right >= maxHeightPos)
        {
            if (currVal < *right)
            {
                whiteRectAreaSum += (*right - currVal) * std::distance(right, height.end() - 1);
                currVal = *right;
            }
            --right;
        }
        // [0,2,0] 总 3 * 2 - 黑2 - 白2*2
        return static_cast<int>(height.size()) * maxHeight - blackRectAreaSum - whiteRectAreaSum;
    }
};
 // @lc code=end