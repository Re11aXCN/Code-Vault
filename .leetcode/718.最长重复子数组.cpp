/*
 * @lc app=leetcode.cn id=718 lang=cpp
 *
 * [718] 最长重复子数组
 */

// @lc code=start

// 解法选择宏定义
#define DP_VERSION           // 默认：动态规划解法（优化版）
// #define SLIDING_WINDOW_VERSION  // 滑动窗口解法
// #define SA_VERSION            // 后缀自动机解法
// #define SIMPLE_DP_VERSION     // 简单动态规划解法

class Solution {
public:

#if defined(DP_VERSION)
    /**
     * 解法一：动态规划（空间优化版）
     * 思路：使用一维DP数组，从后向前遍历避免覆盖问题
     * 时间复杂度：O(m*n)，空间复杂度：O(min(m,n))
     * 状态定义：dp[j]表示以nums2[j-1]结尾的子数组与以nums1[i-1]结尾的子数组的最大公共长度
     */
    int findLength(vector<int>& nums1, vector<int>& nums2) {
        // 确保nums1是较短的数组，以优化空间复杂度
        if (nums1.size() > nums2.size()) std::swap(nums1, nums2);
        
        // dp数组：dp[j]表示当前处理nums1的第i个元素时，以nums2[j-1]结尾的公共子数组长度
        std::vector<int> dp(nums2.size() + 1, 0);
        int result = 0;  // 记录全局最大值
        
        // 遍历nums1的每个元素
        for (int i = 1; i <= nums1.size(); ++i) {
            // 从后向前遍历，避免覆盖前一行的数据
            #pragma clang loop unroll_count(8)  // 循环展开优化提示
            for (int j = nums2.size(); j >= 1; --j) {
                if (nums2[j - 1] != nums1[i - 1]) dp[j] = 0;// 当前元素不相等，公共长度为0
                // 当前元素相等，公共长度 = 左上角值 + 1
                else if((dp[j] = dp[j - 1] + 1) > result) result = dp[j];
                /*
                if (nums2[j - 1] == nums1[i - 1]) {
                    if((dp[j] = dp[j - 1] + 1) > result) result = dp[j];
                }
                else dp[j] = 0;
                */
            }
        }
        return result;
    }

#elif defined(SLIDING_WINDOW_VERSION)
    /**
     * 解法二：滑动窗口对齐法
     * 思路：将两个数组错位对齐，在重叠部分比较
     * 时间复杂度：O((m+n)*min(m,n))，空间复杂度：O(1)
     * 优势：不需要额外空间，适合内存受限场景
     */
    int findLength(vector<int>& nums1, vector<int>& nums2) {
        if (nums1.empty() || nums2.empty()) return 0;
        int m = nums1.size(), n = nums2.size();
        int maxLen = 0;
        
        // delta表示nums2相对于nums1的偏移量
        // delta范围：从nums1完全在nums2左侧到nums1完全在nums2右侧
        for (int delta = -(m - 1); delta <= n - 1; ++delta) {
            // 计算两个数组的重叠部分
            const int start_i = delta < 0 ? -delta : 0;  // nums1的起始索引
            const int start_j = delta > 0 ? delta : 0;   // nums2的起始索引
            const int steps = std::min(m - start_i, n - start_j);  // 重叠长度
            
            int len = 0;  // 当前连续匹配长度
            int i = start_i, j = start_j;
            
            // 遍历重叠部分
            #pragma clang loop unroll_count(4)  // 循环展开优化提示
            for (int k = 0; k < steps; ++k, ++i, ++j) {
                // 比较对应位置元素
                bool match = (nums1[i] == nums2[j]);
                // 匹配成功，连续长度+1
                if (match) { if (++len > maxLen) maxLen = len; }
                else len = 0; // 匹配失败，重置连续长度
            }
        }

        return maxLen;
    }

#elif defined(SIMPLE_DP_VERSION)
    /**
     * 解法三：简单动态规划（朴素版）
     * 思路：使用二维DP思路，但通过优化减少循环
     * 时间复杂度：O(m*n)，空间复杂度：O(1)
     * 通过分两轮比较减少判断次数
     */
    int findLength(vector<int>& nums1, vector<int>& nums2) {
        if (nums1.empty() || nums2.empty()) return 0;
        int num1Size = nums1.size(), num2Size = nums2.size();
        int maxLen = 0;

        // 第一轮：固定nums1的起始位置，nums2从0开始
        for (int start1 = 0; start1 < num1Size; ++start1) {
            int i = start1, j = 0, len = 0;
            #pragma clang loop unroll_count(4)
            while (i < num1Size && j < num2Size) {
                if (nums1[i] != nums2[j]) len = 0;
                else if (++len > maxLen) maxLen = len;
                ++i, ++j;
            }
        }
        
        // 第二轮：固定nums2的起始位置（从1开始，0已在第一轮覆盖），nums1从0开始
        for (int start2 = 1; start2 < num2Size; ++start2) {
            int i = 0, j = start2, len = 0;
            #pragma clang loop unroll_count(4)
            while (i < num1Size && j < num2Size) {
                if (nums1[i] != nums2[j]) len = 0;
                else if (++len > maxLen) maxLen = len;
                ++i, ++j;
            }
        }
        
        return maxLen;
    }

#elif defined(SA_VERSION)
    /**
     * 解法四：后缀自动机（Suffix Automaton）
     * 思路：为nums1构建后缀自动机，然后用nums2进行匹配
     * 时间复杂度：O(m+n)，空间复杂度：O(m)
     * 优势：理论最优复杂度，但常数较大，适合超大数组
     */
    
    // 后缀自动机状态结构
    struct State {
        int len;                  // 该状态能接受的最长子串长度
        int link;                 // 后缀链接，指向更短的状态
        std::unordered_map<int, int> next;  // 转移函数，键为字符（这里为整数）
    };

    // 后缀自动机类
    class SuffixAutomaton {
    private:
        std::vector<State> st;  // 状态数组
        int last;               // 最后添加的状态索引
        int size;               // 状态总数

    public:
        // 构造函数，预分配空间
        SuffixAutomaton(int maxSize) {
            st.resize(maxSize * 2);  // 后缀自动机状态数不超过2n-1
            last = 0;
            size = 1;
            st[0].len = 0;
            st[0].link = -1;  // 初始状态无后缀链接
        }

        // 扩展自动机，添加一个字符
        void extend(int c) {
            int cur = size++;  // 创建新状态
            st[cur].len = st[last].len + 1;
            int p = last;
            
            // 为所有没有c转移的状态添加转移
            while (p != -1 && st[p].next.find(c) == st[p].next.end()) {
                st[p].next[c] = cur;
                p = st[p].link;
            }
            
            if (p == -1) {
                st[cur].link = 0;  // 链接到初始状态
            } else {
                int q = st[p].next[c];
                if (st[p].len + 1 == st[q].len) {
                    st[cur].link = q;
                } else {
                    // 需要克隆状态q
                    int clone = size++;
                    st[clone].len = st[p].len + 1;
                    st[clone].next = st[q].next;  // 复制转移
                    st[clone].link = st[q].link;
                    
                    // 重定向转移
                    while (p != -1 && st[p].next[c] == q) {
                        st[p].next[c] = clone;
                        p = st[p].link;
                    }
                    st[q].link = st[cur].link = clone;
                }
            }
            last = cur;
        }

        // 在自动机中查找与给定字符串的最长公共子串
        int findLongestCommonSubstring(const std::vector<int>& s) {
            int v = 0;  // 当前状态
            int l = 0;  // 当前匹配长度
            int best = 0;  // 最优匹配长度
            
            for (int i = 0; i < s.size(); ++i) {
                int c = s[i];
                
                // 如果当前状态没有c转移，沿后缀链接回溯
                while (v != 0 && st[v].next.find(c) == st[v].next.end()) {
                    v = st[v].link;
                    l = st[v].len;  // 更新匹配长度为当前状态的最大长度
                }
                
                // 如果有c转移，进入新状态
                if (st[v].next.find(c) != st[v].next.end()) {
                    v = st[v].next[c];
                    l++;  // 匹配长度增加
                }
                
                // 更新最优值
                if (l > best) {
                    best = l;
                }
            }
            return best;
        }
    };
    
    // 使用后缀自动机解决问题
    int findLength(vector<int>& nums1, vector<int>& nums2) {
        // 为nums1构建后缀自动机
        SuffixAutomaton sam(nums1.size());
        for (int num : nums1) {
            sam.extend(num);
        }
        // 在自动机中匹配nums2，返回最长匹配长度
        return sam.findLongestCommonSubstring(nums2);
    }

#endif

};
// @lc code=end



