#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <regex>
#include <chrono>
#include <iomanip>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class LeetcodeContentParser {
private:
    std::string _current_date; // 当前日期，格式：YYYY-MM-DD

    // 私有方法：小写驼峰命名
    std::string _parseDate(const std::string& date_str) {
        if (date_str.find("小时前") != std::string::npos) {
            return _parseHoursAgo(date_str);
        }
        else if (date_str == "昨天") {
            return _parseYesterday();
        }
        else if (_isWeekday(date_str)) {
            return _parseWeekday(date_str);
        }
        else {
            return _parseExactDate(date_str);
        }
    }

    std::string _parseHoursAgo(const std::string& hours_str) {
        std::regex pattern(R"((\d+)\s*小时前)");
        std::smatch match;

        if (std::regex_search(hours_str, match, pattern) && match.size() > 1) {
            int hours = std::stoi(match[1]);

            // 解析当前日期
            std::tm tm = {};
            std::istringstream ss(_current_date);
            ss >> std::get_time(&tm, "%Y-%m-%d");

            auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
            tp -= std::chrono::hours(hours);

            auto new_time_t = std::chrono::system_clock::to_time_t(tp);
            std::tm new_tm = *std::localtime(&new_time_t);

            std::ostringstream result;
            result << std::put_time(&new_tm, "%Y-%m-%d");
            return result.str();
        }
        return _current_date; // 默认返回当前日期
    }

    std::string _parseYesterday() {
        std::tm tm = {};
        std::istringstream ss(_current_date);
        ss >> std::get_time(&tm, "%Y-%m-%d");

        auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
        tp -= std::chrono::hours(24);

        auto new_time_t = std::chrono::system_clock::to_time_t(tp);
        std::tm new_tm = *std::localtime(&new_time_t);

        std::ostringstream result;
        result << std::put_time(&new_tm, "%Y-%m-%d");
        return result.str();
    }

    bool _isWeekday(const std::string& str) {
        static const std::vector<std::string> weekdays = {
            "星期一", "星期二", "星期三", "星期四", "星期五", "星期六", "星期日"
        };

        for (const auto& weekday : weekdays) {
            if (str == weekday) {
                return true;
            }
        }
        return false;
    }

    std::string _parseWeekday(const std::string& weekday_str) {
        // 简化处理：计算相对于当前日期最近的指定星期几
        std::vector<std::string> weekdays = {
            "星期一", "星期二", "星期三", "星期四", "星期五", "星期六", "星期日"
        };

        // 找到目标星期几的索引
        int target_index = -1;
        for (size_t i = 0; i < weekdays.size(); ++i) {
            if (weekdays[i] == weekday_str) {
                target_index = i;
                break;
            }
        }

        if (target_index == -1) return _current_date;

        // 解析当前日期
        std::tm tm = {};
        std::istringstream ss(_current_date);
        ss >> std::get_time(&tm, "%Y-%m-%d");

        // 计算当前星期几 (0=周日, 1=周一, ..., 6=周六)
        auto current_time_t = std::mktime(&tm);
        std::tm current_tm = *std::localtime(&current_time_t);
        int current_weekday = current_tm.tm_wday;
        if (current_weekday == 0) current_weekday = 7; // 周日调整为7

        int target_weekday = target_index + 1; // 星期一到星期日对应1-7

        int days_diff = current_weekday - target_weekday;
        if (days_diff <= 0) {
            days_diff += 7;
        }

        auto tp = std::chrono::system_clock::from_time_t(current_time_t);
        tp -= std::chrono::hours(24 * days_diff);

        auto new_time_t = std::chrono::system_clock::to_time_t(tp);
        std::tm new_tm = *std::localtime(&new_time_t);

        std::ostringstream result;
        result << std::put_time(&new_tm, "%Y-%m-%d");
        return result.str();
    }

    std::string _parseExactDate(const std::string& date_str) {
        std::regex pattern(R"((\d{1,2})月(\d{1,2})日)");
        std::smatch match;

        if (std::regex_search(date_str, match, pattern) && match.size() > 2) {
            int month = std::stoi(match[1]);
            int day = std::stoi(match[2]);

            // 解析当前年份
            std::tm current_tm = {};
            std::istringstream ss(_current_date);
            ss >> std::get_time(&current_tm, "%Y-%m-%d");

            std::tm target_tm = {};
            target_tm.tm_year = current_tm.tm_year;
            target_tm.tm_mon = month - 1;
            target_tm.tm_mday = day;

            auto time_t_val = std::mktime(&target_tm);
            std::tm final_tm = *std::localtime(&time_t_val);

            std::ostringstream result;
            result << std::put_time(&final_tm, "%Y-%m-%d");
            return result.str();
        }

        return date_str; // 如果无法解析，返回原字符串
    }

    std::pair<int, std::string> _parseProblem(const std::string& problem_str) {
        std::regex pattern(R"((\d+)\.\s*(.+))");
        std::smatch match;

        if (std::regex_search(problem_str, match, pattern) && match.size() > 2) {
            int number = std::stoi(match[1]);
            std::string title = match[2];
            return { number, title };
        }

        return { 0, problem_str }; // 解析失败时的默认值
    }

    std::string _trim(const std::string& str) {
        size_t start = str.find_first_not_of(" \t\n\r");
        size_t end = str.find_last_not_of(" \t\n\r");
        if (start == std::string::npos) return "";
        return str.substr(start, end - start + 1);
    }

public:
    // 公有方法：大写命名
    LeetcodeContentParser(const std::string& current_date) : _current_date(current_date) {}

    struct LeetcodeRecord {
        std::string date;
        int number;
        std::string title;
        std::string level;
    };

    std::vector<LeetcodeRecord> ParseContent(const std::string& content) {
        std::vector<LeetcodeRecord> records;
        std::istringstream stream(content);
        std::string line;

        LeetcodeRecord current_record;
        bool has_date = false;
        bool has_problem = false;
        bool has_level = false;

        while (std::getline(stream, line)) {
            line = _trim(line);
            if (line.empty()) continue;

            // 检查是否是日期行
            if (line.find("小时前") != std::string::npos ||
                line == "昨天" ||
                _isWeekday(line) ||
                line.find("月") != std::string::npos && line.find("日") != std::string::npos) {

                // 如果已经有完整的记录，保存它
                if (has_date && has_problem && has_level) {
                    records.push_back(current_record);
                }

                // 开始新记录
                current_record = LeetcodeRecord();
                current_record.date = _parseDate(line);
                has_date = true;
                has_problem = false;
                has_level = false;

            }
            // 检查是否是题目行（包含数字和点）
            else if (std::regex_match(line, std::regex(R"(\d+\..+)"))) {
                auto [number, title] = _parseProblem(line);
                current_record.number = number;
                current_record.title = title;
                has_problem = true;
            }
            // 检查是否是难度行
            else if (line == "简单" || line == "中等" || line == "困难") {
                current_record.level = line;
                has_level = true;

                // 当收集到难度时，记录完成
                if (has_date && has_problem && has_level) {
                    records.push_back(current_record);
                    // 重置标志，但保持日期，因为下一个记录可能共享相同日期
                    has_problem = false;
                    has_level = false;
                }
            }
        }

        // 添加最后一个记录
        if (has_date && has_problem && has_level) {
            records.push_back(current_record);
        }

        return records;
    }

    json SerializeToJson(const std::vector<LeetcodeRecord>& records) {
        json result = json::array();

        for (const auto& record : records) {
            json item;
            item["date"] = record.date;
            item["number"] = record.number;
            item["title"] = record.title;
            item["level"] = record.level;
            result.push_back(item);
        }

        return result;
    }

    void SetCurrentDate(const std::string& current_date) {
        _current_date = current_date;
    }

    std::string GetCurrentDate() const {
        return _current_date;
    }
};

// 使用示例
int main() {
    std::string leetcode_content = R"(
1 小时前
647. 回文子串
中等
通过
15
2 小时前
583. 两个字符串的删除操作
中等
通过
5
2 小时前
72. 编辑距离
中等
通过
18
昨天
1035. 不相交的线
中等
通过
7
星期三
188. 买卖股票的最佳时机 IV
困难
通过
2
11月9日
70. 爬楼梯
简单
通过
8
    )";

    // 创建解析器，设置当前日期为2025-11-14
    LeetcodeContentParser parser("2025-11-14");

    // 解析内容
    auto records = parser.ParseContent(leetcode_content);

    // 序列化为JSON
    auto json_data = parser.SerializeToJson(records);

    // 输出结果
    std::cout << json_data.dump(4) << std::endl;

    return 0;
}