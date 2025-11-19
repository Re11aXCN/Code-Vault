/*
 * @lc app=leetcode.cn id=264 lang=cpp
 *
 * [264] 丑数 II
 */

// @lc code=start
class Solution {
    inline static constexpr unsigned CACHE_SIZE = 1690;
public:
    // 静态数组 + 16次循环展开 + 预计算优化
    int nthUglyNumber(int n) {
        if (n <= 0 || n > CACHE_SIZE) return 0;

        static /*thread_local*/ int ugly[CACHE_SIZE];
        ugly[0] = 1;
        int i2 = 0, i3 = 0, i5 = 0;

        int i = 1;
        for (; i + 15 < n; i += 16) {
            // 批量预加载值
            int val2 = ugly[i2], val3 = ugly[i3], val5 = ugly[i5];

            for (int j = 0; j < 16; j++) {
                int next2 = val2 * 2;
                int next3 = val3 * 3;
                int next5 = val5 * 5;

                // 手动内联min函数
                int min_val = (next2 < next3) ?
                    ((next2 < next5) ? next2 : next5) :
                    ((next3 < next5) ? next3 : next5);

                ugly[i + j] = min_val;

                // 条件更新
                if (min_val == next2) {
                    i2++;
                    val2 = ugly[i2];
                }
                if (min_val == next3) {
                    i3++;
                    val3 = ugly[i3];
                }
                if (min_val == next5) {
                    i5++;
                    val5 = ugly[i5];
                }
            }
        }

        // 处理剩余元素
        for (; i < n; i++) {
            int next2 = ugly[i2] * 2;
            int next3 = ugly[i3] * 3;
            int next5 = ugly[i5] * 5;

            int min_val = (next2 < next3) ?
                ((next2 < next5) ? next2 : next5) :
                ((next3 < next5) ? next3 : next5);

            ugly[i] = min_val;

            if (min_val == next2) i2++;
            if (min_val == next3) i3++;
            if (min_val == next5) i5++;
        }

        return ugly[n - 1];
    }
};
// @lc code=end

