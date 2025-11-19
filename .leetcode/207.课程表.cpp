// 稀疏最优，三目运算优化
// 稠密情况下慢于递归，1.1~1.4倍
// 稀疏情况下快于递归，5倍以上
/*
5个节点，无环依赖关系：prerequisites = [[1,0], [2,0], [3,1], [4,2], [3,2]]
// 处理每个边，统计到 outDegree[i+1]
[1,0] → outDegree[0+1]++ → outDegree[1] = 1
[2,0] → outDegree[0+1]++ → outDegree[1] = 2
[3,1] → outDegree[1+1]++ → outDegree[2] = 1  
[4,2] → outDegree[2+1]++ → outDegree[3] = 1
[3,2] → outDegree[2+1]++ → outDegree[3] = 2

// 最终 outDegree 数组
outDegree = [0, 2, 1, 2, 0, 0]
  索引:       0  1  2  3  4  5
inDegree =  [0, 1, 1, 2, 1]

// 计算 inclusive prefix sum
offset[0] = 0
offset[1] = 0 + 2 = 2
offset[2] = 2 + 1 = 3  
offset[3] = 3 + 2 = 5
offset[4] = 5 + 0 = 5
offset[5] = 5 + 0 = 5

// 最终 offset 数组
offset = [0, 2, 3, 5, 5, 5]
 索引:     0  1  2  3  4  5

// currentPos 初始 = offset = [0, 2, 3, 5, 5, 5]

[1,0] → edges[currentPos[0]] = 1, currentPos[0]++ → edges[0]=1
[2,0] → edges[currentPos[0]] = 2, currentPos[0]++ → edges[1]=2  
[3,1] → edges[currentPos[1]] = 3, currentPos[1]++ → edges[2]=3
[4,2] → edges[currentPos[2]] = 4, currentPos[2]++ → edges[3]=4
[3,2] → edges[currentPos[2]] = 3, currentPos[2]++ → edges[4]=3

// 最终 edges 数组
edges = [1, 2, 3, 4, 3]
 索引:    0  1  2  3  4
*/
class Solution {
public:
    bool canFinish(int numCourses, vector<vector<int>>& prerequisites) {
        std::vector<int> outDegree(numCourses + 1, 0);
        std::vector<int> inDegree(numCourses, 0);

        #pragma clang loop vectorize(enable) interleave(enable) unroll_count(8)
        for(auto& pre : prerequisites) {
            ++outDegree[pre[1] + 1];
            ++inDegree[pre[0]];
        }
        /*
        #pragma clang loop unroll_count(8)
        for(int i = 0; i < numCourses; ++i) outDegree[i + 1] += outDegree[i];
        
        std::partial_sum(outDegree.begin(), outDegree.end(), outDegree.begin());
        */
        std::inclusive_scan(/*std::execution::par, */outDegree.begin(), outDegree.end(), outDegree.begin());
        
        std::vector<int> edges(prerequisites.size());
        std::vector<int> currentPos = outDegree;

        #pragma clang loop vectorize(enable) interleave(enable) unroll_count(8)
        for(auto& pre : prerequisites) edges[currentPos[pre[1]]++] = pre[0];

        std::vector<int>& queue = currentPos;
        int front{ 0 }, back{ 0 }, count{ 0 };
        #pragma clang loop vectorize(enable) unroll_count(8)
        for(int i = 0; i < numCourses; ++i) {
            if (inDegree[i] == 0) queue[back++] = i;
        }

        while(front < back) {
            int u = queue[front++];
            ++count;

            #pragma clang loop vectorize(enable) unroll_count(4)
            for(int i = outDegree[u]; i < outDegree[u + 1]; ++i) {
                int v = edges[i];
                if (--inDegree[v] == 0) queue[back++] = v;
            }
        }
        return count == numCourses;
    }
};

/*
假设：numCourses = 4, prerequisites = [[1,0],[2,0],[3,1],[3,2]]

依赖关系：0 → 1, 0 → 2, 1 → 3, 2 → 3

outDegree = [2,1,1,0] (课程0有2个出边，课程1有1个出边...)
inDegree = [0,1,1,2] (课程0入度0，课程3入度2)
// 将outDegree转换为前缀和
offset = [2,3,4,4]

vector<int> edges(4);        // [0,0,0,0]
vector<int> currentPos = offset;  // [2,3,4,4]

// 填充edges数组
[1,0]: u=0, edges[--currentPos[0]=1] = 1  // edges=[0,1,0,0], currentPos=[1,3,4,4]
[2,0]: u=0, edges[--currentPos[0]=0] = 2  // edges=[2,1,0,0], currentPos=[0,3,4,4]  
[3,1]: u=1, edges[--currentPos[1]=2] = 3  // edges=[2,1,3,0], currentPos=[0,2,4,4]
[3,2]: u=2, edges[--currentPos[2]=3] = 3  // edges=[2,1,3,3], currentPos=[0,2,3,4]

课程0的出边：索引 0-1 (edges[0]=2, edges[1]=1) → 0→2, 0→1

课程1的出边：索引 2-2 (edges[2]=3) → 1→3

课程2的出边：索引 3-3 (edges[3]=3) → 2→3

课程3的出边：无
*/
class Solution {
public:
    bool canFinish(int numCourses, vector<vector<int>>& prerequisites) {
        // Step 1: 统计每个顶点的出度和入度
        // 注意：这里变量命名有误导，outDegree实际上用于后续构建CSR的offset数组
        vector<int> outDegree(numCourses, 0);  // 用于记录每个课程的出边数量（后续课程数）
        vector<int> inDegree(numCourses, 0);   // 记录每个课程的入度（先修课程数）
        
        // 统计每个顶点的出度和入度
        #pragma GCC unroll 8  // 编译器优化提示：尝试循环展开8次
        for (auto& pre : prerequisites) {
            // pre[1] → pre[0] 的依赖关系
            ++outDegree[pre[1]];  // 课程pre[1]的出度+1（有多少后续课程）
            ++inDegree[pre[0]];   // 课程pre[0]的入度+1（有多少先修课程）
        }
        
        // Step 2: 构建CSR(Compressed Sparse Row)格式的邻接表
        // 重用outDegree数组作为CSR的offset数组
        vector<int>& offset = outDegree;  // offset[i]表示课程i的出边在edges中的起始位置
        
        // 将出度转换为前缀和，构建offset数组
        #pragma GCC unroll 8
        for (int i = 1; i < numCourses; ++i) {
            offset[i] += offset[i - 1];  // offset[i]现在表示前i个课程的总出边数
        }
        
        // 创建edges数组存储所有出边
        vector<int> edges(prerequisites.size());
        vector<int> currentPos = offset;  // 用于记录当前插入位置
        
        // 填充edges数组
        #pragma GCC unroll 8
        for (auto& pre : prerequisites) {
            int u = pre[1];  // 当前课程
            int v = pre[0];  // 后续课程
            edges[--currentPos[u]] = v;  // 将后续课程v插入到u的出边列表中
        }
        // 此时offset数组的含义：
        // - offset[i] 表示课程i的出边在edges中的结束位置（下一个位置的索引）
        // - 课程i的出边范围：从 (i==0?0:offset[i-1]) 到 offset[i]
        
        // Step 3: 拓扑排序 - Kahn算法
        vector<int>& queue = currentPos;  // 用定长数组模拟队列，避免动态扩容
        int front = 0, rear = 0;       // 队列头尾指针
        
        // 初始化：将所有入度为0的课程加入队列
        #pragma GCC unroll 8
        for (int i = 0; i < numCourses; ++i) {
            if (inDegree[i] == 0) {
                queue[rear++] = i;  // 入队
            }
        }
        
        int processed = 0;  // 记录已处理的课程数
        
        // BFS遍历
        while (front < rear) {
            int u = queue[front++];  // 出队
            ++processed;
            
            // 获取课程u的所有出边范围
            int start = (u == 0) ? 0 : offset[u - 1];
            int end = offset[u];
            
            // 遍历u的所有后续课程
            #pragma GCC unroll 8
            for (int i = start; i < end; ++i) {
                int v = edges[i];  // 后续课程v
                // 移除u→v的边，即减少v的入度
                if (--inDegree[v] == 0) {
                    queue[rear++] = v;  // 如果v的入度变为0，加入队列
                }
            }
        }
        
        // 如果所有课程都被处理，说明没有环，可以完成所有课程
        return processed == numCourses;
    }
};
/* 
 * @lc app=leetcode.cn id=207 lang=cpp 
 * 
 * [207] 课程表 
 */ 

// @lc code=start
#include <vector>
#include <queue>
using namespace std;

class Solution {
public:
    bool canFinish(int numCourses, vector<vector<int>>& prerequisites) {
#define DFS
#ifdef DFS
        // 构建邻接表
        vector<vector<int>> graph(numCourses);
        for (auto& pre : prerequisites) {
            graph[pre[1]].push_back(pre[0]);  // 添加边：先修课程 -> 当前课程
        }
        
        // 访问状态数组：0=未访问，1=正在访问，2=已完成访问
        vector<int> visited(numCourses, 0);
        
        // 对每个未访问的节点进行DFS
        for (int i = 0; i < numCourses; i++) {
            if (visited[i] == 0 && !dfs(graph, visited, i)) {
                return false;  // 如果检测到环，则无法完成所有课程
            }
        }
        
        return true;  // 没有检测到环，可以完成所有课程
#else
        // 构建邻接表和入度数组
        vector<vector<int>> graph(numCourses);  // 邻接表
        vector<int> inDegree(numCourses, 0);    // 入度数组
        
        // 填充邻接表和入度数组
        for (auto& pre : prerequisites) {
            int course = pre[0];  // 当前课程
            int preCourse = pre[1];  // 先修课程
            graph[preCourse].push_back(course);  // 添加边：先修课程 -> 当前课程
            inDegree[course]++;  // 当前课程的入度加1
        }
        
        // 将所有入度为0的节点加入队列
        queue<int> q;
        for (int i = 0; i < numCourses; i++) {
            if (inDegree[i] == 0) {
                q.push(i);
            }
        }
        
        // 拓扑排序
        int count = 0;  // 记录已访问的节点数
        while (!q.empty()) {
            int curr = q.front();
            q.pop();
            count++;  // 已访问节点数加1
            
            // 遍历当前节点的所有邻接节点
            for (int next : graph[curr]) {
                inDegree[next]--;  // 将邻接节点的入度减1
                // 如果入度变为0，则加入队列
                if (inDegree[next] == 0) {
                    q.push(next);
                }
            }
        }
        
        // 如果已访问的节点数等于总节点数，则说明不存在环
        return count == numCourses;
#endif
    }
private:
    // DFS检测环
    bool dfs(const vector<vector<int>>& graph, vector<int>& visited, int curr) {
        // 如果当前节点正在被访问，说明存在环
        if (visited[curr] == 1) {
            return false;
        }
        
        // 如果当前节点已经被完全访问过，无需再次访问
        if (visited[curr] == 2) {
            return true;
        }
        
        // 标记当前节点为"正在访问"
        visited[curr] = 1;
        
        // 访问所有邻接节点
        for (int next : graph[curr]) {
            if (!dfs(graph, visited, next)) {
                return false;  // 如果检测到环，则返回false
            }
        }
        
        // 标记当前节点为"已完成访问"
        visited[curr] = 2;
        
        return true;  // 当前节点及其所有后继节点都没有环
    }
};
// @lc code=end
/*
// 随机生成测试
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <cassert>

using namespace std;

class TestCaseGenerator {
private:
    mt19937 rng;
    
public:
    TestCaseGenerator() : rng(random_device{}()) {}
    
    // 生成无环的依赖关系（确保可以完成）
    vector<vector<int>> generateAcyclicPrerequisites(int numCourses, int edgeDensity = 0.3) {
        vector<vector<int>> prerequisites;
        
        // 生成一个有效的拓扑序列
        vector<int> order(numCourses);
        for (int i = 0; i < numCourses; ++i) {
            order[i] = i;
        }
        shuffle(order.begin(), order.end(), rng);
        
        // 根据拓扑序列生成边（只能从前面指向后面）
        int maxEdges = min(numCourses * (numCourses - 1) / 2, 
                          static_cast<int>(numCourses * numCourses * edgeDensity));
        
        for (int i = 0; i < numCourses && prerequisites.size() < maxEdges; ++i) {
            for (int j = i + 1; j < numCourses && prerequisites.size() < maxEdges; ++j) {
                // 50%概率添加这条边
                if (uniform_real_distribution<double>(0, 1)(rng) < 0.5) {
                    prerequisites.push_back({order[j], order[i]}); // order[i] → order[j]
                }
            }
        }
        
        // 打乱边的顺序
        shuffle(prerequisites.begin(), prerequisites.end(), rng);
        return prerequisites;
    }
    
    // 生成有环的依赖关系（确保不能完成）
    vector<vector<int>> generateCyclicPrerequisites(int numCourses) {
        vector<vector<int>> prerequisites;
        
        if (numCourses < 3) {
            // 对于小规模，创建一个简单环
            for (int i = 0; i < numCourses; ++i) {
                prerequisites.push_back({i, (i + 1) % numCourses});
            }
            return prerequisites;
        }
        
        // 创建一个环：0->1->2->...->(n-1)->0
        for (int i = 0; i < numCourses - 1; ++i) {
            prerequisites.push_back({i + 1, i});
        }
        prerequisites.push_back({0, numCourses - 1}); // 闭合环
        
        // 添加一些额外边增加复杂度
        for (int i = 0; i < numCourses / 2; ++i) {
            int u = uniform_int_distribution<int>(0, numCourses - 1)(rng);
            int v = uniform_int_distribution<int>(0, numCourses - 1)(rng);
            if (u != v) {
                prerequisites.push_back({v, u});
            }
        }
        
        shuffle(prerequisites.begin(), prerequisites.end(), rng);
        return prerequisites;
    }
        
    vector<vector<int>> generateDensePrerequisites(int numCourses, double density = 0.8) {
        vector<vector<int>> prerequisites;
        int maxEdges = numCourses * (numCourses - 1);
        int targetEdges = maxEdges * density;
        
        // 生成所有可能的边（避免重复）
        vector<pair<int, int>> allEdges;
        for (int i = 0; i < numCourses; ++i) {
            for (int j = 0; j < numCourses; ++j) {
                if (i != j) {
                    allEdges.push_back({i, j});
                }
            }
        }
        
        // 随机打乱并选择前 targetEdges 条
        shuffle(allEdges.begin(), allEdges.end(), rng);
        for (int i = 0; i < min(targetEdges, (int)allEdges.size()); ++i) {
            prerequisites.push_back({allEdges[i].first, allEdges[i].second});
        }
        
        return prerequisites;
    }
    
    // 生成随机依赖关系（可能包含环）
    vector<vector<int>> generateRandomPrerequisites(int numCourses, int minEdges, int maxEdges) {
        vector<vector<int>> prerequisites;
        int numEdges = uniform_int_distribution<int>(minEdges, maxEdges)(rng);
        
        for (int i = 0; i < numEdges; ++i) {
            int u = uniform_int_distribution<int>(0, numCourses - 1)(rng);
            int v = uniform_int_distribution<int>(0, numCourses - 1)(rng);
            if (u != v) {
                prerequisites.push_back({u, v});
            }
        }
        
        return prerequisites;
    }
    
    // 可视化依赖关系
    void visualizePrerequisites(int numCourses, const vector<vector<int>>& prerequisites) {
        cout << "课程数量: " << numCourses << endl;
        cout << "依赖关系 (" << prerequisites.size() << " 条):" << endl;
        
        // 统计每个课程的入度和出度
        vector<int> inDegree(numCourses, 0), outDegree(numCourses, 0);
        for (const auto& pre : prerequisites) {
            outDegree[pre[1]]++;
            inDegree[pre[0]]++;
        }
        
        // 打印依赖关系图
        for (const auto& pre : prerequisites) {
            cout << "  " << pre[1] << " → " << pre[0] << endl;
        }
        
        // 打印统计信息
        cout << "\n度统计:" << endl;
        for (int i = 0; i < numCourses; ++i) {
            cout << "  课程" << i << ": 入度=" << inDegree[i] << ", 出度=" << outDegree[i] << endl;
        }
    }
};

// 测试函数
void runTests() {
    TestCaseGenerator generator;
    
    cout << "=== 测试1: 无环依赖关系 ===" << endl;
    {
        int numCourses = 6;
        auto prerequisites = generator.generateAcyclicPrerequisites(numCourses, 0.4);
        generator.visualizePrerequisites(numCourses, prerequisites);
        
        Solution sol;
        bool result = sol.canFinish(numCourses, prerequisites);
        cout << "能否完成所有课程: " << (result ? "是" : "否") << endl;
        assert(result == true);
        cout << "✓ 测试通过" << endl << endl;
    }
    
    cout << "=== 测试2: 有环依赖关系 ===" << endl;
    {
        int numCourses = 5;
        auto prerequisites = generator.generateCyclicPrerequisites(numCourses);
        generator.visualizePrerequisites(numCourses, prerequisites);
        
        Solution sol;
        bool result = sol.canFinish(numCourses, prerequisites);
        cout << "能否完成所有课程: " << (result ? "是" : "否") << endl;
        assert(result == false);
        cout << "✓ 测试通过" << endl << endl;
    }
    
    cout << "=== 测试3: 空依赖关系 ===" << endl;
    {
        int numCourses = 4;
        vector<vector<int>> prerequisites;
        generator.visualizePrerequisites(numCourses, prerequisites);
        
        Solution sol;
        bool result = sol.canFinish(numCourses, prerequisites);
        cout << "能否完成所有课程: " << (result ? "是" : "否") << endl;
        assert(result == true);
        cout << "✓ 测试通过" << endl << endl;
    }
    
    cout << "=== 测试4: 大规模测试 ===" << endl;
    {
        int numCourses = 1000;
        auto prerequisites = generator.generateAcyclicPrerequisites(numCourses, 0.1);
        
        Solution sol;
        bool result = sol.canFinish(numCourses, prerequisites);
        cout << "课程数量: " << numCourses << endl;
        cout << "依赖关系数量: " << prerequisites.size() << endl;
        cout << "能否完成所有课程: " << (result ? "是" : "否") << endl;
        assert(result == true);
        cout << "✓ 大规模测试通过" << endl << endl;
    }
    
    cout << "=== 测试5: 随机依赖关系 ===" << endl;
    {
        int numCourses = 8;
        auto prerequisites = generator.generateRandomPrerequisites(numCourses, 5, 15);
        generator.visualizePrerequisites(numCourses, prerequisites);
        
        Solution sol;
        bool result = sol.canFinish(numCourses, prerequisites);
        cout << "能否完成所有课程: " << (result ? "是" : "否") << endl;
        
        // 验证结果：使用简单的环检测
        vector<int> inDegree(numCourses, 0);
        for (const auto& pre : prerequisites) {
            inDegree[pre[0]]++;
        }
        
        // 简单验证：如果所有课程都有入度，则可能有环
        bool allHaveInDegree = true;
        for (int i = 0; i < numCourses; ++i) {
            if (inDegree[i] == 0) {
                allHaveInDegree = false;
                break;
            }
        }
        
        if (allHaveInDegree) {
            cout << "注意: 所有课程都有前置依赖，很可能存在环" << endl;
        }
        cout << "✓ 随机测试完成" << endl << endl;
    }
}

// 性能测试
void performanceTest() {
    TestCaseGenerator generator;
    Solution sol;
    
    cout << "=== 性能测试 ===" << endl;
    
    vector<int> testSizes = {100, 500, 1000, 2000};
    for (int numCourses : testSizes) {
        auto prerequisites = generator.generateAcyclicPrerequisites(numCourses, 0.05);
        
        auto start = chrono::high_resolution_clock::now();
        bool result = sol.canFinish(numCourses, prerequisites);
        auto end = chrono::high_resolution_clock::now();
        
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
        
        cout << "课程数: " << numCourses 
             << ", 边数: " << prerequisites.size()
             << ", 时间: " << duration.count() << "μs"
             << ", 结果: " << (result ? "通过" : "失败") << endl;
        
        assert(result == true);
    }
}

int main() {
    cout << "开始拓扑排序算法测试..." << endl << endl;
    
    try {
        runTests();
        performanceTest();
        cout << endl << "🎉 所有测试通过!" << endl;
    } catch (const exception& e) {
        cout << "❌ 测试失败: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
*/








/*
https://quick-bench.com/q/KMmAMihI0RDH1X6MPjsFx_oVHQA

#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <cassert>
#include <queue>
using namespace std;

class TestCaseGenerator {
private:
    mt19937 rng;
    
public:
    TestCaseGenerator() : rng(random_device{}()) {}
    
    // 生成无环的依赖关系（确保可以完成）
    vector<vector<int>> generateAcyclicPrerequisites(int numCourses, int edgeDensity = 0.3) {
        vector<vector<int>> prerequisites;
        
        // 生成一个有效的拓扑序列
        vector<int> order(numCourses);
        for (int i = 0; i < numCourses; ++i) {
            order[i] = i;
        }
        shuffle(order.begin(), order.end(), rng);
        
        // 根据拓扑序列生成边（只能从前面指向后面）
        int maxEdges = min(numCourses * (numCourses - 1) / 2, 
                          static_cast<int>(numCourses * numCourses * edgeDensity));
        
        for (int i = 0; i < numCourses && prerequisites.size() < maxEdges; ++i) {
            for (int j = i + 1; j < numCourses && prerequisites.size() < maxEdges; ++j) {
                // 50%概率添加这条边
                if (uniform_real_distribution<double>(0, 1)(rng) < 0.5) {
                    prerequisites.push_back({order[j], order[i]}); // order[i] → order[j]
                }
            }
        }
        
        // 打乱边的顺序
        shuffle(prerequisites.begin(), prerequisites.end(), rng);
        return prerequisites;
    }
    
    // 生成有环的依赖关系（确保不能完成）
    vector<vector<int>> generateCyclicPrerequisites(int numCourses) {
        vector<vector<int>> prerequisites;
        
        if (numCourses < 3) {
            // 对于小规模，创建一个简单环
            for (int i = 0; i < numCourses; ++i) {
                prerequisites.push_back({i, (i + 1) % numCourses});
            }
            return prerequisites;
        }
        
        // 创建一个环：0->1->2->...->(n-1)->0
        for (int i = 0; i < numCourses - 1; ++i) {
            prerequisites.push_back({i + 1, i});
        }
        prerequisites.push_back({0, numCourses - 1}); // 闭合环
        
        // 添加一些额外边增加复杂度
        for (int i = 0; i < numCourses / 2; ++i) {
            int u = uniform_int_distribution<int>(0, numCourses - 1)(rng);
            int v = uniform_int_distribution<int>(0, numCourses - 1)(rng);
            if (u != v) {
                prerequisites.push_back({v, u});
            }
        }
        
        shuffle(prerequisites.begin(), prerequisites.end(), rng);
        return prerequisites;
    }
    
    // 生成随机依赖关系（可能包含环）
    vector<vector<int>> generateRandomPrerequisites(int numCourses, int minEdges, int maxEdges) {
        vector<vector<int>> prerequisites;
        int numEdges = uniform_int_distribution<int>(minEdges, maxEdges)(rng);
        
        for (int i = 0; i < numEdges; ++i) {
            int u = uniform_int_distribution<int>(0, numCourses - 1)(rng);
            int v = uniform_int_distribution<int>(0, numCourses - 1)(rng);
            if (u != v) {
                prerequisites.push_back({u, v});
            }
        }
        
        return prerequisites;
    }
    
    // 可视化依赖关系
    void visualizePrerequisites(int numCourses, const vector<vector<int>>& prerequisites) {
        cout << "课程数量: " << numCourses << endl;
        cout << "依赖关系 (" << prerequisites.size() << " 条):" << endl;
        
        // 统计每个课程的入度和出度
        vector<int> inDegree(numCourses, 0), outDegree(numCourses, 0);
        for (const auto& pre : prerequisites) {
            outDegree[pre[1]]++;
            inDegree[pre[0]]++;
        }
        
        // 打印依赖关系图
        for (const auto& pre : prerequisites) {
            cout << "  " << pre[1] << " → " << pre[0] << endl;
        }
        
        // 打印统计信息
        cout << "\n度统计:" << endl;
        for (int i = 0; i < numCourses; ++i) {
            cout << "  课程" << i << ": 入度=" << inDegree[i] << ", 出度=" << outDegree[i] << endl;
        }
    }
};

// 递归DFS辅助函数
bool dfs(const vector<vector<int>>& graph, vector<int>& visited, int curr) {
    // 如果当前节点正在被访问，说明存在环
    if (visited[curr] == 1) {
        return false;
    }
    
    // 如果当前节点已经被完全访问过，无需再次访问
    if (visited[curr] == 2) {
        return true;
    }
    
    // 标记当前节点为"正在访问"
    visited[curr] = 1;
    
    // 访问所有邻接节点
    for (int next : graph[curr]) {
        if (!dfs(graph, visited, next)) {
            return false;  // 如果检测到环，则返回false
        }
    }
    
    // 标记当前节点为"已完成访问"
    visited[curr] = 2;
    
    return true;  // 当前节点及其所有后继节点都没有环
}

// 全局变量用于benchmark
static int numCourses = 1000;
static TestCaseGenerator generator;
static auto prerequisites = generator.generateRandomPrerequisites(numCourses, 500, 2000);

static void TopologicalSorting_CSR(benchmark::State& state) {
  for (auto _ : state) {
      // Step 1: 统计每个顶点的出度和入度
      vector<int> outDegree(numCourses, 0);
      vector<int> inDegree(numCourses, 0);
      
      // 统计每个顶点的出度和入度
      for (auto& pre : prerequisites) {
          ++outDegree[pre[1]];
          ++inDegree[pre[0]];
      }
      
      // Step 2: 构建CSR格式的邻接表
      vector<int>& offset = outDegree;
      
      for (int i = 1; i < numCourses; ++i) {
          offset[i] += offset[i - 1];
      }
      
      vector<int> edges(prerequisites.size());
      vector<int> currentPos = offset;
      
      for (auto& pre : prerequisites) {
          int u = pre[1];
          int v = pre[0];
          edges[--currentPos[u]] = v;
      }
      
      // Step 3: 拓扑排序 - Kahn算法
      vector<int>& queue = currentPos;
      int front = 0, rear = 0;
      
      for (int i = 0; i < numCourses; ++i) {
          if (inDegree[i] == 0) {
              queue[rear++] = i;
          }
      }
      
      int processed = 0;
      
      while (front < rear) {
          int u = queue[front++];
          ++processed;
          
          int start = (u == 0) ? 0 : offset[u - 1];
          int end = offset[u];
          
          for (int i = start; i < end; ++i) {
              int v = edges[i];
              if (--inDegree[v] == 0) {
                  queue[rear++] = v;
              }
          }
      }
      
      benchmark::DoNotOptimize(processed);
  }
}

BENCHMARK(TopologicalSorting_CSR);

static void TopologicalSorting_AdjacencyList(benchmark::State& state) {
  for (auto _ : state) {
        // 构建邻接表和入度数组
        vector<vector<int>> graph(numCourses);  // 邻接表
        vector<int> inDegree(numCourses, 0);    // 入度数组

        // 填充邻接表和入度数组
        for (auto& pre : prerequisites) {
            int course = pre[0];  // 当前课程
            int preCourse = pre[1];  // 先修课程
            graph[preCourse].push_back(course);  // 添加边：先修课程 -> 当前课程
            ++inDegree[course];  // 当前课程的入度加1
        }

        // 将所有入度为0的节点加入队列
        queue<int> q;
        for (int i = 0; i < numCourses; ++i) {
            if (inDegree[i] == 0) {
                q.push(i);
            }
        }

        // 拓扑排序
        int count = 0;  // 记录已访问的节点数
        while (!q.empty()) {
            int curr = q.front();
            q.pop();
            ++count;  // 已访问节点数加1
            
            // 遍历当前节点的所有邻接节点
            for (int next : graph[curr]) {
                --inDegree[next];  // 将邻接节点的入度减1
                // 如果入度变为0，则加入队列
                if (inDegree[next] == 0) {
                    q.push(next);
                }
            }
        }
        benchmark::DoNotOptimize(count);
  }
}

BENCHMARK(TopologicalSorting_AdjacencyList);

static void Recursion(benchmark::State& state) {
  for (auto _ : state) {
      // 构建邻接表
      vector<vector<int>> graph(numCourses);
      for (auto& pre : prerequisites) {
          graph[pre[1]].push_back(pre[0]);  // 添加边：先修课程 -> 当前课程
      }
      
      // 访问状态数组：0=未访问，1=正在访问，2=已完成访问
      vector<int> visited(numCourses, 0);
      
      bool canFinish = true;
      
      // 对每个未访问的节点进行DFS
      for (int i = 0; i < numCourses && canFinish; ++i) {
          if (visited[i] == 0) {
              canFinish = dfs(graph, visited, i);
          }
      }
      
      benchmark::DoNotOptimize(canFinish);
  }
}

BENCHMARK(Recursion);


*/