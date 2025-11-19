class Solution {
public:
    vector<int> findOrder(int numCourses, vector<vector<int>>& prerequisites) {
        vector<int> outDegree(numCourses + 1, 0);
        vector<int> inDegree(numCourses, 0);
        
        #pragma GCC unroll 8
        for (auto& pre : prerequisites) {
            ++outDegree[pre[1] + 1];
            ++inDegree[pre[0]];
        }
        
        vector<int>& offset = outDegree;
        #pragma GCC unroll 8
        for (int i = 0; i < numCourses; ++i) {
            offset[i + 1] += offset[i];
        }
        
        vector<int> edges(prerequisites.size());
        vector<int> currentPos = offset;
        
        #pragma GCC unroll 8
        for (auto& pre : prerequisites) {
            int u = pre[1];
            int v = pre[0];
            edges[currentPos[u]++] = v;
        }
        
        vector<int>& queue = currentPos;
        int front = 0, rear = 0;
        
        #pragma GCC unroll 8
        for (int i = 0; i < numCourses; ++i) {
            if (inDegree[i] == 0) queue[rear++] = i;
        }
        queue.pop_back();
        
        int processed = 0;
        vector<int> result;
        result.reserve(numCourses);
        while (front < rear) {
            int u = queue[front++];
            result.push_back(u);
            ++processed;
            
            #pragma GCC unroll 8
            for (int i = offset[u]; i < offset[u + 1]; ++i) {
                int v = edges[i];
                if (--inDegree[v] == 0) {
                    queue[rear++] = v; 
                }
            }
        }
        
        return processed == numCourses ? result : vector<int>{};
    }
};

/* 
 * @lc app=leetcode.cn id=210 lang=cpp 
 * 
 * [210] 课程表 II 
 */ 

// @lc code=start 
#include <vector>
#include <queue>
using namespace std;
/*
### 1. 邻接表 + BFS（广度优先搜索）
这是最直观的解法，使用拓扑排序来解决问题：

1. 构建邻接表表示课程依赖关系
2. 计算每个课程的入度（有多少先修课程）
3. 将所有入度为0的课程加入队列（这些课程可以直接学习）
4. 不断从队列中取出课程，将其加入结果数组，并减少其后续课程的入度
5. 如果最终结果数组的大小等于课程总数，说明可以完成所有课程，返回结果；否则返回空数组
### 2. 邻接表 + DFS（深度优先搜索）
DFS方法也可以用于拓扑排序：

1. 构建邻接表表示课程依赖关系
2. 使用DFS遍历图，同时检测是否有环
3. 在DFS的后序遍历位置记录节点
4. 最后将结果反转，得到正确的拓扑排序顺序
## 两种方法的比较
两种方法都是有效的解决方案，时间复杂度都是O(V+E)，空间复杂度也都是O(V+E)。

- BFS方法 ：更直观，易于理解，直接得到正确的拓扑排序顺序
- DFS方法 ：实现稍微复杂一些，需要额外的反转步骤，但在某些情况下可能更高效
*/
#define ADJACENCY_LIST_BFS
//#define ADJACENCY_LIST_DFS
class Solution { 
public: 
    vector<int> findOrder(int numCourses, vector<vector<int>>& prerequisites) {
#ifdef ADJACENCY_LIST_BFS
        // 邻接表实现（BFS拓扑排序）
        vector<vector<int>> graph(numCourses);  // 邻接表
        vector<int> inDegree(numCourses, 0);    // 入度数组
        vector<int> result;                     // 结果数组，存储拓扑排序顺序
        
        // 构建邻接表和计算入度
        for (auto& pre : prerequisites) {
            int course = pre[0];       // 当前课程
            int preCourse = pre[1];    // 先修课程
            graph[preCourse].push_back(course);  // 添加边：先修课程 -> 当前课程
            inDegree[course]++;        // 当前课程入度加1
        }
        
        // 将所有入度为0的节点加入队列
        queue<int> q;
        for (int i = 0; i < numCourses; i++) {
            if (inDegree[i] == 0) {
                q.push(i);
            }
        }
        
        // 拓扑排序
        while (!q.empty()) {
            int curr = q.front();
            q.pop();
            result.push_back(curr);  // 将当前节点加入结果
            
            // 遍历当前节点的所有邻接节点
            for (int next : graph[curr]) {
                inDegree[next]--;    // 将邻接节点的入度减1
                // 如果入度变为0，则加入队列
                if (inDegree[next] == 0) {
                    q.push(next);
                }
            }
        }
        
        // 如果结果数组的大小等于课程数，说明所有课程都可以完成
        // 否则返回空数组，表示无法完成所有课程
        return result.size() == numCourses ? result : vector<int>();
#elif defined(ADJACENCY_LIST_DFS)
        // 邻接表实现（DFS拓扑排序）
        vector<vector<int>> graph(numCourses);  // 邻接表
        vector<int> visited(numCourses, 0);     // 访问状态：0=未访问，1=正在访问，2=已完成访问
        vector<int> result;                     // 结果数组，存储拓扑排序顺序
        
        // 构建邻接表
        for (auto& pre : prerequisites) {
            int course = pre[0];       // 当前课程
            int preCourse = pre[1];    // 先修课程
            graph[preCourse].push_back(course);  // 添加边：先修课程 -> 当前课程
        }
        
        // 检查是否有环
        bool hasCycle = false;
        
        // 对每个未访问的节点进行DFS
        for (int i = 0; i < numCourses && !hasCycle; i++) {
            if (visited[i] == 0) {
                dfs(graph, visited, result, i, hasCycle);
            }
        }
        
        // 如果有环，返回空数组
        if (hasCycle) {
            return {};
        }
        
        // 反转结果数组，得到正确的拓扑排序顺序
        reverse(result.begin(), result.end());
        return result;
#endif
    }

private:
#ifdef ADJACENCY_LIST_DFS
    // DFS函数，用于拓扑排序
    void dfs(const vector<vector<int>>& graph, vector<int>& visited, vector<int>& result, 
             int curr, bool& hasCycle) {
        // 如果当前节点正在被访问，说明存在环
        if (visited[curr] == 1) {
            hasCycle = true;
            return;
        }
        
        // 如果当前节点已经被完全访问过，无需再次访问
        if (visited[curr] == 2) {
            return;
        }
        
        // 标记当前节点为"正在访问"
        visited[curr] = 1;
        
        // 访问所有邻接节点
        for (int next : graph[curr]) {
            dfs(graph, visited, result, next, hasCycle);
            if (hasCycle) {
                return;
            }
        }
        
        // 标记当前节点为"已完成访问"
        visited[curr] = 2;
        
        // 将当前节点加入结果（注意：这里是逆序的，最后需要反转）
        result.push_back(curr);
    }
#endif
}; 
// @lc code=end 