// 最优
class Solution {
public:
    bool exist(vector<vector<char>>& board, string word) {
        int ROWS( board.size() ), COLS( board.front().size() );
        if (word.length() > ROWS * COLS) return false;

        std::array<int, 52> count{};
        for (auto& rowVec : board)
            #pragma GCC unroll 8
            for (char c : rowVec) {
                std::isupper(c) ? ++count[c - 'A'] : ++count[c - 'a' + 26];
            }
            
        #pragma GCC unroll 8
        for (char c : word)
            if ((std::isupper(c) ? --count[c - 'A'] : --count[c - 'a' + 26]) < 0) return false;

        std::vector<uint8_t> visit(ROWS * COLS, false);
        for(int row = 0; row < ROWS; ++row) {
            #pragma GCC unroll 8
            for(int col = 0; col < COLS; ++col) {
                if (dfs(board, word, visit, row, col, ROWS, COLS, 0)) return true;
            }
        }
        return false;
    }

    bool dfs(auto& board, auto& word, auto& visit, int row, int col, int ROWS, int COLS, int start) {
        if (start == word.length()) return true;

        if (col >= COLS || col < 0 || row >= ROWS || row < 0 ||
            board[row][col] != word[start] || visit[row * COLS + col])
            return false;

        visit[row * COLS + col] = true;

        bool result = dfs(board, word, visit, row, col + 1, ROWS, COLS, start + 1)
            || dfs(board, word, visit, row, col - 1, ROWS, COLS, start + 1)
            || dfs(board, word, visit, row + 1, col, ROWS, COLS, start + 1)
            || dfs(board, word, visit, row - 1, col, ROWS, COLS, start + 1);
        
        visit[row * COLS + col] = false;

        return result;
    }
};


class Solution {
public:
    bool exist(vector<vector<char>>& board, string word) {
        int ROWS( board.size() ), COLS( board.front().size() );
        if (word.length() > ROWS * COLS) return false;

        std::array<int, 52> count{};
        for (auto& rowVec : board)
            #pragma GCC unroll 8
            for (char c : rowVec) {
                std::isupper(c) ? ++count[c - 'A'] : ++count[c - 'a' + 26];
            }

        #pragma GCC unroll 8
        for (char c : word)
            if ((std::isupper(c) ? --count[c - 'A'] : --count[c - 'a' + 26]) < 0) return false;

        std::vector<uint32_t> visit((ROWS * COLS + 31) >> 5, 0);
        for(int row = 0; row < ROWS; ++row) {
            #pragma GCC unroll 8
            for(int col = 0; col < COLS; ++col) {
                if (dfs(board, word, visit, row, col, ROWS, COLS, 0)) return true;
            }
        }
        return false;
    }

    bool dfs(auto& board, auto& word, auto& visit, int row, int col, int ROWS, int COLS, int start) {
        if (start == word.length()) return true;

        if (col >= COLS || col < 0 || row >= ROWS || row < 0 ||
            board[row][col] != word[start])
            return false;

        int pos = row * COLS + col;
        int idx = pos >> 5;  // pos / 32
        int bit = pos & 31;  // pos % 32
        
        if (visit[idx] & (1 << bit)) return false;
        
        visit[idx] |= (1 << bit);
        
        bool result = dfs(board, word, visit, row, col+1, ROWS, COLS, start+1) ||
                   dfs(board, word, visit, row, col-1, ROWS, COLS, start+1) ||
                   dfs(board, word, visit, row+1, col, ROWS, COLS, start+1) ||
                   dfs(board, word, visit, row-1, col, ROWS, COLS, start+1);
        
        visit[idx] &= ~(1 << bit);
        return result;
    }
};

class Solution {
public:
    bool exist(vector<vector<char>>& board, string word) {
        int ROWS( board.size() ), COLS( board.front().size() );
        std::vector<uint8_t> visit(ROWS * COLS, false);
        for(int row = 0; row < ROWS; ++row) {
            #pragma GCC unroll 8
            for(int col = 0; col < COLS; ++col) {
                if (dfs(board, word, visit, row, col, ROWS, COLS, 0)) return true;
            }
        }
        return false;
    }

    bool dfs(auto& board, auto& word, auto& visit, int row, int col, int ROWS, int COLS, int start) {
        if (start == word.length()) return true;
        // 边界检查、访问检查、字符匹配检查
        if (col >= COLS || col < 0 || row >= ROWS || row < 0 ||
            visit[row * COLS + col] ||  board[row][col] != word[start])
            return false;

        // 标记为已访问
        visit[row * COLS + col] = true;

        // 尝试四个方向
        bool result{false};
        result |= dfs(board, word, visit, row, col + 1, ROWS, COLS, start + 1);
        result |= dfs(board, word, visit, row, col - 1, ROWS, COLS, start + 1);
        result |= dfs(board, word, visit, row + 1, col, ROWS, COLS, start + 1);
        result |= dfs(board, word, visit, row - 1, col, ROWS, COLS, start + 1);
        
        // 回溯，取消访问标记
        visit[row * COLS + col] = false;
        return result;
    }
};
/* 
 * @lc app=leetcode.cn id=79 lang=cpp 
 * 
 * [79] 单词搜索 
 */ 

// @lc code=start 
#include <vector>
#include <string>
#include <unordered_map>
using namespace std;

class Solution { 
public: 
    bool exist(vector<vector<char>>& board, string word) {
        if (board.empty() || board[0].empty()) return false;
        if (word.empty()) return true;
        
        int m = board.size();
        int n = board[0].size();
        
        // 预处理：检查字符频率
        if (!checkCharFrequency(board, word, m, n)) {
            return false;
        }
        
        // 从每个单元格开始尝试搜索
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) {
                // 如果当前单元格的字符与单词的第一个字符匹配，开始DFS
                if (board[i][j] == word[0]) {
                    // 直接修改原数组来标记访问过的单元格
                    char temp = board[i][j];
                    board[i][j] = '#'; // 标记为已访问
                    
                    if (dfs(board, word, 1, i, j, m, n)) {
                        return true;
                    }
                    
                    // 恢复原值（回溯）
                    board[i][j] = temp;
                }
            }
        }
        
        return false;
    }
    
private:
    // 方向数组，表示上、右、下、左四个方向
    const int dx[4] = {-1, 0, 1, 0};
    const int dy[4] = {0, 1, 0, -1};
    
    // 检查字符频率是否满足要求
    bool checkCharFrequency(const vector<vector<char>>& board, const string& word, int m, int n) {
        // 统计单词中每个字符的出现次数
        unordered_map<char, int> wordCount;
        for (char c : word) {
            wordCount[c]++;
        }
        
        // 统计网格中每个字符的出现次数
        unordered_map<char, int> boardCount;
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) {
                boardCount[board[i][j]]++;
            }
        }
        
        // 检查每个字符的频率
        for (auto& [c, count] : wordCount) {
            if (boardCount[c] < count) {
                return false; // 网格中的字符不足以构成单词
            }
        }
        
        return true;
    }
    
    // DFS函数
    bool dfs(vector<vector<char>>& board, const string& word, int index, 
             int x, int y, int m, int n) {
        // 如果已经匹配到单词的最后一个字符，返回true
        if (index == word.size()) {
            return true;
        }
        
        // 尝试四个方向
        for (int i = 0; i < 4; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            
            // 检查新位置是否有效且字符匹配
            if (nx >= 0 && nx < m && ny >= 0 && ny < n && board[nx][ny] == word[index]) {
                // 标记为已访问
                char temp = board[nx][ny];
                board[nx][ny] = '#';
                
                // 递归搜索
                if (dfs(board, word, index + 1, nx, ny, m, n)) {
                    return true;
                }
                
                // 恢复原值（回溯）
                board[nx][ny] = temp;
            }
        }
        
        return false;
    }
}; 
// @lc code=end 