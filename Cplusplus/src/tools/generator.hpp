#pragma once

#include <vector>
#include <random>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <regex>
template<typename T>
auto generate_vec_data(size_t n, T min_val, T max_val) -> std::vector<T> {
    if (n < 1) return {};
    std::random_device rd;
    std::mt19937 gen(rd());

    if constexpr (std::is_integral_v<T>) {
        if constexpr (sizeof(T) == 1) {
            std::uniform_int_distribution<int> dist(static_cast<int>(min_val), static_cast<int>(max_val));
            std::vector<T> data(n);
            for (auto& val : data) {
                val = static_cast<T>(dist(gen));
            }
            return data;
        }
        else {
            std::uniform_int_distribution<T> dist(min_val, max_val);
            std::vector<T> data(n);
            for (auto& val : data) val = dist(gen);
            return data;
        }
    }
    else {
        std::uniform_real_distribution<T> dist(min_val, max_val);
        std::vector<T> data(n);
        for (auto& val : data) val = dist(gen);
        return data;
    }
}

template<typename T>
void serialize_vectors(const std::vector<std::vector<T>>& vectors,
    const std::string& type_name,
    size_t vector_size,
    T min_val, T max_val) {
    // vec_uint32_t-1000-[-65536,65536].txt
    std::string filename = "vec_" + type_name + "-" +
        std::to_string(vector_size) +
        "-[" + std::to_string(min_val) + "," +
        std::to_string(max_val) + "].txt";

    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return;
    }

    for (const auto& vec : vectors) {
        file << "[";
        for (size_t i = 0; i < vec.size(); ++i) {
            file << vec[i];
            if (i != vec.size() - 1) {
                file << ",";
            }
        }
        file << "]\n";
    }

    file.close();
    std::cout << "数据已保存到: " << filename << std::endl;
}

struct FileInfo {
    std::string type_name;
    size_t vector_size;
    int min_val;
    int max_val;
    size_t line_count;
};

static FileInfo parse_filename(const std::string& filename) {
    FileInfo info;

    // 使用正则表达式解析文件名
    std::regex pattern(R"(vec_(\w+)-(\d+)-\[(-?\d+),(-?\d+)\].txt)");
    std::smatch matches;

    if (std::regex_match(filename, matches, pattern) && matches.size() == 5) {
        info.type_name = matches[1].str();
        info.vector_size = std::stoul(matches[2].str());
        info.min_val = std::stoi(matches[3].str());
        info.max_val = std::stoi(matches[4].str());
    }

    // 计算文件行数
    std::ifstream temp_file(filename);
    info.line_count = 0;
    std::string line;
    while (std::getline(temp_file, line)) {
        if (!line.empty()) {
            info.line_count++;
        }
    }
    temp_file.close();

    return info;
}

template<typename T>
std::vector<std::vector<T>> deserialize_vectors(const std::string& filename) {
    FileInfo info = parse_filename(filename);

    std::vector<std::vector<T>> result;
    result.reserve(info.line_count); // 预留外部vector大小

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return result;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        // 移除方括号
        line = line.substr(1, line.length() - 2);

        std::vector<T> vec;
        vec.reserve(info.vector_size); // 预留内部vector大小

        std::stringstream ss(line);
        std::string token;

        while (std::getline(ss, token, ',')) {
            // 转换为对应类型
            std::stringstream converter(token);
            T value;
            converter >> value;
            vec.push_back(value);
        }

        result.push_back(std::move(vec));
    }

    file.close();
    return result;
}