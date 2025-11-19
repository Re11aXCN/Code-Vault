/*
第一段：处理左边界字符移除
cpp

// 处理左边界字符移除
if (count[left_char] == 0) ++diff;  // 你要移出去相等的字符，得++差异
++count[left_char];
if (count[left_char] == 0) --diff;

第一行： if (count[left_char] == 0) ++diff;

    含义：在移除字符之前检查当前状态

    如果 count[left_char] == 0，说明这个字符当前是"平衡状态"（p和s中这个字符的数量相等）

    当我们移除这个字符后，平衡会被打破，所以差异计数 diff 要增加

第二行： ++count[left_char];

    含义：实际执行字符移除操作

    因为我们的计数逻辑是：p中的字符+1，s中的字符-1

    从窗口中移除s的一个字符，相当于减少s的计数，所以要做 +1 操作

第三行： if (count[left_char] == 0) --diff;

    含义：移除后检查是否达到了新的平衡

    如果移除后 count[left_char] == 0，说明这个字符又回到了平衡状态

    所以差异计数 diff 要减少

第二段：处理右边界字符添加
cpp

// 处理右边界字符添加
if (count[right_char] == 0) ++diff;
--count[right_char];
if (count[right_char] == 0) --diff;

第四行： if (count[right_char] == 0) ++diff;

    含义：在添加字符之前检查当前状态

    如果 count[right_char] == 0，说明这个字符当前是平衡状态

    当我们添加这个字符后，平衡会被打破，所以差异计数 diff 要增加

第五行： --count[right_char];

    含义：实际执行字符添加操作

    向窗口中添加s的一个字符，相当于增加s的计数，所以要做 -1 操作

第六行： if (count[right_char] == 0) --diff;

    含义：添加后检查是否达到了新的平衡

    如果添加后 count[right_char] == 0，说明这个字符又回到了平衡状态

    所以差异计数 diff 要减少

直观理解

可以这样理解：

    count[char] = 0：这个字符在p和当前窗口中数量相等（平衡）

    count[char] ≠ 0：这个字符在p和当前窗口中数量不相等（不平衡）

    diff：记录有多少个字符处于不平衡状态

操作逻辑：

    在改变状态前，如果当前是平衡的，改变后就会不平衡 → diff++

    执行改变操作

    改变后，如果达到了新的平衡 → diff--
*/
class Solution {
public:
    std::vector<int> findAnagrams(std::string s, std::string p) {
        const size_t main_str_length = s.size();
        const size_t pattern_str_count = p.size();
        if (main_str_length < pattern_str_count)  return {};
        std::vector<int> res;
        res.reserve(8);

        //count 数组的真正含义是：p的字符频率 - 当前窗口的字符频率
        std::array<int, 26> count{};
        #pragma GCC unroll 8
        for(int i = 0; i < pattern_str_count; ++i)
        {
            ++count[p[i] - 'a'];// p中的字符：增加计数
            --count[s[i] - 'a'];// 窗口中的字符：减少计数
        }
        /*
        如果某个字符在p和窗口中频率相同 → count[char] = 0
        如果p中多 → count[char] > 0
        如果窗口中多 → count[char] < 0
        */

        int diff = 0;
        #pragma GCC unroll 4
        for (int c : count) if (c != 0) ++diff;

        if (diff == 0) res.push_back(0);

        #pragma GCC unroll 4
        for (int i = pattern_str_count; i < main_str_length; ++i) {
            char left_char = s[i - pattern_str_count] - 'a';
            char right_char = s[i] - 'a';
            /*
            为什么移除字符要 ++count[left_char]
            直觉上：移除字符应该减少计数，但这里 count 记录的是 差值！
            逻辑推导：
                移除字符 left_char 从窗口中
                窗口频率减少 → p相对于窗口的频率差增加
                所以 count[left_char]++（差值增加）
            */
            // 处理左边界字符移除
            // 移除前移除后不会同时发生
            if (count[left_char] == 0) ++diff; // 移除之前，如果是平衡状态说明字符相等，你要移除自然需要 ++差值
            ++count[left_char]; // 移除，回补 p的计数
            if (count[left_char] == 0) --diff; // 移除之后，如果是平衡状态，说明差异减少了，--1
            
            // 处理右边界字符添加
            if (count[right_char] == 0) ++diff;
            --count[right_char]; // p相对于窗口的频率差减少
            if (count[right_char] == 0) --diff;
            
            if (diff == 0) res.push_back(i - pattern_str_count + 1);
        }
        return res;
    }
};

/*
 * @lc app=leetcode.cn id=438 lang=cpp
 *
 * [438] 找到字符串中所有字母异位词
 */

// @lc code=start
#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <algorithm>
#include <ranges>
using namespace std;
class Solution {
public:
    // 仅含小写字母
    vector<int> findAnagrams(string s, string p) {
#ifdef SORT_OR_MARK
        constexpr size_t SORT_THRESHOLD = 15;
        auto generateCountKey = [](const string& s)
            -> string {
            array<uint8_t, 26> counts{}; // 26个小写字母的计数
            for (unsigned char c : s) {  // 处理带符号char的溢出
                ++counts[c - 'a'];
            }
            // 将计数转换为类似 "2#0#3#..." 的唯一字符串
            string key;
            key.reserve(26 * 2); // 预分配空间优化
            for (auto cnt : counts) {
                key += to_string(cnt) + '#'; // #分隔避免不同计数组合碰撞
            }
            return key;
        };
        const size_t main_str_length = s.size();
        const size_t pattern_str_count = p.size();
        if (main_str_length < pattern_str_count)  return {};
        const size_t pattern_count = main_str_length + 1 -pattern_str_count;
        vector<int> anagram_start_i_arr;
        anagram_start_i_arr.reserve(pattern_count);
        auto begin_iter = s.begin();
        string pattern_key;
        if (pattern_str_count <= SORT_THRESHOLD)
        {
            pattern_key = move(p);
            ranges::sort(pattern_key);
            for (int idx = 0; idx < pattern_count; ++idx, ++begin_iter) {
                string sub_str{ begin_iter, begin_iter + pattern_str_count };
                ranges::sort(sub_str);
                if (pattern_key == sub_str) {
                    anagram_start_i_arr.push_back(idx);
                }
            }
        }
        else
        {
            pattern_key = move(generateCountKey(p));
            for (int idx = 0; idx < pattern_count; ++idx, ++begin_iter) {
                if (pattern_key == generateCountKey({ begin_iter, begin_iter + pattern_str_count })) {
                    anagram_start_i_arr.push_back(idx);
                }
            }
        }
       
        return anagram_start_i_arr;
#else
    /*
     * 窗口滑动，如何找到主串所有字串和模式串是anagram，并返回字串的起始位置
     * 采用滑动窗口法，窗口大小为字串长度，窗口滑动，窗口内字符计数，窗口计数与模式串计数比较，相等则记录下标
     * 时间复杂度O(n)，空间复杂度O(1)
     * cbaebabacd
     * abc（cba）
     *  abc（bae）
     * 如何设置滑动窗口——>统计法，
     * 仅26个英文小写字母，采用array<int, 26>统计
     * abc是 1 1 1 000... abca是2 1 1 000... aabc是 2 1 1 000...完全没问题
     * 通过array比较就能够得到下标
     */
    /*  步骤
     * 1、判断主串长度是否小于字串特殊情况
     * 2、初始化首个滑动窗口，初始化模式串窗口
     * 3、判断两个窗口是否相等，决定是否插入首个窗口，0下标
     * 4、循环 处理 下一个窗口，左侧--收缩，右侧++扩大，通过比较两个窗口是否相等，决定插入下表
     */
    const size_t main_str_length = s.size();
    const size_t pattern_str_count = p.size();
    if (main_str_length < pattern_str_count)  return {};

    array<int, 26> pattern_count{};
    array<int, 26> window{};
    vector<int> result;
    
    for (auto&& [i, c] : p | views::enumerate) {
        ++pattern_count[c - 'a'];   // 初始化模式串计数
        ++window[s[i] - 'a'];   // 初始化窗口计数
    }

    if (window == pattern_count) 
        result.push_back(0);

    // 滑动窗口
    for (int i = pattern_str_count; i < main_str_length; ++i) {
        --window[s[i - pattern_str_count] - 'a']; // 移出左边界字符
        ++window[s[i] - 'a'];      // 移入右边界字符
        if (window == pattern_count) 
            result.push_back(i - pattern_str_count + 1);
    }

    return result;
#endif
    }
};
// @lc code=end
#ifdef _DEBUG
int main()
{
    // a 2 5 7
    // b 1 4 6 9
    // c 0 8 3
    // e 10

    // abab
    // a 0 2
    // b 1 3
    Solution obj;
    obj.findAnagrams("cbaebabacd", "abc");
    return 0;
}
#endif
