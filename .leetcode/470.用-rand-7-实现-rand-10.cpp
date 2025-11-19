/*
 * @lc app=leetcode.cn id=470 lang=cpp
 *
 * [470] 用 Rand7() 实现 Rand10()
 */
// 题解：https://leetcode.cn/problems/implement-rand10-using-rand7/solutions/427572/cong-pao-ying-bi-kai-shi-xun-xu-jian-jin-ba-zhe-da/
// @lc code=start
// The rand7() API is already defined for you.
// int rand7();
// @return a random integer in the range 1 to 7

class Solution {
public:
    int rand10() {
        while (true) {
            int x = (rand7() - 1) * 7 + (rand7() - 1); // 0~48
            if (x >= 1 && x <= 40) return x % 10 + 1;
            
            x = (x % 40) * 7 + rand7(); // 1~63
            if (x <= 60) return x % 10 + 1;

            x = (x - 61) * 7 + 7; // 1~21
            if (x <= 20) return x % 10 + 1;
        }
    }
};
// @lc code=end

