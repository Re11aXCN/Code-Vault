/* 
 * @lc app=leetcode.cn id=51 lang=cpp 
 * 
 * [51] N 皇后 
 */ 

// @lc code=start 
#include <vector>
#include <string>
#include <stack>
#include <random>
#include <cmath>
#include <algorithm>
using namespace std;
class Solution {
#ifdef Ant_Colony_Optimization
public:
    vector<vector<string>> solveNQueens(int n) {
        // 蚁群算法参数
        const int ANT_COUNT = 10;        // 蚂蚁数量
        const int MAX_ITERATIONS = 100;  // 最大迭代次数
        const double ALPHA = 1.0;        // 信息素重要程度
        const double BETA = 2.0;         // 启发式信息重要程度
        const double RHO = 0.5;          // 信息素蒸发率
        const double Q = 100.0;          // 信息素增加强度

        // 初始化信息素矩阵
        vector<vector<double>> pheromone(n, vector<double>(n, 1.0));

        // 启发式信息矩阵（固定值，表示位置的吸引力）
        vector<vector<double>> heuristic(n, vector<double>(n, 1.0));

        // 最佳解
        vector<int> bestSolution;
        int bestConflicts = n * n;

        // 随机数生成器
        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<> dis(0.0, 1.0);

        // 迭代优化
        for (int iter = 0; iter < MAX_ITERATIONS; iter++) {
            // 每只蚂蚁构建一个解
            for (int ant = 0; ant < ANT_COUNT; ant++) {
                vector<int> solution(n, -1);  // solution[row] = col

                // 为每一行选择一列放置皇后
                for (int row = 0; row < n; row++) {
                    vector<double> probability(n, 0.0);
                    double sum = 0.0;

                    // 计算每列的选择概率
                    for (int col = 0; col < n; col++) {
                        // 检查是否与已放置的皇后冲突
                        bool conflict = false;
                        for (int prevRow = 0; prevRow < row; prevRow++) {
                            if (solution[prevRow] == col ||
                                solution[prevRow] - prevRow == col - row ||
                                solution[prevRow] + prevRow == col + row) {
                                conflict = true;
                                break;
                            }
                        }

                        if (!conflict) {
                            // 计算选择概率
                            probability[col] = pow(pheromone[row][col], ALPHA) *
                                pow(heuristic[row][col], BETA);
                            sum += probability[col];
                        }
                    }

                    // 如果所有列都有冲突，随机选择一列
                    if (sum == 0.0) {
                        solution[row] = gen() % n;
                    }
                    else {
                        // 根据概率选择一列
                        double r = dis(gen) * sum;
                        double cumulativeProbability = 0.0;
                        for (int col = 0; col < n; col++) {
                            cumulativeProbability += probability[col];
                            if (r <= cumulativeProbability) {
                                solution[row] = col;
                                break;
                            }
                        }
                    }
                }

                // 计算解的冲突数
                int conflicts = calculateConflicts(solution, n);

                // 更新最佳解
                if (conflicts < bestConflicts) {
                    bestConflicts = conflicts;
                    bestSolution = solution;

                    // 如果找到无冲突的解，提前结束
                    if (conflicts == 0) {
                        break;
                    }
                }

                // 更新信息素
                double deltaPheromone = conflicts == 0 ? Q : Q / (conflicts + 1);
                for (int row = 0; row < n; row++) {
                    pheromone[row][solution[row]] += deltaPheromone;
                }
            }

            // 如果找到无冲突的解，提前结束
            if (bestConflicts == 0) {
                break;
            }

            // 信息素蒸发
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    pheromone[i][j] *= (1.0 - RHO);
                    if (pheromone[i][j] < 0.1) pheromone[i][j] = 0.1;
                }
            }
        }

        // 如果没有找到解，返回空结果
        if (bestConflicts > 0) {
            return {};
        }

        // 将最佳解转换为题目要求的格式
        return convertToResult(bestSolution, n);
    }

private:
    // 计算解的冲突数
    int calculateConflicts(const vector<int>& solution, int n) {
        int conflicts = 0;
        for (int i = 0; i < n; i++) {
            for (int j = i + 1; j < n; j++) {
                // 检查是否在同一列或同一对角线上
                if (solution[i] == solution[j] ||
                    abs(solution[i] - solution[j]) == abs(i - j)) {
                    conflicts++;
                }
            }
        }
        return conflicts;
    }

    // 将解转换为题目要求的格式
    vector<vector<string>> convertToResult(const vector<int>& solution, int n) {
        vector<vector<string>> result;
        vector<string> board(n, string(n, '.'));

        for (int row = 0; row < n; row++) {
            board[row][solution[row]] = 'Q';
        }

        result.push_back(board);
        return result;
    }
#elif defined(NSCR)
public:
    vector<vector<string>> solveNQueens(int n) {
        vector<vector<string>> res;
        if (n == 0) return res;

        // 初始化数据结构
        vector<int> Qy(n, 0);          // 每行皇后所在的列
        vector<int> ptrconflict(n, 0); // 冲突指针数组
        vector<bool> isPlaced(n, false); // 标记是否已放置皇后
        int ptr_move = 0;              // 当前处理的皇后索引
        int ptr_place = 0;             // 已放置的皇后数量
        int conflict = -1;             // 冲突状态

        // 初始化皇后位置
        auto InitQueens = [&]() {
            fill(Qy.begin(), Qy.end(), 0);
            fill(ptrconflict.begin(), ptrconflict.end(), 0);
            fill(isPlaced.begin(), isPlaced.end(), false);
            if (n == 6) Qy[0] = 2;     // 特殊处理n=6的情况
        };

        // 检查皇后qm的冲突情况
        auto CheckThreat = [&](int qm) {
            int threat_ctr = 0;
            int enemy_ptr = -1;
            for (int i = 0; i < n; ++i) {
                if (i == qm || !isPlaced[i]) continue;
                if (Qy[i] == Qy[qm] || 
                    (i - Qy[i] == qm - Qy[qm]) || 
                    (i + Qy[i] == qm + Qy[qm])) {
                    enemy_ptr = i;
                    if (++threat_ctr > 1) {
                        enemy_ptr = -2; // 多冲突标记
                        break;
                    }
                }
            }
            return threat_ctr > 1 ? -2 : enemy_ptr;
        };

        // 移动皇后qm寻找新位置
        auto MoveDown = [&](int qm) {
            int home = Qy[qm];
            int mconflict = -1;
            for (int i = 0; i < n; ++i) {
                Qy[qm] = (Qy[qm] + 1) % n;
                if (Qy[qm] == 0) isPlaced[qm] = true;

                mconflict = CheckThreat(qm);
                if (mconflict >= 0 && mconflict != ptrconflict[qm]) {
                    ptrconflict[qm] = mconflict;
                    ptrconflict[mconflict] = qm;
                    break;
                }
                if (mconflict == -1) {
                    isPlaced[qm] = true;
                    break;
                }
                if (Qy[qm] == home) {
                    isPlaced[qm] = true;
                    break;
                }
            }
            return mconflict;
        };

        InitQueens();

        while (ptr_place < n || conflict != -1) {
            conflict = CheckThreat(ptr_move);

            if (conflict >= 0) {
                ptrconflict[ptr_move] = conflict;
                ptrconflict[conflict] = ptr_move;
                if (isPlaced[ptr_move]) {
                    ptr_move = conflict;
                }
                conflict = MoveDown(ptr_move);
            } else if (conflict == -1) {
                isPlaced[ptr_move] = true;
                ptr_place++;
                if (ptr_place < n) {
                    ptr_move = ptr_place;
                }
            } else if (conflict == -2) {
                conflict = MoveDown(ptr_move);
            }

            // 当所有皇后放置完毕且无冲突时生成解
            if (ptr_place == n && conflict == -1) {
                vector<string> board(n, string(n, '.'));
                for (int i = 0; i < n; ++i) {
                    board[i][Qy[i]] = 'Q';
                }
                res.push_back(board);
                break; // 若需要所有解可调整此处
            }
        }

        return res;
    }
#else
public:
    vector<vector<string>> solveNQueens(int n) {
        vector<vector<string>> res;
        vector<int> path(n, -1);  // 记录每行皇后所在的列，初始为-1表示未放置
        stack<pair<int, int>> stk;
        stk.push({ 0, 0 });  // 初始状态：处理第0行第0列

        while (!stk.empty()) {
            auto [row, col] = stk.top();
            stk.pop();

            // 如果当前列超出范围，跳过
            if (col >= n) {
                continue;
            }

            // 检查当前位置(row, col)是否有效
            bool valid = true;
            for (int i = 0; i < row; ++i) {
                if (path[i] == col || abs(row - i) == abs(col - path[i])) {
                    valid = false;
                    break;
                }
            }

            if (valid) {
                path[row] = col;  // 放置皇后

                if (row == n - 1) {  // 找到一个可行解
                    vector<string> board(n, string(n, '.'));
                    for (int i = 0; i < n; ++i) {
                        board[i][path[i]] = 'Q';
                    }
                    res.push_back(board);
                    // 继续尝试当前行的下一列
                    stk.push({ row, col + 1 });
                }
                else {
                    // 当前行处理完毕后，继续处理下一行，同时保存当前行的下一列状态以便回溯
                    stk.push({ row, col + 1 });  // 先压入当前行的下一列，保证LIFO顺序
                    stk.push({ row + 1, 0 });    // 处理下一行的起始列
                }
            }
            else {
                // 当前位置无效，尝试当前行的下一列
                stk.push({ row, col + 1 });
            }
        }

        return res;
    }
#endif // Ant_Colony_Optimization

};
// @lc code=end