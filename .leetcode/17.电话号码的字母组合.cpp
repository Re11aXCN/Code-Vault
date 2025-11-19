/*
 * @lc app=leetcode.cn id=17 lang=cpp
 *
 * [17] 电话号码的字母组合
 */

// @lc code=start
class Solution {
public:
    std::vector<std::string> letterCombinations(std::string digits) {
        // 如果输入为空，返回空结果
        if(digits.empty()) return {};
        
        // 建立数字到字母的映射关系
        static std::array<const char*, 10> phoneMap = {
            "_0", "_1", "abc", "def", "ghi", "jkl", "mno", "pqrs", "tuv", "wxyz"
        };

        std::vector<std::string> result; result.reserve(std::pow(3, digits.size()));
        std::string currSet; currSet.reserve(digits.size());

        // 调用回溯函数生成所有组合
        backtrack(phoneMap, digits, result, currSet, 0);
        return result;
    }

private:
    // 回溯函数
    void backtrack(const std::array<const char*, 10>& phoneMap, const std::string& digits,
        std::vector<std::string>& result, std::string& currSet, int start)
    {
        // 如果当前组合长度等于输入数字长度，说明找到了一个完整组合
        if (start == digits.size()) {
            result.push_back(currSet);
            return;
        }
        
        // 获取当前数字对应的所有可能字母
        std::string_view letters{ phoneMap[digits[start] - '0'] };
        
        // 尝试每一个可能的字母
        for (char letter : letters) {
            // 将当前字母加入组合
            currSet.push_back(letter);
            
            // 递归处理下一个数字
            backtrack(phoneMap, digits, result, currSet, start + 1);
            
            // 回溯，移除当前字母
            currSet.pop_back();
        }
    }
};
// @lc code=end

