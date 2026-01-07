#include "FileRandomSelector.h"
#ifdef _WIN32
#  include <windows.h>
#else
#  include <unistd.h>
#endif

bool FileRandomSelector::ParseArgs(int argc, char* argv [])
{
  if (argc < 2) {
    std::cerr << "Usage: " << argv [0]
              << " --suffix=ext --filter=[file1,file2] --rand-num=N"
              << std::endl;
    return false;
  }

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv [i];

    if (arg.find("--suffix=") == 0) {
      _suffix     = arg.substr(9);
      _cache_file = "cache." + _suffix + ".txt";
    } else if (arg.find("--filter=") == 0) {
      std::string filterStr = arg.substr(9);
      parseFilterList(filterStr);
    } else if (arg.find("--rand-num=") == 0) {
      _rand_num = std::stoi(arg.substr(11));
    }
  }

  if (_suffix.empty() || _rand_num <= 0) {
    std::cerr << "Error: suffix and rand-num are required" << std::endl;
    return false;
  }

  return true;
}

void FileRandomSelector::Execute()
{
  // 加载历史记录
  if (!loadHistory()) {
    std::cerr << "Warning: Failed to load history file" << std::endl;
  }

  // 计算文件统计信息
  auto [total_files, cached_files] = calculateFileCounts();
  _total_files                     = total_files;
  _cached_files                    = cached_files;

  // 检查是否需要清空缓存
  if (_cached_files >= _total_files) {
    std::cout << "All files have been selected, clearing cache..." << std::endl;
    clearHistory();
    _cached_files = 0;
  }

  // 获取过滤后的文件列表
  auto files = getFilteredFiles();

  if (files.empty()) {
    std::cout << "No files found matching the criteria" << std::endl;
    // 即使没有文件，也要更新缓存文件头信息
    if (!saveHistory()) {
      std::cerr << "Warning: Failed to save history file" << std::endl;
    }
    return;
  }

  // 随机选择文件
  auto selected = randomSelectFiles(files);

  // 输出结果
  std::cout << std::endl;
  for (const auto& file : selected) {
    std::cout << file << std::endl;
  }
  std::cout << std::endl;

  // 创建新的历史记录条目
  HistoryEntry new_entry;
  new_entry.timestamp = getCurrentTimestamp();
  new_entry.files     = selected;
  _history_entries.push_back(new_entry);

  // 更新已选文件数量和集合
  _cached_files += selected.size();
  for (const auto& file : selected) {
    _history_files.insert(file);
  }

  // 保存历史记录
  if (!saveHistory()) {
    std::cerr << "Warning: Failed to save history file" << std::endl;
  }

  // 再次检查是否需要清空缓存
  if (_cached_files >= _total_files) {
    std::cout << "All files have been selected, clearing cache..." << std::endl;
    clearHistory();
  }
}

std::string FileRandomSelector::GetCurrentExecutablePath() const
{
  try {
#ifdef _WIN32
    wchar_t path [MAX_PATH];
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    return fs::path(path).parent_path().string();
#else
    return fs::canonical("/proc/self/exe").parent_path().string();
#endif
  } catch (const std::exception& e) {
    std::cerr << "Error getting executable path: " << e.what() << std::endl;
    return fs::current_path().string();
  }
}

std::vector<std::string> FileRandomSelector::getFilteredFiles()
{
  std::vector<std::string> result;
  fs::path                 currentPath = GetCurrentExecutablePath();

  try {
    for (const auto& entry : fs::directory_iterator(currentPath)) {
      if (entry.is_regular_file()) {
        std::string filename = entry.path().filename().string();

        // 检查文件后缀
        if (!_suffix.empty()) {
          if (filename.length() <= _suffix.length() ||
              filename.substr(filename.length() - _suffix.length() - 1) !=
                  "." + _suffix) {
            continue;
          }
        }

        // 检查过滤列表
        if (std::find(_filter_list.begin(), _filter_list.end(), filename) !=
            _filter_list.end()) {
          continue;
        }

        // 检查历史记录
        if (_history_files.find(filename) != _history_files.end()) { continue; }

        result.push_back(filename);
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "Error reading directory: " << e.what() << std::endl;
  }

  return result;
}

std::pair<int, int> FileRandomSelector::calculateFileCounts()
{
  int      total_count    = 0;
  int      filtered_count = 0;
  fs::path currentPath    = GetCurrentExecutablePath();

  try {
    // 计算总文件数（指定后缀）
    for (const auto& entry : fs::directory_iterator(currentPath)) {
      if (entry.is_regular_file()) {
        std::string filename = entry.path().filename().string();

        // 检查文件后缀
        if (!_suffix.empty()) {
          if (filename.length() <= _suffix.length() ||
              filename.substr(filename.length() - _suffix.length() - 1) !=
                  "." + _suffix) {
            continue;
          }
        }

        total_count++;

        // 检查是否在过滤列表中
        if (std::find(_filter_list.begin(), _filter_list.end(), filename) !=
            _filter_list.end()) {
          filtered_count++;
        }
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "Error calculating file counts: " << e.what() << std::endl;
  }

  int effective_total = total_count - filtered_count;
  return { effective_total, static_cast<int>(_history_files.size()) };
}

bool FileRandomSelector::loadHistory()
{
  std::ifstream file(_cache_file);
  if (!file.is_open()) { return false; }

  // 读取第一行（头信息）
  std::string header;
  if (!std::getline(file, header)) { return false; }

  if (!parseCacheHeader(header)) { return false; }

  // 读取历史记录条目
  std::string  line;
  HistoryEntry current_entry;
  bool         in_entry = false;

  while (std::getline(file, line)) {
    // 跳过空行
    if (line.empty()) { continue; }

    // 检查是否是时间戳行
    if (line.front() == '[' && line.back() == ']') {
      // 如果已经有一个条目在处理中，先保存它
      if (in_entry && !current_entry.files.empty()) {
        _history_entries.push_back(current_entry);
        // 更新历史文件集合
        for (const auto& file : current_entry.files) {
          _history_files.insert(file);
        }
      }

      // 开始新的条目
      current_entry = HistoryEntry();
      current_entry.timestamp =
          line.substr(1, line.length() - 2);  // 去掉方括号
      in_entry = true;
    } else if (in_entry) {
      // 如果是文件名行
      current_entry.files.push_back(line);
    }
  }

  // 保存最后一个条目
  if (in_entry && !current_entry.files.empty()) {
    _history_entries.push_back(current_entry);
    for (const auto& file : current_entry.files) {
      _history_files.insert(file);
    }
  }

  file.close();
  return true;
}

bool FileRandomSelector::saveHistory()
{
  std::ofstream file(_cache_file);
  if (!file.is_open()) { return false; }

  // 写入头信息
  file << "totalFile=" << _total_files << ", cacheNum=" << _cached_files
       << std::endl;

  // 写入所有历史记录条目
  for (const auto& entry : _history_entries) {
    file << "[" << entry.timestamp << "]" << std::endl;
    for (const auto& filename : entry.files) {
      file << filename << std::endl;
    }
    file << std::endl;  // 条目之间的空行
  }

  file.close();
  return true;
}

void FileRandomSelector::clearHistory()
{
  _history_files.clear();
  _history_entries.clear();
  fs::remove(_cache_file);
}

std::vector<std::string>
FileRandomSelector::randomSelectFiles(const std::vector<std::string>& files)
{
  if (files.empty()) { return {}; }

  // 如果请求数量大于可用文件数量，返回所有可用文件
  int select_count = std::min(_rand_num, static_cast<int>(files.size()));

  std::vector<std::string> shuffled = files;
  std::random_device       rd;
  std::mt19937             g(rd());
  std::shuffle(shuffled.begin(), shuffled.end(), g);

  std::vector<std::string> selected(shuffled.begin(),
                                    shuffled.begin() + select_count);

  return selected;
}

void FileRandomSelector::parseFilterList(const std::string& filterStr)
{
  if (filterStr.length() < 2 || filterStr.front() != '[' ||
      filterStr.back() != ']') {
    return;
  }

  std::string content = filterStr.substr(1, filterStr.length() - 2);
  size_t      start   = 0;
  size_t      end     = 0;

  while ((end = content.find(',', start)) != std::string::npos) {
    std::string item = content.substr(start, end - start);
    // 去除前后空格
    item.erase(0, item.find_first_not_of(' '));
    item.erase(item.find_last_not_of(' ') + 1);
    if (!item.empty()) { _filter_list.push_back(item); }
    start = end + 1;
  }

  // 添加最后一个项目
  std::string lastItem = content.substr(start);
  lastItem.erase(0, lastItem.find_first_not_of(' '));
  lastItem.erase(lastItem.find_last_not_of(' ') + 1);
  if (!lastItem.empty()) { _filter_list.push_back(lastItem); }
}

bool FileRandomSelector::parseCacheHeader(const std::string& header)
{
  size_t totalPos = header.find("totalFile=");
  size_t cachePos = header.find("cacheNum=");

  if (totalPos == std::string::npos || cachePos == std::string::npos) {
    return false;
  }

  try {
    // 解析 totalFile
    size_t totalStart = totalPos + 10;
    size_t totalEnd   = header.find(',', totalStart);
    if (totalEnd == std::string::npos) return false;
    std::string totalStr = header.substr(totalStart, totalEnd - totalStart);
    _total_files         = std::stoi(totalStr);

    // 解析 cacheNum
    size_t      cacheStart = cachePos + 9;
    std::string cacheStr   = header.substr(cacheStart);
    _cached_files          = std::stoi(cacheStr);

    return true;
  } catch (const std::exception& e) {
    std::cerr << "Error parsing cache header: " << e.what() << std::endl;
    return false;
  }
}

std::string FileRandomSelector::getCurrentTimestamp() const
{
  auto now        = std::chrono::system_clock::now();
  auto time_t_now = std::chrono::system_clock::to_time_t(now);

  std::tm tm;
#ifdef _WIN32
  localtime_s(&tm, &time_t_now);
#else
  localtime_r(&time_t_now, &tm);
#endif

  std::ostringstream oss;
  oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S%z");

  std::string timestamp = oss.str();

  // 格式化时区部分为 +08:00 格式
  if (timestamp.length() > 5) { timestamp.insert(timestamp.length() - 2, ":"); }

  return timestamp;
}
