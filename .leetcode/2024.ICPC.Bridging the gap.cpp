/*
 * @lc app=leetcode.cn id=994 lang=cpp
 *
 * [2024.ICPC] Bridging the gap
 */

// @lc code=start
#include <vector>
#include <algorithm>
using namespace std;

class Solution {
public:
    int minTime(vector<int>& people, int limit) {
        int length = people.size();
        if(limit >= length) return people[length - 1];
        sort(people.begin(), people.end());
        int fast = people[0];
        if(limit > 2) {
            int time = 0;
            int move = limit - 1;
            for(int i = length - 1; i > 0; i -= move){
                time += people[i] + (i > move ? fast : 0);
            }
            return time;
        }
        else {
            int second = people[1];
            int start = fast + second;
            int time = start;
            for (int i = length - 1; i > 1; i -= limit) {
                time += people[i] + (i > limit ? second + start : 0);
            }
            return length % 2 == 0 ? time - fast : time;
        }
    }
};
// @lc code=end
int main()
{
    Solution s;
    vector<int> people{ 2,4,7,9,12,16,20 };
    auto res = s.minTime(people, 3);
    return 0;
}

