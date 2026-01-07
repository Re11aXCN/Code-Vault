namespace std {
    template <size_t N>
    struct hash<array<int, N>> {
        size_t operator()(const array<int, N>& arr) const {
            hash<int> hasher;
            size_t seed = 0;
            #pragma clang loop unroll_count(4)
            for (int elem : arr) {
                seed ^= hasher(elem) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };
}
class Solution {
public:
    vector<vector<string>> groupAnagrams(vector<string>& strs) {
        int size = strs.size(), index = 0;
        std::vector<std::vector<std::string>> result;
        result.reserve(size >> 1);
        std::unordered_map<std::string, int> anagramMap;
        // std::unordered_map<std::array<int, 26>, int> anagramMap;
        anagramMap.reserve(size >> 1);

        for (auto& str : strs) {
            std::string key = str;
            std::sort(key.begin(), key.end());
            //std::array<int, 26> key{};
            //#pragma clang loop unroll_count(4)
            //for (char c : str) ++key[c - 'a'];

            if (auto findIter = anagramMap.find(key); findIter != anagramMap.end()) {
                result[findIter->second].push_back(str);
            }
            else {
                std::vector<std::string> anagrams; anagrams.reserve(4);
                anagrams.push_back(str);
                result.push_back(std::move(anagrams));
                //anagramMap.try_emplace(std::move(key), index++);
                anagramMap.try_emplace(key, index++);
            }
        }
        return result;
    }
};

//////////////////////
class Solution {
public:
    vector<vector<string>> groupAnagrams(vector<string>& strs) {
        std::unordered_map<std::string, int> groupMap; groupMap.reserve(strs.size() >> 1);
        std::vector<std::vector<std::string>> result; result.reserve(strs.size() >> 1);
        for (auto& str : strs) {
            if (int size = str.size(); size <= 16) {
                std::string key{ str };
                std::sort(key.begin(), key.end());
                auto it = groupMap.find(key);
                if (it != groupMap.end()) {
                    result[it->second].emplace_back(std::move(str));
                }
                else {
                    std::vector<std::string> anagrams; anagrams.reserve(size * size);
                    anagrams.push_back(std::move(str));
                    result.push_back(std::move(anagrams));
                    groupMap.try_emplace(std::move(key), result.size() - 1);
                }
            }
            else {
                std::string key(26, '0');
                #pragma GCC unroll 4
                for (char c : str) key[(c - 'a')] += 1;
                auto it = groupMap.find(key);
                if (it != groupMap.end()) {
                    result[it->second].emplace_back(std::move(str));
                }
                else {
                    std::vector<std::string> anagrams; anagrams.reserve(size * size);
                    anagrams.push_back(std::move(str));
                    result.push_back(std::move(anagrams));
                    groupMap.try_emplace(std::move(key), result.size() - 1);
                }
            }
        }
        return result;
    }

    vector<vector<string>> groupAnagrams(vector<string>& strs) {
        std::unordered_map<std::string, std::vector<std::string>> groupMap; groupMap.reserve(strs.size() >> 1);
        
        for (auto& str : strs) {
            if (int size = str.size(); size <= 16) {
                std::string key{ str };
                std::sort(key.begin(), key.end());
                auto& map = groupMap[std::move(key)];
                map.reserve(size * size);
                map.push_back(std::move(str));
            }
            else {
                std::string key(26, '0');
                #pragma GCC unroll 4
                for (char c : str) key[(c - 'a')] += 1;
                auto& map = groupMap[std::move(key)];
                map.reserve(size * size);
                map.push_back(std::move(str));
            }
        }
        
        std::vector<std::vector<std::string>> result; result.reserve(strs.size() >> 1);
        for (auto& [k, v] : groupMap) {
            result.push_back(std::move(v));
        }
        return result;
    }
};

/*
 * @lc app=leetcode.cn id=49 lang=cpp
 *
 * [49] 字母异位词分组
 */

// @lc code=start
#include <ranges>
#include <array>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
using namespace std;

class Solution {
public:
    vector<vector<string>> groupAnagrams(vector<string>& strs) {
        auto arrayHash = [fn = hash<int>{}](const array<int, 26>& arr) -> size_t {
            return accumulate(arr.begin(), arr.end(), 0u, 
                [&](size_t acc, int num) {
                    return (acc << 1) ^ fn(num);
                });
        };
        
        unordered_map<array<int, 26>, vector<string>, decltype(arrayHash)> groups(0, arrayHash);
        
        for (const string& str : strs) {
            array<int, 26> counts{};
            for (char c : str) {
                ++counts[c - 'a'];
            }
            groups[counts].push_back(str);
        }
        
        vector<vector<string>> result;
        result.reserve(groups.size());
        for (auto& [_, group] : groups) {
            result.push_back(move(group));
        }
        return result;
    }
};

class Solution {
public:
    std::vector<std::vector<std::string>> groupAnagrams(std::vector<std::string>& strs) {
        if(strs.empty()) return {};
        std::unordered_map<std::string, std::vector<std::string>> map;
        map.reserve(strs.size());
        int i = 0;
        for ( ; i + 15 < strs.size(); i += 16)
        {
            for (int j = 0; j < 16; ++j)
            {
                auto& ref = strs[i + j];
                std::string str(ref);
                std::sort(str.begin(), str.end());
                if(auto it = map.find(str); it != map.end())
                {
                    it->second.emplace_back(ref);
                }
                else
                {
                    std::vector<std::string> temp;
                    temp.reserve(16);
                    temp.emplace_back(ref);
                    map.try_emplace(std::move(str), std::move(temp));
                }
            }
        }
        for ( ; i < strs.size(); ++i)
        {
            auto& ref = strs[i];
            std::string str(ref);
            std::sort(str.begin(), str.end());
            if(auto it = map.find(str); it != map.end())
            {
                it->second.emplace_back(ref);
            }
            else
            {
                std::vector<std::string> temp;
                temp.reserve(16);
                temp.emplace_back(ref);
                map.try_emplace(std::move(str), std::move(temp));
            }
        }

        std::vector<std::vector<std::string>> res;
        res.reserve(map.size());

        for(auto & [k, v] : map)
        {
            res.emplace_back(std::move(v));
        }
        return res;
    }
};
class Solution {
public:
    bool is_anagram(const string& s, const string& t) {
        if (s.size() != t.size()) return false;
        array<uint8_t, 26> counts{};
        for (unsigned char c : s) counts[c - 'a']++;
        for (unsigned char c : t) if (--counts[c - 'a'] < 0) return false; // 提前终止
        return true;
    }

    /*
    * hash，统计给定字符串数组内的所有anagram组
    */
    // 策略：排序（生成顺序唯一）或者根据字符出现次数进行统计（统计的序列唯一）
    /*
    * 排序法：
    * 步骤：
    * 1、循环获取每个str，如何排序，使用hashmap存储排序的值作为key。使用没有排序的值作为value插入数组
    * 统计法：
    * 1、使用array 26统计str的字母出现次数，然后构造一个唯一字符串，作为key返回，使用原str作为value插入数组
    * 
    * 2、循环去除hashmap的value数组拼接返回结果
    */
    vector<vector<string>> groupAnagrams(vector<string>& strs) {
        constexpr size_t SORT_THRESHOLD = 15; 
        auto generateCountKey = [](const string& s) {
            array<uint8_t, 26> counts{}; // 26个小写字母的计数
            for (unsigned char c : s) {  // 处理带符号char的溢出
                ++counts[c - 'a'];
            }
            // 将计数转换为类似 "2#0#3#..." 的唯一字符串
            string key;
            key.reserve(26 * 2); // 预分配空间优化
            for (auto cnt : counts) {
                key += 'a' + count;
                key += '|'; // 分隔避免不同计数组合碰撞
            }
            return key;
        };
        unordered_map<string, vector<string>> anagramGroups;
        for (auto&& str : strs) {
            string key;
            // 根据长度选择排序法或计数法生成key
            if (str.size() <= SORT_THRESHOLD) {
                key = str;
                ranges::sort(key); // C++20 ranges排序
            } else {
                key = generateCountKey(str);
            }
            // 移动语义减少拷贝
            anagramGroups[move(key)].push_back(move(str));
        }
        
        vector<vector<string>> result;
        result.reserve(anagramGroups.size());
        
        // 使用ranges::views::values直接提取哈希表的值（C++23）
        auto valuesView = anagramGroups | views::values;
        for (auto&& group : valuesView) {
            result.push_back(move(group));
        }
        
        return result;
    }
};
// @lc code=end

class Solution {
    // 哈希混合函数 - 基于Boost的实现
    static std::size_t hash_mix(std::size_t v) {
        if constexpr (sizeof(std::size_t) == 8) {
            // 64位版本
            std::uint64_t m = 0xe9846af9b1a615dULL;
            std::uint64_t x = v;
            
            x ^= x >> 32;
            x *= m;
            x ^= x >> 32;
            x *= m;
            x ^= x >> 28;
            
            return static_cast<std::size_t>(x);
        } else {
            // 32位版本
            std::uint32_t m1 = 0x21f0aaad;
            std::uint32_t m2 = 0x735a2d97;
            std::uint32_t x = v;
            
            x ^= x >> 16;
            x *= m1;
            x ^= x >> 15;
            x *= m2;
            x ^= x >> 15;
            
            return static_cast<std::size_t>(x);
        }
    }

    // 字符串哈希函数 - 基于Boost的hash_range实现
    struct string_hash {
        std::size_t operator()(const std::string& str) const {
            const char* data = str.data();
            std::size_t size = str.size();
            std::size_t seed = 0;
            
            if constexpr (sizeof(std::size_t) <= 4) {
                // 32位或更小平台
                return hash_range_32(seed, data, data + size);
            } else {
                // 64位平台
                return hash_range_64(seed, data, data + size);
            }
        }
        
    private:
        // 32位版本的hash_range
        static std::size_t hash_range_32(std::size_t seed, const char* first, const char* last) {
            const char* p = first;
            std::size_t n = static_cast<std::size_t>(last - first);
            
            std::uint32_t q = 0x9e3779b9U;
            std::uint32_t k = 0xe35e67b1U; // q * q
            
            std::uint64_t h = mul32(static_cast<std::uint32_t>(seed) + q, k);
            std::uint32_t w = static_cast<std::uint32_t>(h & 0xFFFFFFFF);
            
            h ^= n;
            
            while (n >= 4) {
                std::uint32_t v1 = read32le(p);
                w += q;
                h ^= mul32(v1 + w, k);
                p += 4;
                n -= 4;
            }
            
            std::uint32_t v1 = 0;
            if (n >= 1) {
                std::size_t x1 = (n - 1) & 2;
                std::size_t x2 = n >> 1;
                v1 = static_cast<std::uint32_t>(static_cast<unsigned char>(p[x1])) << (x1 * 8) |
                    static_cast<std::uint32_t>(static_cast<unsigned char>(p[x2])) << (x2 * 8) |
                    static_cast<std::uint32_t>(static_cast<unsigned char>(p[0]));
            }
            
            w += q;
            h ^= mul32(v1 + w, k);
            w += q;
            h ^= mul32(static_cast<std::uint32_t>(h & 0xFFFFFFFF) + w, 
                    static_cast<std::uint32_t>(h >> 32) + w + k);
            
            return static_cast<std::uint32_t>(h & 0xFFFFFFFF) ^ static_cast<std::uint32_t>(h >> 32);
        }
        
        // 64位版本的hash_range
        static std::size_t hash_range_64(std::size_t seed, const char* first, const char* last) {
            const char* p = first;
            std::size_t n = static_cast<std::size_t>(last - first);
            
            std::uint64_t q = 0x9e3779b97f4a7c15ULL;
            std::uint64_t k = 0xdf442d22ce4859b9ULL; // q * q
            
            std::uint64_t w = mulx(seed + q, k);
            std::uint64_t h = w ^ n;
            
            while (n >= 8) {
                std::uint64_t v1 = read64le(p);
                w += q;
                h ^= mulx(v1 + w, k);
                p += 8;
                n -= 8;
            }
            
            std::uint64_t v1 = 0;
            if (n >= 4) {
                v1 = static_cast<std::uint64_t>(read32le(p + n - 4)) << ((n - 4) * 8) | read32le(p);
            } else if (n >= 1) {
                std::size_t x1 = (n - 1) & 2;
                std::size_t x2 = n >> 1;
                v1 = static_cast<std::uint64_t>(static_cast<unsigned char>(p[x1])) << (x1 * 8) |
                    static_cast<std::uint64_t>(static_cast<unsigned char>(p[x2])) << (x2 * 8) |
                    static_cast<std::uint64_t>(static_cast<unsigned char>(p[0]));
            }
            
            w += q;
            h ^= mulx(v1 + w, k);
            
            return mulx(h + w, k);
        }
        
        // 辅助函数 - 32位乘法
        static std::uint64_t mul32(std::uint32_t x, std::uint32_t y) {
            return static_cast<std::uint64_t>(x) * y;
        }
        
        // 辅助函数 - 64位乘法 (简化版)
        static std::uint64_t mulx(std::uint64_t x, std::uint64_t y) {
            // 简化的64位乘法混合
            // 在实际实现中可能需要更复杂的处理
            return x * y + (x >> 32) * (y >> 32);
        }
        
        // 读取小端32位值
        static std::uint32_t read32le(const char* p) {
            return static_cast<std::uint32_t>(static_cast<unsigned char>(p[0])) |
                static_cast<std::uint32_t>(static_cast<unsigned char>(p[1])) << 8 |
                static_cast<std::uint32_t>(static_cast<unsigned char>(p[2])) << 16 |
                static_cast<std::uint32_t>(static_cast<unsigned char>(p[3])) << 24;
        }
        
        // 读取小端64位值
        static std::uint64_t read64le(const char* p) {
            return static_cast<std::uint64_t>(static_cast<unsigned char>(p[0])) |
                static_cast<std::uint64_t>(static_cast<unsigned char>(p[1])) << 8 |
                static_cast<std::uint64_t>(static_cast<unsigned char>(p[2])) << 16 |
                static_cast<std::uint64_t>(static_cast<unsigned char>(p[3])) << 24 |
                static_cast<std::uint64_t>(static_cast<unsigned char>(p[4])) << 32 |
                static_cast<std::uint64_t>(static_cast<unsigned char>(p[5])) << 40 |
                static_cast<std::uint64_t>(static_cast<unsigned char>(p[6])) << 48 |
                static_cast<std::uint64_t>(static_cast<unsigned char>(p[7])) << 56;
        }
    };
public:
    std::vector<std::vector<std::string>> groupAnagrams(std::vector<std::string>& strs) {
        if (strs.empty()) return {};
        
        std::unordered_map<std::string, std::vector<std::string>, string_hash> map;
        map.reserve(strs.size() * 1.5);
        
        const size_t n = strs.size();
        
        for (size_t i = 0; i < n; ++i) {
            const std::string& s = strs[i];
            int count[26] = {0};
            
            const char* data = s.data();
            const size_t len = s.size();
            
            size_t j = 0;
            for (; j + 7 < len; j += 8) {
                count[data[j] - 'a']++;
                count[data[j + 1] - 'a']++;
                count[data[j + 2] - 'a']++;
                count[data[j + 3] - 'a']++;
                count[data[j + 4] - 'a']++;
                count[data[j + 5] - 'a']++;
                count[data[j + 6] - 'a']++;
                count[data[j + 7] - 'a']++;
            }
            
            for (; j < len; ++j) {
                count[data[j] - 'a']++;
            }
            
            char key[78];
            char* key_ptr = key;
            for (int k = 0; k < 26; ++k) {
                *key_ptr++ = '#';
                int num = count[k];
                
                if (num < 10) {
                    *key_ptr++ = '0' + num;
                } else {
                    *key_ptr++ = '0' + (num / 10);
                    *key_ptr++ = '0' + (num % 10);
                }
            }
            *key_ptr = '\0';
            
            map[std::string(key, key_ptr - key)].push_back(s);
        }
        
        std::vector<std::vector<std::string>> result;
        result.reserve(map.size());
        for (auto it = map.begin(); it != map.end(); ++it) {
            result.emplace_back(move(it->second));
        }
        
        return result;
    }
};
/*




*/

#include <immintrin.h>

class Solution {
public:
    std::vector<std::vector<std::string>> groupAnagrams(std::vector<std::string>& strs) {
        constexpr size_t SORT_THRESHOLD = 15; 

        auto generateCountKey = [](const std::string& s) static -> std::string {
            std::array<uint8_t, 26> counts{};
            
            // 使用SIMD处理主循环（每次处理16个字符）
            const size_t len = s.size();
            size_t i = 0;
            
            if (len >= 16) {
                // 创建26个计数器的SIMD向量（每个计数器是32位以支持更大数值）
                alignas(16) std::array<__m128i, 26> simd_counts{};
                for (auto& sc : simd_counts) {
                    sc = _mm_setzero_si128();
                }
                
                // 主SIMD循环
                for (; i + 15 < len; i += 16) {
                    // 加载16个字符
                    __m128i chars = _mm_loadu_si128(
                        reinterpret_cast<const __m128i*>(s.data() + i));
                    
                    // 为每个字母更新计数器
                    for (int letter = 0; letter < 26; ++letter) {
                        __m128i target = _mm_set1_epi8('a' + letter);
                        __m128i mask = _mm_cmpeq_epi8(chars, target);
                        __m128i increment = _mm_and_si128(mask, _mm_set1_epi8(1));
                        
                        // 将8位累加扩展到32位
                        __m128i sum8 = _mm_sad_epu8(increment, _mm_setzero_si128());
                        simd_counts[letter] = _mm_add_epi32(simd_counts[letter], sum8);
                    }
                }
                
                // 合并SIMD计数结果
                alignas(16) std::array<uint32_t, 4> temp{};
                for (int letter = 0; letter < 26; ++letter) {
                    _mm_store_si128(reinterpret_cast<__m128i*>(temp.data()), simd_counts[letter]);
                    counts[letter] += temp[0] + temp[2];  // sad结果在0和2位置
                }
            }
            
            // 处理剩余字符（标量处理）
            for (; i < len; ++i) {
                unsigned char c = s[i];
                if (c >= 'a' && c <= 'z') {
                    ++counts[c - 'a'];
                }
            }
            
            // 生成key - 使用循环展开和std::to_chars
            std::string key;
            char buffer[26 * 8];  // 每个数字最多4位 + '#' + 安全边界
            char* ptr = buffer;
            
            // 循环展开4次处理26个计数器（24次主循环 + 2次剩余）
            for (int j = 0; j < 24; j += 4) {
                // 第1组
                auto result1 = std::to_chars(ptr, ptr + 8, counts[j]);
                ptr = result1.ptr;
                *ptr++ = '#';
                
                // 第2组  
                auto result2 = std::to_chars(ptr, ptr + 8, counts[j + 1]);
                ptr = result2.ptr;
                *ptr++ = '#';
                
                // 第3组
                auto result3 = std::to_chars(ptr, ptr + 8, counts[j + 2]);
                ptr = result3.ptr;
                *ptr++ = '#';
                
                // 第4组
                auto result4 = std::to_chars(ptr, ptr + 8, counts[j + 3]);
                ptr = result4.ptr;
                *ptr++ = '#';
            }
            
            // 处理最后2个计数器
            auto result25 = std::to_chars(ptr, ptr + 8, counts[24]);
            ptr = result25.ptr;
            *ptr++ = '#';
            
            auto result26 = std::to_chars(ptr, ptr + 8, counts[25]);
            ptr = result26.ptr;
            *ptr++ = '#';
            
            key.assign(buffer, ptr - buffer - 1);  // 去掉最后一个'#'
            return key;
        };
        std::unordered_map<std::string, std::vector<std::string>> anagramGroups;
        size_t n = strs.size();
        size_t i = 0;

        // 辅助函数处理单个字符串
        auto processString = [&](size_t index) {
            std::string key;
            if (strs[index].size() <= SORT_THRESHOLD) {
                key = strs[index];
                std::ranges::sort(key);
            } else {
                key = generateCountKey(strs[index]);
            }
            anagramGroups[std::move(key)].push_back(std::move(strs[index]));
        };

        // 每次处理4个元素
        for (; i + 3 < n; i += 4) {
            processString(i);
            processString(i+1);
            processString(i+2);
            processString(i+3);
        }

        // 处理剩余元素
        for (; i < n; ++i) {
            processString(i);
        }
        
        std::vector<std::vector<std::string>> result;
        result.reserve(anagramGroups.size());
        for (auto&& [k, v] : anagramGroups) {
            result.emplace_back(std::move(v));
        }
        return result;
    }
};

