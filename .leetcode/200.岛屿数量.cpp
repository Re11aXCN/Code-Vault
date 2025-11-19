/*
 * @lc app=leetcode.cn id=200 lang=cpp
 *
 * [200] 岛屿数量
 */

// @lc code=start
class Solution {
public:
/*
1. 压缩法
    核心思想：将相邻的陆地压缩成单个点，最终统计剩余陆地的数量。
    问题：难以高效实现合并操作，可能需要多次遍历网格，时间复杂度较高。
    复杂度：实现不当可能导致 O(M²N²) 的时间复杂度。
2. 路径连通性检查（DFS/BFS）
    核心思想：通过DFS或BFS遍历所有相连的陆地，标记已访问区域。
    优势：每个单元格仅访问一次，时间复杂度为 O(M×N)。
    空间优化：通过修改原网格（将访问过的 '1' 改为 '0'），空间复杂度为 O(1)。
3. 并查集（Union-Find）
    核心思想：将相邻的陆地合并到同一集合，最终统计独立集合的数量。
    复杂度：时间复杂度接近 O(M×N)，但实现复杂，常数因子较大。
    适用场景：更适合动态连通性问题（如实时合并和查询）。
*/
    int numIslands(vector<vector<char>>& grid) {
        if (grid.empty()) return 0; // 处理空网格的情况
        
        int rows = grid.size();
        int cols = grid[0].size();
        int islandCount = 0;
        
        // 遍历网格中的每一个单元格
        for (int i = 0; i < rows; ++i) {
            #pragma GCC unroll 8
            for (int j = 0; j < cols; ++j) {
                // 发现未访问的陆地（'1'）
                if (grid[i][j] == '1') {
                    // 通过DFS标记整个岛屿
                    dfsMarking(grid, i, j, rows, cols);
                    ++islandCount; // 岛屿计数加1
                }
            }
        }
        return islandCount;
    }
    
private:
    // DFS辅助函数：标记当前岛屿的所有相连陆地
    void dfsMarking(vector<vector<char>>& grid, int i, int j, int rows, int cols) {
        // 边界检查或当前单元格为水域（'0'）
        if (i < 0 || i >= rows || j < 0 || j >= cols || grid[i][j] != '1') {
            return;
        }
        
        grid[i][j] = '0'; // 标记当前单元格为已访问
        
        // 递归遍历四个方向（上、下、左、右）
        dfsMarking(grid, i + 1, j, rows, cols); // 下方
        dfsMarking(grid, i - 1, j, rows, cols); // 上方
        dfsMarking(grid, i, j + 1, rows, cols); // 右方
        dfsMarking(grid, i, j - 1, rows, cols); // 左方
    }
};
// @lc code=end
using std::vector;

class UnionFind {
private:
    vector<int> parent;  // 存储每个节点的父节点
    vector<int> rank;    // 用于按秩合并的秩数组
    int count;           // 连通分量（岛屿）的数量
    
public:
    // 构造函数：初始化并查集
    UnionFind(vector<vector<char>>& grid) {
        int rows = grid.size();
        int cols = grid[0].size();
        count = 0;
        
        parent.resize(rows * cols);
        rank.resize(rows * cols, 0);
        
        // 初始化并查集
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                if (grid[i][j] == '1') {
                    int idx = i * cols + j;
                    parent[idx] = idx;  // 初始时每个陆地自成一个集合
                    ++count;            // 每个陆地都算作一个岛屿
                }
            }
        }
    }
    
    // 查找操作：带路径压缩
    int find(int x) {
        if (parent[x] != x) {
            parent[x] = find(parent[x]);  // 路径压缩
        }
        return parent[x];
    }
    
    // 合并操作：按秩合并
    void unionSet(int x, int y) {
        int rootX = find(x);
        int rootY = find(y);
        
        if (rootX != rootY) {
            // 按秩合并：将秩小的树合并到秩大的树下
            if (rank[rootX] > rank[rootY]) {
                parent[rootY] = rootX;
            } else if (rank[rootX] < rank[rootY]) {
                parent[rootX] = rootY;
            } else {
                parent[rootY] = rootX;
                rank[rootX] += 1;
            }
            --count;  // 合并后连通分量减少
        }
    }
    
    int getCount() const {
        return count;
    }
};

class Solution {
public:
    int numIslands(vector<vector<char>>& grid) {
        if (grid.empty()) return 0;
        
        int rows = grid.size();
        int cols = grid[0].size();
        
        UnionFind uf(grid);  // 初始化并查集
        
        // 定义四个方向：右、下
        vector<vector<int>> directions = {{0, 1}, {1, 0}};
        
        // 遍历网格，合并相邻的陆地
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                if (grid[i][j] == '1') {
                    // 将二维坐标转换为一维索引
                    int idx = i * cols + j;
                    
                    // 只检查右方和下方，避免重复合并
                    for (auto& dir : directions) {
                        int ni = i + dir[0];
                        int nj = j + dir[1];
                        
                        // 检查边界和是否为陆地
                        if (ni >= 0 && ni < rows && nj >= 0 && nj < cols && grid[ni][nj] == '1') {
                            int nidx = ni * cols + nj;
                            uf.unionSet(idx, nidx);  // 合并相邻陆地
                        }
                    }
                }
            }
        }
        
        return uf.getCount();  // 返回连通分量数量
    }
};
