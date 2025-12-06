vector<vector<int>> merge(vector<vector<int>>& intervals) {
    std::sort(intervals.begin(), intervals.end(), [](const auto& a, const auto& b) { return a[0] < b[0]; });
    intervals.push_back({INT_MAX, INT_MAX});
    std::vector<std::vector<int>> result; result.reserve(intervals.size());
    
    for(int i = 1; i < intervals.size(); ++i) {
        if (intervals[i - 1][1] >= intervals[i][1]) {
            std::swap(intervals[i - 1], intervals[i]);
        }
        else if (intervals[i - 1][1] >= intervals[i][0]) {
            intervals[i][0] = intervals[i - 1][0];
        }
        else {
            result.emplace_back(intervals[i - 1].begin(), intervals[i - 1].end());
        }
    }
    return result;
}
/*
 * @lc app=leetcode.cn id=56 lang=cpp
 *
 * [56] 合并区间
 */

// @lc code=start
class Solution {
public:
    // [1,3],[2,6],[5,10]
    // [1,8],[2,6],[5,10]

    // [1,6],[1,8]
    // [1,8],[1,6]

    // [1,2],[3,4],[5,6],[7,8]
    // [1,2],[3,4],[5,6],[7,8],[9,10]

    vector<vector<int>> merge(vector<vector<int>>& intervals) {
        if (intervals.size() <= 1) return intervals;

        auto compare = [](const auto& v1, const auto& v2) { return v1.front() < v2.front(); };
        std::sort(intervals.begin(), intervals.end(), compare);

        vector<vector<int>> res;
        res.reserve(intervals.size());

        auto prev = intervals.begin(), next = intervals.begin() + 1;
        for (; next != intervals.end(); ++prev, ++next) {
            if ((*prev).back() < (*next).front()) {
                res.push_back(*prev);
            }
            else if ((*prev).back() > (*next).back()) {
                (*next).front() = (*prev).front();
                (*next).back() = (*prev).back();
            }
            else if ((*prev).back() >= (*next).front()) {
                (*next).front() = (*prev).front();
            }
        }
        res.push_back(*prev);

        return res;
    }
};


#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iostream>
using namespace std;
template<class K, class V>
using hashmap = unordered_map<K, V>;
class Solution {
public:
    // [3,3],[4,5],[1,3],[2,6],[8,10],[15,18],[1,6]
    // [1,6]
    // [2,6]
    // [3,3]
    // [4,5]
    // [8,10]
    // [15,18]
    vector<vector<int>> merge(vector<vector<int>>& intervals) {
        if (intervals.empty()) return {}; // 处理空输入

        // 按区间起始点升序排序（关键步骤）
        sort(intervals.begin(), intervals.end(), [](const vector<int>& a, const vector<int>& b) {
            return a[0] < b[0];
            });

        vector<vector<int>> merged; // 存储合并后的区间
        merged.push_back(intervals[0]); // 初始化第一个区间

        // 遍历所有区间
        for (const auto& interval : intervals) {
            vector<int>& last = merged.back(); // 当前合并后的最后一个区间
            if (interval[0] <= last[1]) { // 检查重叠
                // 合并区间，更新结束点为两者最大值
                last[1] = max(last[1], interval[1]);
            }
            else {
                // 无重叠，直接添加新区间
                merged.push_back(interval);
            }
        }

        return merged;
        /*
        * 我的思路：维护了hash表，然后排序，然后合并；和标准解法比较，多了hash表维护，开销高了
        * 标准解法：先排序，然后合并，
        const size_t intervals_length = intervals.size();
        hashmap<int, vector<int>> category_dict;
        category_dict.reserve(intervals_length);
        vector<int> range_start;
        range_start.reserve(intervals_length);
        for(const auto& range : intervals) {
            if(category_dict.contains(range[0])) {
                category_dict[range[0]][1] = max(category_dict[range[0]][1], range[1]);
            }
            else {
                range_start.push_back(range[0]);
                category_dict[range[0]] = range;
            }
        }
        sort(range_start.begin(), range_start.end(), less<>{});
        int left = 0, right = 1;
        for ( ; right < range_start.size(); ) {
            int range_start_left = range_start[left];
            int range_start_right = range_start[right];
            if (category_dict[range_start_left][1] >= category_dict[range_start_right][0]) {
                category_dict[range_start_left][1] = max(category_dict[range_start_left][1], category_dict[range_start_right][1]);
                category_dict.erase(range_start_right);
                ++right;
            }
            else {
                left = right;
                ++right;
            }
        }
        vector<vector<int>> result;
        result.reserve(category_dict.size());
        for(auto&& [k, v] : category_dict) {
            result.push_back(move(v));
        }
        return result;
        */
    }
};
// @lc code=end
int main()
{
    Solution s;
    vector<vector<int>> intervals = {{3,3},{4,5},{1,3},{2,6},{8,10},{15,18},{1,6}};
    auto result = s.merge(intervals);
    for(const auto& range : result) {
        cout << "[" << range[0] << "," << range[1] << "]" << endl;
    }
    return 0;
}
