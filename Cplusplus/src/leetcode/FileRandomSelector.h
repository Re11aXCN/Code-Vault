#pragma once

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// 历史记录条目结构
struct HistoryEntry
{
  std::string              timestamp;
  std::vector<std::string> files;
};

class FileRandomSelector
{
public:

  // 构造函数
  FileRandomSelector() = default;

  // 解析命令行参数
  bool ParseArgs(int argc, char* argv []);

  // 执行随机选择流程
  void Execute();

  // 获取当前可执行文件路径
  std::string GetCurrentExecutablePath() const;

private:

  // 获取当前目录下符合条件的文件列表
  std::vector<std::string> getFilteredFiles();

  // 计算总文件数和已选文件数
  std::pair<int, int> calculateFileCounts();

  // 从文件中读取历史记录
  bool loadHistory();

  // 保存历史记录到文件
  bool saveHistory();

  // 清空历史记录
  void clearHistory();

  // 随机选择文件
  std::vector<std::string>
  randomSelectFiles(const std::vector<std::string>& files);

  // 解析过滤列表
  void parseFilterList(const std::string& filterStr);

  // 解析缓存文件头信息
  bool parseCacheHeader(const std::string& header);

  // 获取当前ISO8601时间戳
  std::string getCurrentTimestamp() const;

  // 成员变量
  std::string               _suffix;
  std::vector<std::string>  _filter_list;
  int                       _rand_num = 0;
  std::set<std::string>     _history_files;    // 用于快速查找的已选文件集合
  std::vector<HistoryEntry> _history_entries;  // 带时间戳的历史记录
  std::string               _cache_file   = "cache.txt";
  int                       _total_files  = 0;
  int                       _cached_files = 0;
};

/*
* int main(int argc, char* argv[]) {
    FileRandomSelector selector;

    if (!selector.ParseArgs(argc, argv)) {
        return 1;
    }

    selector.Execute();

    return 0;
}
用于 leetcode算法随机题目进行review的程序。
程序名字.exe --suffix=cpp --filter=[0.review.cpp, 2024.ICPC.Bridging the
gap.cpp] --rand-num=5
*/