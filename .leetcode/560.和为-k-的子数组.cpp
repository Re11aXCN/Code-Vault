
// 仅能用于非负数
class Solution {
public:
    int subarraySum(std::vector<int>& nums, int k) {
        int count{ 0 }, window{ 0 };
        for(int i = 0; i < nums.size(); ++i)
        {
            int num = nums[i];
            
            if(num < k) 
            {
                window += num;
                if (window == k) {
                    window = num, ++count;
                }
            }
            else if (num > k)
            {
                window = 0;
            }
            else ++count;
        }
        return count;
    }
};
/*
 * @lc app=leetcode.cn id=560 lang=cpp
 *
 * [560] 和为 K 的子数组
 */

// @lc code=start
#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <functional> // 用于 std::function
#include <ranges>     // 使用C++20的ranges和views
using namespace std;
// 边结构体，包含起点、终点、权重和索引
struct Edge {
    string from;
    string to;
    int weight;
    int index;
};

// 邻接表条目，包含目标节点、权重和边索引
struct AdjEntry {
    int node;
    int weight;
    int edge_index;
};

vector<vector<int>> findPaths(const vector<Edge>& edges, int target) {
    // 收集所有唯一的节点名称
    unordered_set<string> node_set;
    for (const auto& edge : edges) {
        node_set.insert(edge.from);
        node_set.insert(edge.to);
    }

    // 将节点名称映射到唯一的整数ID
    unordered_map<string, int> node_id;
    int id = 0;
    for (const auto& node : node_set) {
        node_id[node] = id++;
    }

    // 构建邻接表
    vector<vector<AdjEntry>> adj(node_id.size());
    for (const auto& edge : edges) {
        int u = node_id[edge.from];
        int v = node_id[edge.to];
        adj[u].push_back({ v, edge.weight, edge.index });
        adj[v].push_back({ u, edge.weight, edge.index }); // 无向图，添加双向边
    }

    vector<vector<int>> result; // 存储所有符合条件的路径
    set<vector<int>> unique_paths; // 用于去重

    // DFS遍历函数
    function<void(int, int, vector<int>, unordered_set<int>)> dfs =
        [&](int current_node, int current_sum, vector<int> path, unordered_set<int> visited) {
        // 当路径和等于目标值且路径非空时，保存路径（去重）
        if (current_sum == target && !path.empty()) {
            vector<int> sorted_path = path;
            ranges::sort(sorted_path);
            if (!unique_paths.contains(sorted_path)) {
                unique_paths.insert(sorted_path);
                result.push_back(path);
            }
        }

        // 遍历所有邻接节点
        for (const auto& entry : adj[current_node]) {
            if (!visited.contains(entry.node)) {
                unordered_set<int> new_visited = visited;
                new_visited.insert(entry.node); // 标记已访问节点
                vector<int> new_path = path;
                new_path.push_back(entry.edge_index); // 添加当前边到路径
                int new_sum = current_sum + entry.weight;
                dfs(entry.node, new_sum, new_path, new_visited); // 递归DFS
            }
        }
        };

    // 对每个节点作为起点进行DFS遍历
    for (int node : views::iota(0, static_cast<int>(adj.size()))) {
        dfs(node, 0, {}, { node }); // 初始状态：当前和为0，路径为空，已访问当前节点
    }

    return result;
}
/**
 * @brief 根据权值数组构造链式连接的边列表
 * @param weights 边的权值数组
 * @return vector<Edge> 生成的边列表，边按顺序连接 A0-A1, A1-A2...
 */
vector<Edge> createEdges(const vector<int>& weights) {
    vector<Edge> edges;
    for (size_t i = 0; i < weights.size(); ++i) {
        // 生成节点名称：A0, A1, A2...
        string from_node = "A" + to_string(i);
        string to_node = "A" + to_string(i + 1);

        // 创建边对象
        edges.emplace_back(Edge{
            .from = from_node,
            .to = to_node,
            .weight = weights[i],
            .index = static_cast<int>(i)  // 边索引与数组索引一致
            });
    }
    return edges;
}
class Solution {
public:
    /// <summary>
    /*
    * 核心思想
    * 前缀和：通过维护前缀和数组，快速计算任意子数组的和。 
    * 哈希表优化：记录前缀和出现的次数，利用差值直接查找符合条件的子数组。

    * 步骤说明
    * 前缀和定义：pre_sum[i] 表示数组前 i 个元素的和。
    * 例如：nums = [1,2,3]，则 pre_sum = [0,1,3,6]。
    * 子数组和公式：子数组 nums[j..i] 的和为 pre_sum[i+1] - pre_sum[j]。
    * 目标条件：寻找满足 pre_sum[i+1] - pre_sum[j] = k 的 j 的数量。
    * 等价于：对于每个 i，统计哈希表中 pre_sum[i+1] - k 的出现次数。
    * 哈希表记录：实时维护前缀和及其出现次数的映射，避免重复计算。
    */
    /// </summary>
    /// <param name="nums"></param>
    /// <param name="k"></param>
    /// <returns></returns>
    int subarraySum(vector<int>& nums, int k) {
#ifdef UndirectedGraph_DFS
        auto paths = findPaths(createEdges(nums), k);
        return static_cast<int>(paths.size());
#else
        // 要点：将问题转化为 寻找差为k的前缀和组合的个数


        // k 前缀和的值，v前缀和取值为key的子数组个数
        std::unordered_map<int, int> preSumMap;
        preSumMap[0] = 1;   // 表示子数组为空，和为0，个数1
        int res{0}, longerPreNum{0};
        
        for(int num : nums)
        {
            longerPreNum += num;

            if(auto it = preSumMap.find(/*shorterPreNum*/longerPreNum - k); it != preSumMap.end())
            {
                res += it->second;
            }
            // 将当前前缀和加入哈希表
            preSumMap[longerPreNum]++;
        }
        return res;
#endif
    }
};
// @lc code=end
int main()
{
    // 1,2,3,4,5,6,7,1,23,21,3,1,2,1,1,1,1,1,12,2,3,2,3,2,2
    // 示例输入：边列表和目標值
    vector<Edge> edges = createEdges({ 1,2,3,4,5,6,7,1,23,21,3,1,2,1,1,1,1,1,12,2,3,2,3,2,2 });
    int target = 1;

    // 查找所有符合条件的路径
    auto paths = findPaths(edges, target);

    // 输出结果
    for (const auto& path : paths) {
        cout << "[";
        for (size_t i = 0; i < path.size(); ++i) {
            cout << path[i];
            if (i != path.size() - 1) cout << ", ";
        }
        cout << "]" << endl;
    }

    return 0;
}
#ifdef TEST
int subarraySum(vector<int>& nums, int k) {
    if (nums.empty()) return 0;
    if (nums.size() == 1) {
        if (nums[0] == k) return 1;
        else return 0;
    }
    int left = 0;
    int right = 1;
    int result = 0;
    int sum = 0;
    while (right < nums.size()) {
        if (nums[left] == k && nums[right] != k) {
            ++result;
            if (nums[right] > k) {
                left = right + 1;
                right = left + 1;
            }
            else {
                ++left;
                ++right;
            }
        }
        else if (nums[left] != k && nums[right] == k) {
            ++result;
            left = right + 1;
            right = left + 1;
        }
        else {
            if (nums[left] > k && nums[right] > k) {
                left = right + 1;
                right = left + 1;
            }
            else if (nums[left] < k && nums[right] > k) {
                if (nums[left] < 0 && nums[right] > 0) {
                    if ((nums[left] + nums[right]) == k) {
                        ++result;
                        if (k == 0) {
                            int using_right = right + 1;
                            while (using_right < nums.size() && nums[using_right] == 0) {
                                ++using_right;
                                ++result;
                            }
                        }
                    }
                }
                left = right + 1;
                right = left + 1;
            }
            else if (nums[left] > k && nums[right] < k) {
                if (nums[left] > 0 && nums[right] < 0) {
                    if ((nums[left] + nums[right]) == k) {
                        ++result;
                        if (k == 0) {
                            int using_right = right + 1;
                            while (using_right < nums.size() && nums[using_right] == 0) {
                                ++using_right;
                                ++result;
                            }
                        }
                    }
                }
                ++left;
                ++right;
            }
            else {
                sum = nums[left] + nums[right];
                if (nums[left] == 0 && nums[right] == 0) result += 2;
                if (sum > k) {
                    ++left;
                    ++right;
                }
                else if (sum == k) {
                    ++result;
                    ++left;
                    ++right;
                }
                else {
                    int using_right = right + 1;
                    while (using_right < nums.size()) {
                        sum += nums[using_right];
                        ++using_right;
                        if (sum > k) {
                            break;
                        }
                        else if (sum == k) {
                            ++result;
                            break;
                        }
                    }
                    ++left;
                    ++right;
                }
            }
        }
    }
    return result;
}
// int subarraySum(vector<int>& nums, int k) {
//     -1 0
//     1 3 5 6 7 9 11
//     2 1 2 10 13 14
//     3 15
//     4 4 8 12 15
//     // -1 2 2 1 4 1 1 1 4 1 2 1 4 2 2 4 2 3 k=3 输出5
//     // 切割大于等于3的数，如果等于3，就++
//     // -1 2 2 1 切割 1 1 1 切割 1 2 1 切割 2 2 切割 2
//     // 另外，想要子数组和=k，那么必须有一个数小于k的一半向下取整，可以考虑切割的时候处理？
//     // 循环切割后的数组，如果数组大小是1，直接跳过
//     int length = nums.size();
//     int result = 0;
//     int base_num = k / 2;
//     bool has_less_base_num = false;
//     vector<int> sub_arr;
//     sub_arr.reserve(length);
//     int left = 0;
//     int sum = 0;
//     int p = 0;
//     for (int i = 0; i < length; ++i)
//     {
//         if (nums[i] > k) {
//             if (!has_less_base_num) {
//                 sub_arr.clear();
//                 continue;
//             }
//             for (int right = 1; right < sub_arr.size(); ++right) {
//                 sum = sub_arr[left] + sub_arr[right];
//                 if (sum > k) {
//                     ++left;
//                     ++right;
//                 }
//                 else if (sum == k) {
//                     ++result;
//                 }
//                 else {
//                     int stored_right = right;
//                     p = right + 1;
//                     while()
//                 }
//             }
//             sub_arr.clear();
//             has_less_base_num = false;
//         }
//         else if (nums[i] == k) {
//             ++result;
//         }
//         else {
//             if (!has_less_base_num && base_num >= nums[i]) {
//                 has_less_base_num = true;
//             }
//             sub_arr.push_back(nums[i]);
//         }
//     }
//     return result;
// }
#endif // TEST