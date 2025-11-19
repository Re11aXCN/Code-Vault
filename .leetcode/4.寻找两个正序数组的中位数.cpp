#include <vector>
#include <algorithm>
#include <limits>
#include <iterator>
using namespace std;

/*
 * @lc app=leetcode.cn id=4 lang=cpp
 *
 * [4] 寻找两个正序数组的中位数
 */

// @lc code=start
class Solution {
public:
    double findMedianSortedArrays(vector<int>& nums1, vector<int>& nums2) {
        // 解法选择开关
        // #define SOLUTION_MERGE_ALL           // 解法1: 完整合并，时间复杂度O(m+n)，空间复杂度O(m+n)
        // #define SOLUTION_PARTIAL_MERGE       // 解法2: 部分合并，时间复杂度O((m+n)/2)，空间复杂度O(1)
        // #define SOLUTION_SWAP_METHOD         // 解法3: 交换法，通过交换元素使数组有序
        #define SOLUTION_BINARY_SEARCH         // 解法4: 二分查找，时间复杂度O(log(min(m,n)))，空间复杂度O(1)

#ifdef SOLUTION_MERGE_ALL
        return mergeAllMethod(nums1, nums2);
#elif defined(SOLUTION_PARTIAL_MERGE)
        return partialMergeMethod(nums1, nums2);
#elif defined(SOLUTION_SWAP_METHOD)
        return swapMethod(nums1, nums2);
#elif defined(SOLUTION_BINARY_SEARCH)
        return binarySearchMethod(nums1, nums2);
#endif
    }

private:
    // ==================== 解法1: 完整合并方法 ====================
    // 思路: 将两个数组合并成一个有序数组，然后直接取中位数
    // 时间复杂度: O(m+n)
    // 空间复杂度: O(m+n)
    double mergeAllMethod(vector<int>& nums1, vector<int>& nums2) {
        vector<int> merged_array;
        merged_array.reserve(nums1.size() + nums2.size());
        
        // 使用标准库的merge函数合并两个有序数组
        merge(nums1.begin(), nums1.end(), 
              nums2.begin(), nums2.end(), 
              back_inserter(merged_array));

        int total_size = merged_array.size();
        int middle_index = total_size >> 1;
        
        // 根据总长度奇偶性返回中位数
        if (total_size & 1) {
            // 奇数情况: 直接返回中间元素
            return static_cast<double>(merged_array[middle_index]);
        } else {
            // 偶数情况: 返回中间两个元素的平均值
            return static_cast<double>(merged_array[middle_index - 1] + merged_array[middle_index]) / 2.0;
        }
    }

    // ==================== 解法2: 部分合并方法 ====================
    // 思路: 只遍历到中位数的位置，不需要完全合并整个数组
    // 时间复杂度: O((m+n)/2)
    // 空间复杂度: O(1)
    double partialMergeMethod(vector<int>& nums1, vector<int>& nums2) {
        int nums1_size = nums1.size();
        int nums2_size = nums2.size();
        // 让nums1始终是较短的数组，提高效率
        if (nums1_size > nums2_size) {
            return partialMergeMethod(nums2, nums1);
        }
        // 处理空数组的特殊情况
        if (nums1.empty()) {
            return getMedianFromSingleArray(nums2);
        }
        if (nums2.empty()) {
            return getMedianFromSingleArray(nums1);
        }
        
        int total_size = nums1_size + nums2_size;
        int median_position = total_size >> 1;  // 中位数的位置
        
        int nums1_index = 0, nums2_index = 0;      // 两个数组的遍历指针
        int previous_value = 0, current_value = 0; // 保存当前值和前一个值
        
        // 遍历到中位数的位置
        for (int step_count = 0; step_count <= median_position; step_count++) {
            previous_value = current_value;  // 保存前一个值（用于偶数长度情况）
            
            // 选择两个数组中较小的元素前进
            if (nums1_index < nums1_size && 
                (nums2_index >= nums2_size || nums1[nums1_index] <= nums2[nums2_index])) {
                current_value = nums1[nums1_index++];
            } else {
                current_value = nums2[nums2_index++];
            }
        }
        
        // 根据总长度奇偶性返回结果
        if (total_size & 1) {
            return static_cast<double>(current_value);
        } else {
            return static_cast<double>(previous_value + current_value) / 2.0;
        }
    }
    
    // 辅助函数: 从单个数组中获取中位数
    double getMedianFromSingleArray(vector<int>& arr) {
        int size = arr.size();
        int middle_index = size >> 1;
        if (size & 1) {
            return static_cast<double>(arr[middle_index]);
        } else {
            return static_cast<double>(arr[middle_index - 1] + arr[middle_index]) / 2.0;
        }
    }

    // ==================== 解法3: 交换方法 ====================
    // 思路: 通过交换元素使两个数组满足 nums1.back() <= nums2.front()，然后递归处理
    // 时间复杂度: 平均O(min(m,n))，最坏O(m*n)
    // 空间复杂度: O(1) (不考虑递归栈空间)
    double swapMethod(vector<int>& nums1, vector<int>& nums2) {
        int nums1_size = nums1.size();
        int nums2_size = nums2.size();
        
        // 处理空数组的特殊情况
        if (nums1.empty()) {
            return getMedianFromSingleArray(nums2);
        }
        if (nums2.empty()) {
            return getMedianFromSingleArray(nums1);
        }
        
        // 检查是否已经满足 nums1.back() <= nums2.front()
        if (nums1.back() <= nums2.front()) {
            return calculateMedianForSeparatedArrays(nums1, nums2);
        }
        
        // 检查是否满足 nums2.back() <= nums1.front()
        if (nums2.back() <= nums1.front()) {
            return calculateMedianForSeparatedArrays(nums2, nums1);
        }
        
        // 通过交换元素使数组满足条件
        optimizeArraysOrder(nums1, nums2);
        
        // 递归调用，现在应该满足条件了
        return swapMethod(nums1, nums2);
    }
    
    // 辅助函数: 计算已分离数组的中位数（nums1全部小于等于nums2）
    double calculateMedianForSeparatedArrays(vector<int>& first_array, vector<int>& second_array) {
        int first_size = first_array.size();
        int second_size = second_array.size();
        int total_size = first_size + second_size;
        int median_index = total_size / 2;
        
        if (total_size & 1) {
            // 奇数情况
            if (second_size > first_size) {
                return static_cast<double>(second_array[median_index - first_size]);
            } else {
                return static_cast<double>(first_array[median_index]);
            }
        } else {
            // 偶数情况
            if (second_size > first_size) {
                return static_cast<double>(second_array[median_index - first_size - 1] + 
                                          second_array[median_index - first_size]) / 2.0;
            } else if (first_size > second_size) {
                return static_cast<double>(first_array[median_index - 1] + 
                                          first_array[median_index]) / 2.0;
            } else {
                return static_cast<double>(first_array.back() + second_array.front()) / 2.0;
            }
        }
    }
    
    // 辅助函数: 优化数组顺序，使nums1.back() <= nums2.front()
    void optimizeArraysOrder(vector<int>& nums1, vector<int>& nums2) {
        auto nums1_iterator = nums1.begin();
        auto nums2_iterator = nums2.begin();
        
        // 找到第一个不满足顺序的位置
        while (nums1_iterator != nums1.end() && 
               nums2_iterator != nums2.end() && 
               *nums1_iterator <= *nums2_iterator) {
            ++nums1_iterator;
        }
        
        // 交换不满足顺序的元素
        while (nums1_iterator != nums1.end() && nums2_iterator != nums2.end()) {
            if (*nums1_iterator > *nums2_iterator) {
                swap(*nums1_iterator, *nums2_iterator);
                
                // 保持nums2的有序性
                auto temp_iterator = nums2_iterator;
                ++temp_iterator;
                while (temp_iterator != nums2.end() && *nums2_iterator > *temp_iterator) {
                    swap(*nums2_iterator, *temp_iterator);
                    nums2_iterator = temp_iterator;
                    ++temp_iterator;
                }
            }
            ++nums1_iterator;
        }
    }

    // ==================== 解法4: 二分查找方法 ====================
    // 思路: 在较短的数组上进行二分查找，找到合适的分割点
    // 时间复杂度: O(log(min(m,n)))
    // 空间复杂度: O(1)
    // 详细描述：
    // 分割两个数字划分为不同部分，左边均小于右边，左边部分至多比右边部分多1，取决于是奇是偶
    // 返回左边的最大值就是我们要寻找的中位数（奇），如果是偶，中位数是 左边最大加右边最小 / 2
    //
    // [1 2 5 | 6(i)]
    // [3 4 | 7(j) 8 9] 
    //
    // (nums1Len + nums2Len + 1) / 2 = 5 = i + j
    // 
    // 需要满足条件 
    // num[i - 1] < num[i] 恒成立
    // num[i - 1] < num[j]
    // num[j - 1] < num[j] 恒成立
    // num[j - 1] < num[i]
    //
    // 通过上述条件找到i就能够确定j
    // 比喻为一根绳子，将左半部分串起来，i+j是绳子长度，它们是此消彼长的关系，
    // 特殊情况绳子可能在上/下半部分，所以可以假设 左 右 两端为 负无穷 和 正无穷
    // 即i可能为0，nums1左边没有最大值假设为负无穷INT_MIN，
    // i也可能为nums1的size，分割的右边没有元素右边没有最小值，假设为正无穷INT_MAX
    // j同理
    double binarySearchMethod(vector<int>& nums1, vector<int>& nums2) {
        // 确保nums1是较短的数组，减少二分查找次数
        if (nums1.size() > nums2.size()) {
            return binarySearchMethod(nums2, nums1);
        }
        
        int nums1_size = nums1.size();
        int nums2_size = nums2.size();
        int total_size = nums1_size + nums2_size;
        int left_part_size = (total_size + 1) >> 1; // 左半部分应有的元素数量
        
        int search_left = 0, search_right = nums1_size; // nums1分割点的搜索范围
        
        while (search_left <= search_right) {
            // 在nums1中选择分割点
            int nums1_split_index = (search_left + search_right) >> 1;
            // 计算nums2中对应的分割点
            int nums2_split_index = left_part_size - nums1_split_index;
            
            // 处理边界情况，确定四个关键值
            int nums1_left_max = (nums1_split_index == 0) ? numeric_limits<int>::min() : nums1[nums1_split_index - 1];
            int nums1_right_min = (nums1_split_index == nums1_size) ? numeric_limits<int>::max() : nums1[nums1_split_index];
            int nums2_left_max = (nums2_split_index == 0) ? numeric_limits<int>::min() : nums2[nums2_split_index - 1];
            int nums2_right_min = (nums2_split_index == nums2_size) ? numeric_limits<int>::max() : nums2[nums2_split_index];
            
            // 检查分割点是否正确
            if (nums1_left_max <= nums2_right_min && nums2_left_max <= nums1_right_min) {
                // 找到正确的分割点
                if (total_size & 1) {
                    // 奇数情况: 左半部分的最大值就是中位数
                    return static_cast<double>(max(nums1_left_max, nums2_left_max));
                } else {
                    // 偶数情况: 左半部分最大值和右半部分最小值的平均值
                    return (max(nums1_left_max, nums2_left_max) + min(nums1_right_min, nums2_right_min)) / 2.0;
                }
            } else if (nums1_left_max > nums2_right_min) {
                // nums1的分割点太靠右，需要左移
                search_right = nums1_split_index - 1;
            } else {
                // nums1的分割点太靠左，需要右移
                search_left = nums1_split_index + 1;
            }
        }
        
        return 0.0; // 理论上不会执行到这里
    }
};
// @lc code=end