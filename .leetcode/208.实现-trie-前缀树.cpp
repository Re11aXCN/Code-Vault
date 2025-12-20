#include <string>
#include <array>
/* 
 * @lc app=leetcode.cn id=208 lang=cpp 
 * 
 * [208] 实现 Trie (前缀树) 
 */ 

// @lc code=start


class Trie {
private:
    // Trie节点结构
    struct TrieNode {
        std::array<TrieNode *, 26> children{};  // 只考虑小写字母，使用数组而非map提高性能
        std::uint32_t strSize{ 0 };             // 记录经过该节点的字符串数量
        bool isEndOfWord{ false };                // 标记单词结束
    };
    
    TrieNode* _root; // 根节点

    void _deleteAllNodes(TrieNode *node)
    {
        if (node == nullptr) return;

        for (auto *child : node->children) {
            if (child != nullptr) _deleteAllNodes(child);
        }

        delete node;
    }

public:
    Trie() { _root = new TrieNode(); }
    ~Trie() { _deleteAllNodes(_root); _root = nullptr; }

    Trie(const Trie &) = delete;
    auto operator=(const Trie &) -> Trie & = delete;
    
    void insert(const std::string & word) {
        TrieNode *currentNode = _root;
        #pragma clang loop unroll_count(4)
        for (char chr : word)
        {
            // 获取字符索引
            int index = chr - 'a';

            // 如果没有子节点，则创建一个新节点
            if (currentNode->children[index] == nullptr)
            {
                currentNode->children[index] = new TrieNode();
            }
            // 移动到子节点
            currentNode = currentNode->children[index];
            /*
            Node*& child = currNode->children[index];
            if (child == nullptr) child = new Node{};
            currNode = child;
            */
            // 经过该节点的字符串数量增加
            ++currentNode->strSize;
        }
        // 标记为单词结尾
        currentNode->isEndOfWord = true;
    }
    
    bool search(const std::string & word) {
        TrieNode *currentNode = _root;
        #pragma clang loop unroll_count(4)
        for (char chr : word)
        {
            // 获取字符索引
            int index = chr - 'a';
            // 如果找不到对应的子节点，则字符串不存在
            if (currentNode->children[index] == nullptr)
            {
                return false;
            }
            // 移动到子节点
            currentNode = currentNode->children[index];
            /*
            if (currNode = currNode->children[chr - 'a']; currNode == nullptr) {
                return false;
            }
            */
        }
        // 返回是否为单词结尾
        return currentNode->isEndOfWord;
    }
    
    bool startsWith(const std::string & prefix) {
        TrieNode* node = _root;
        #pragma clang loop unroll_count(4)
        for (char c : prefix) {
            int index = c - 'a';
            if (!node->children[index]) {
                return false; // 路径不存在，前缀不在Trie中
            }
            node = node->children[index];
        }
        return true; // 前缀存在
    }

    bool remove(const std::string &word) 
    {
        // 先检查字符串是否存在
        if (search(word))
        {
            TrieNode *currentNode = _root;
            TrieNode *temp = nullptr;
            int index = 0;
            #pragma clang loop unroll_count(4)
            for (char chr : word)
            {
                // 获取字符索引
                index = chr - 'a';
                temp = currentNode;
                // 移动到子节点
                currentNode = currentNode->children[index];
                // 经过该节点的字符串数量减少
                currentNode->strSize--;
                if (currentNode->strSize == 0)
                {
                    // 删除无用的子树
                    _deleteAllNodes(currentNode);
                    // 清除子节点指针
                    temp->children[index] = nullptr;
                    // 成功删除
                    return true;
                }
            }
            // 标记为非单词结尾
            currentNode->isEndOfWord = false;
            // 成功删除
            return true;
        }
        // 未找到字符串
        return false;
    }
};

/**
 * Your Trie object will be instantiated and called as such:
 * Trie* obj = new Trie();
 * obj->insert(word);
 * bool param_2 = obj->search(word);
 * bool param_3 = obj->startsWith(prefix);
 */
// @lc code=end