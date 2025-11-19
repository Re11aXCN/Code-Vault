/*
 * @lc app=leetcode.cn id=295 lang=cpp
 *
 * [295] 数据流的中位数
 */

// @lc code=start
#include <queue>
#include <array>
#include <ranges>
using namespace std;
class MedianFinder {
public:
    MedianFinder() {}
    
    void addNum(int num) {
        // 保证 max_heap 的大小 >= min_heap
        if (max_heap.empty() || num <= max_heap.top()) {
            max_heap.push(num);
        } else {
            min_heap.push(num);
        }
        
        // 平衡堆大小，确保 max_heap.size() >= min_heap.size() 且差值不超过1
        if (max_heap.size() > min_heap.size() + 1) {
            min_heap.push(max_heap.top());
            max_heap.pop();
        } else if (min_heap.size() > max_heap.size()) {
            max_heap.push(min_heap.top());
            min_heap.pop();
        }
    }
    
    double findMedian() {
        if (max_heap.size() > min_heap.size()) {
            return max_heap.top();
        } else {
            return (max_heap.top() + min_heap.top()) / 2.0;
        }
    }

private:
    priority_queue<int> max_heap; // 存放较小的一半（最大堆）
    priority_queue<int, vector<int>, greater<int>> min_heap; // 存放较大的一半（最小堆）
    /*
    O(nlogn)
public:
    MedianFinder() {
        
    }
    
    void addNum(int num) {
        _data.push_back(num);
        if(_data.size() % 2 == 1) median++;
    }
    
    double findMedian() {
        priority_queue<int, vector<int>, greater<int>> min_heap{_data.begin(), _data.end()};
        int count = -1; double median_num = 0;
        while (count != median)
        {
            median_num = min_heap.top();
            min_heap.pop();
            count++;
        }
        return _data.size() % 2 == 1 ? median_num : (median_num + min_heap.top()) / 2;
    }
private:
    int median = -1;
    vector<int> _data;
    */
};

/**
 * Your MedianFinder object will be instantiated and called as such:
 * MedianFinder* obj = new MedianFinder();
 * obj->addNum(num);
 * double param_2 = obj->findMedian();
 */
// @lc code=end
int main()
{
    MedianFinder obj;
    obj.addNum(6);
    auto r1 = obj.findMedian();
    obj.addNum(10);
    auto r2 = obj.findMedian();
    obj.addNum(2);
    auto r3 = obj.findMedian();
    obj.addNum(6);
    auto r4 = obj.findMedian();
    obj.addNum(5);
    auto r5 = obj.findMedian();
    obj.addNum(0);
    auto r6 = obj.findMedian();
    return 0;
}

