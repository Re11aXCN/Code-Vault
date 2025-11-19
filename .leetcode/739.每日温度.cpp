/*
 * @lc app=leetcode.cn id=739 lang=cpp
 *
 * [739] 每日温度
 */

// @lc code=start

// 单调栈，始终维护一个温度递减的stk，stk存储的是索引
#include <stack>
#include <vector>
using namespace std;
class Solution {
public:
    std::vector<int> dailyTemperatures(std::vector<int>& temperatures) {
        int size = temperatures.size();
        std::vector<int> res(size, 0); // 最后一个始终是0，所以设置为0，而不是1
        std::stack<int> stk_idx; // 单调栈（存储索引，栈内温度递减）

        for(int i = 0; i < size; ++i)
        {
            #pragma GCC unroll 4
            // 当栈非空且当前温度大于栈顶温度时，计算天数差
            while (!stk_idx.empty() && temperatures[i] > temperatures[stk_idx.top()])
            {
                int prev_idx = stk_idx.top();
                res[prev_idx] = i - prev_idx; // 更新结果
                stk_idx.pop();
            }
            stk_idx.push(i);
        }
        return res;
    }
/*
    vector<int> dailyTemperatures(vector<int>& temperatures) {
        
        * 自己的思路
        //73 1
        //74 1
        //75 0    75 2
        //71 0    71 3
        //69 1    
        //72 1
        //76 0
        //73 0
        //0 0 1 1  
        stack<pair<int,int>> indices;
        vector<int> one_nums;
        one_nums.reserve(temperatures.size());
        for(int i = 0; i < temperatures.size() - 1; ++i){
            one_nums.push_back(temperatures[i] < temperatures[i + 1] ? 1 : 0);
            if(temperatures[i] >= temperatures[i + 1]) {
                indices.push({i, temperatures[i]});
            }
            else if (temperatures[i] < temperatures[i + 1]) {
                while(!indices.empty() && indices.top().second < temperatures[i + 1]) {
                    one_nums[indices.top().first] = i + 1 - indices.top().first;
                    indices.pop();
                }
            }
        }
        one_nums.push_back(0);
        return one_nums;
        
    }*/
};
// @lc code=end
int main(int argc, char const* argv[])
{
    Solution s;
    vector<int> temperatures{ 89,62,70,58,47,47,46,76,100,70 };
    auto res = s.dailyTemperatures(temperatures);
    return 0;
}

