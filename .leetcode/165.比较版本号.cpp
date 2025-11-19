/*
 * @lc app=leetcode.cn id=165 lang=cpp
 *
 * [165] 比较版本号
 */
// 存储小数点然后视图比较
// or转换为int
// @lc code=start
class Solution {
public:
// 01.001.10.0.1.1
// 1.1.2

// 1.0.0.1
// 1.0
    int compareVersion(string version1, string version2) {
        version1.push_back('.'), version2.push_back('.');
        int num1{0}, num2{0}, len1{0}, len2{0}, i{0}, j{0};
        while(i < version1.size() && j < version2.size())
        {
            if((version1[i] == '.' && version2[j] == '.')) {
                std::from_chars(version1.data() + len1, version1.data() + i, num1);
                std::from_chars(version2.data() + len2, version1.data() + j, num2);
                if(num1 < num2) return num1 < num2 ? -1 : 1;
                else if (num1 > num2) return 1;
                ++i, ++j;
                len1 = i, len2 = j;
                continue;
            }
            if(i != version1.size() && version1[i] != '.') ++i;
            if(j != version2.size() && version2[j] != '.') ++j;
        }
        if(version2.size() == j) {
            for( ; i < version1.size(); ++i) if(version1[i] >= '1' && version1[i] <= '9') return 1;
        }
        else {
            for( ; j < version2.size(); ++j) if(version2[j] >= '1' && version2[j] <= '9') return -1;
        }

        return 0;
    }
};
// @lc code=end

