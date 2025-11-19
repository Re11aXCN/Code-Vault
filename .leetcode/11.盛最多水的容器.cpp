class Solution {
public:
    int maxArea(std::vector<int>& height) {
        int left = 0, right = height.size() - 1;
        int max{0}, curr{0};
        for(; left < right && left < height.size(); )
        {
            if(int h = height[left]; h < height[right]) {
                curr = h * (right - left);
                if (max < curr) max = curr;
                ++left;
            }
            else {
                curr = height[right] * (right - left);
                if (max < curr) max = curr;
                --right;
            }
        }
        return max;
    }
};

/*
 * @lc app=leetcode.cn id=11 lang=cpp
 *
 * [11] 盛最多水的容器
 */

// @lc code=start
#include <vector>
#include <ranges>
#include <algorithm>
#include <iostream>
using namespace std;
class Solution {
public:
    int maxArea(vector<int>& height) {
        int max_area = 0;
#ifdef BRUTE_FORCE
        // 取决于最小的高度，去找最远比他大的
        int length = height.size() - 1;
        int head = height.front();
        int tail = height.back();
        int max_area = max(
            // 反向查找第一个大于首元素的元素
            static_cast<int>(length -
                distance(height.rbegin(), ranges::find_if(
                    height.rbegin(),
                    height.rend(),
                    [&head](int h) { return h >= head; })
                )
            ) * head,
            // 正向查找第一个大于尾元素的元素
            static_cast<int>(length - 
                distance(height.begin(), ranges::find_if(
                    height.begin(),
                    height.end(),
                    [&tail](int h) { return h >= tail; })
                )
            ) * tail
        );
        
        for(int i = 1; i <= length; ++i)
        {
            max_area = max(max_area, max(
                static_cast<int>(abs(
                    distance(height.rbegin() + length - i, ranges::find_if(
                        height.rbegin(),
                        height.rbegin() + length - i,
                        [&](int h) { return h >= height[i]; })
                    ))
                ) * height[i],
                static_cast<int>(abs(
                    distance(height.begin() + i, ranges::find_if(
                        height.begin(),
                        height.begin() + i,
                        [&](int h) { return h >= height[i]; })
                    ))
                ) * height[i]
            ));
        }
#else
        /// <describe>
        /// 双指针
        /// 给定一个长度为 n 的整数数组 height 。有 n 条垂线，第 i 条线的两个端点是 (i, 0) 和 (i, height[i]) 。
        /// 找出其中的两条线，使得它们与 x 轴共同构成的容器可以容纳最多的水。
        /// 返回容器可以储存的最大水量。
        /// 说明：你不能倾斜容器。
        /// </describe>
        /*
        * 思路：
        * 容器盛水量取决于最小一侧的高度，木桶效应
        * 所有循环计算要以最小的一侧来，为什么
        * 举例：假设首个0号柱子，末尾7号柱子，0号柱子高度<7号柱子高度。水面积取决于0号柱子高度
          如果你进行移动7号柱子到6号柱子，那么计算0-6的面积会小于当前计算0到7的面积
          但是0号柱子移动到1号柱子，因为不确定性，1号柱子是否比0号柱子高，要移动，计算比较面积
          假设1号柱子大于7号柱子，那么同样的你移动1号柱子，到2号柱子，3、4.。。计算得到的面积小于1和7号柱子的面积
          也就是说为什么排除了（4，7）柱子的面积因为已经小于了（1，7）的面积，始终注意取决于最小一侧和最远的比它高的柱子
          （1，7）内涵盖了了（4，7）
          认证体会一下这个逻辑
        */
        /*
        * 步骤：
        * 1、left首right末，循环left<right
        * 2、计算最小的一侧高度
        * 3、计算指针差值根据高度计算面积统计最大面积
        * 4、每次移动最矮的一侧指针，如果相等移动左侧
        */
        int left = 0;                   // 左指针
        int right = height.size() - 1;  // 右指针
        while (left < right) {
            // 计算当前容量并更新最大值
            const int current_height = min(height[left], height[right]);
            max_area = max(max_area, (right - left) * current_height);

            // 移动较矮的一侧指针
            height[left] <= height[right] ? ++left : --right;
        }
#endif
        return max_area;
    }
};
// @lc code=end
#ifdef _DEBUG
int main()
{
    Solution obj;
    vector<int> height{ 8,7,2,1 };
    obj.maxArea(height);
    return 0;
}
#endif
