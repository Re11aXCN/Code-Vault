/*
 * @lc app=leetcode.cn id=20 lang=cpp
 *
 * [20] 有效的括号
 */

// @lc code=start
#include <stack>
using namespace std;
class Solution {
public:
    bool isValid(string s) {
        /*
        if (char top = brackets.top(); 
                top == c - 1 || top == c - 2) 
            {
                brackets.pop();
        }
        */
        /*
        stack<char, vector<char>> brackets;
        for(char c : s) {
            if(brackets.empty() || c == '(' || c == '{' || c == '[') {
                brackets.emplace(c);
                continue;
            }
            char top = brackets.top();
            if (c == ')' && top == '(') {
                brackets.pop();
            }
            else if (c == '}' && top == '{') {
                brackets.pop();
            }
            else if (c == ']' && top == '['){
                brackets.pop();
            }
            else {
                return false;
            }
        }
        return brackets.empty();
        */
        stack<char> brace;
        for (const char& c : s) {
            if (!brace.empty() && (brace.top() == c - 1 || brace.top() == c - 2))
                brace.pop();
            else brace.push(c);
        }
        return !brace.size();
    }
};
// @lc code=end
int main(){
    Solution s;
    string str = "()[]{}}";
    s.isValid(str);
    return 0;
}
