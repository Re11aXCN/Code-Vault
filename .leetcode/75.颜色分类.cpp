// 循环依赖一层覆盖一层
// 都可以填充2
// 是0，从下标0开始填充0
// 1的位置依赖于 0的个数，v < 2，此时是统计1个数，因为我们确保0能够覆盖它
class Solution {
public:
    void sortColors(std::vector<int>& nums) {
        int zeroCount{ 0 }, oneCount{ 0 };

        #pragma GCC unroll 16
        for(int i = 0; i < nums.size(); ++i)
        {
            int v = nums[i];
            nums[i] = 2;
            if(v < 2) nums[oneCount++] = 1;
            if(v == 0) nums[zeroCount++] = 0;
        }
    }
};

class Solution {
public:
    void sortColors(std::vector<int>& nums) {
        int zeroCount{ 0 }, oneCount{ 0 }, twoCount{ 0 };

        #pragma GCC unroll 16
        for(int num : nums)
        {
            if(num == 0) ++zeroCount;
            else if(num == 1) ++oneCount;
            else /*if(num == 2)*/ ++twoCount;
        }
        auto zeroBegin = nums.begin(), oneBegin = zeroBegin + zeroCount, twoBegin = oneBegin + oneCount;

        std::fill(zeroBegin, oneBegin, 0);
        std::fill(oneBegin, twoBegin, 1);
        std::fill(twoBegin, twoBegin + twoCount, 2);
    }
};

/*
 * @lc app=leetcode.cn id=75 lang=cpp
 *
 * [75] 颜色分类
 */

// @lc code=start
#include <vector>
#include <algorithm>
#include <iostream>
using namespace std;
class Solution {
public:

    // 调整以 root 为根的子树为最大堆
    void heapify(vector<int>& arr, int n, int root) {
        int largest = root;    // 初始化最大元素为根节点
        int left = 2 * root + 1;  // 左子节点索引
        int right = 2 * root + 2; // 右子节点索引

        // 找到根、左、右中的最大值
        if (left < n && arr[left] > arr[largest]) {
            largest = left;
        }
        if (right < n && arr[right] > arr[largest]) {
            largest = right;
        }

        // 若最大值不是根节点，则交换并递归调整
        if (largest != root) {
            swap(arr[root], arr[largest]);
            heapify(arr, n, largest);
        }
    }

    // 堆排序主函数
    void heapSort(vector<int>& arr) {
        int n = arr.size();

        // 构建最大堆（从最后一个非叶子节点向上调整）
        for (int i = n / 2 - 1; i >= 0; i--) {
            heapify(arr, n, i);
        }

        // 逐个提取堆顶元素排序
        for (int i = n - 1; i > 0; i--) {
            swap(arr[0], arr[i]); // 将最大值移到末尾
            heapify(arr, i, 0);   // 调整剩余元素为堆
        }
    }
    void sortColors(vector<int>& nums) {
#define SORT_STL 
#define HEAP_SORT_STL
//#define QUICK_SORT_STL
#ifdef SORT_STL
#ifdef HEAP_SORT_STL
        make_heap(nums.begin(), nums.end());
        sort_heap(nums.begin(), nums.end());
#elif defined(QUICK_SORT_STL)
        sort(nums.begin(), nums.end());
#endif // HEAP_SORT_STL
#else
        heapSort(nums);
#endif // SORT_STL

    }
};
// @lc code=end
int main(){
    Solution s;
    vector<int> nums{2,0,2,1,1,0};
    s.sortColors(nums);
    for(auto i:nums){
        cout<<i<<" ";
    }
    return 0;
}

