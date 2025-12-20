/*
 * @lc app=leetcode.cn id=394 lang=cpp
 *
 * [394] 字符串解码
 */

// @lc code=start
#include <stack>
#include <ranges>
class Solution {
public:
    std::string decodeString(std::string s) {
        struct DecodeStringData {
            int bra_str_repeat;
            std::string bra_prev_str;
        };
        std::stack<DecodeStringData> stk; // 存储(重复次数, 前序字符串)
        std::string curr_str; curr_str.reserve(16);
        int curr_num{ 0 };
        #pragma GCC unroll 4
        for(char c : s)
        {
            if(int digit = c - '0'; digit > -1 && digit < 10) {
                curr_num = curr_num * 10 + digit; // 处理多位数
            }
            else if (c == '[') {
                // 压栈并重置状态
                stk.push({curr_num, curr_str});
                curr_num = 0;
                curr_str.clear();
            }
            else if (c == ']') {
                // auto & dec_str_data = stk.top();
                // int repeat = dec_str_data.bra_str_repeat;

                // // 生成重复字符串并拼接前序字符串
                // std::string repeat_str; repeat_str.reserve(repeat * curr_str.size());
                // #pragma GCC unroll 4
                // for(int i = 0; i < repeat; ++i) repeat_str += curr_str;
                // curr_str = std::move(dec_str_data.bra_prev_str + repeat_str);

                // stk.pop();
                auto [repeat, prev_str] = std::move(stk.back());
                stk.pop_back();

                std::string repeat_str; 
                repeat_str.reserve(prev_str.size() + repeat * curr_str.size());
                repeat_str += prev_str;
                #pragma GCC unroll 4
                for(int i = 0; i < repeat; ++i) repeat_str += curr_str;
                curr_str = std::move(repeat_str);
            }
            else {
                curr_str.push_back(c); // 收集当前层字符
            }
        }

        return curr_str;
    }
};

// @lc code=end
