class Solution {
public:
    // 把每一个下标 的 单元格 都看作一个木桶，下标的值代表木桶底部厚度，木桶有左右边界，是否能够装水取决于左右边界是否大于木桶底部厚度
    // 双指针遍历左右，记录左右最大高度，并且根据当前的左右高度进行计算，如果当前左高度小于右高度，说明木桶容量取决于左边最大，反之
    int trap(vector<int>& height) {
        int result{ 0 }, left{ 0 }, right( height.size() - 1 );
        int leftHeightMax{ 0 }, rightHeightMax{ 0 };
        
        while(left < right) {
            int currLeftHeight = height[left], currRightHeight = height[right];

            if (leftHeightMax < currLeftHeight) leftHeightMax = currLeftHeight;
            if (rightHeightMax < currRightHeight) rightHeightMax = currRightHeight;
            // 为什么能够取决于左边最大，难道左边最大如果比currRightHeight大呢，不就取决于currRightHeight的高度了吗
            // 因为你要知道，我们是从最左 和 最右 向中间移动的，如果 leftHeightMax > currRightHeight，
            // 说明 必然 先出现 currLeftHeight > currRightHeight 情况 处理完成之后才会出现currLeftHeight < currRightHeight
            // 类似 盛最多水的容器 的思路，所以始终能够计算当前下标的木桶所容纳多少水
            if (currLeftHeight < currRightHeight) {
                result += leftHeightMax - currLeftHeight;
                ++left;
            }
            else {
                result += rightHeightMax - currRightHeight;
                --right;
            }
        }
        return result;
    }
};
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