#pragma once
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <cstdint>
#include <string>
#include <bit>
using json = nlohmann::json;

// 消息类型枚举
enum class MessageType : uint16_t {
    TEXT = 1,       // 文本消息
    COMMAND = 2,    // 命令消息
    RESPONSE = 3    // 响应消息
};

// 消息头结构 (16字节)
#pragma pack(push, 1)
struct MessageHeader {
    uint16_t type;      // 消息类型
    uint32_t length;    // 消息体长度
    uint64_t timestamp; // 时间戳(用于排序)
};
#pragma pack(pop)

// 完整消息结构
struct Message {
    MessageHeader header;
    std::string body;

    // 默认构造函数
    Message() = default;

    // 构造消息
    Message(MessageType type, const std::string& content) {
        header.type = static_cast<uint16_t>(type);
        header.length = content.size();
        header.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
        body = content;
    }

    // 序列化为JSON字符串
    std::string to_json() const {
        json j;
        j["type"] = header.type;
        j["length"] = header.length;
        j["timestamp"] = header.timestamp;
        j["body"] = body;
        return j.dump();
    }

    // 从JSON字符串反序列化
    static Message from_json(const std::string& json_str) {
        try {
            auto j = json::parse(json_str);
            Message msg;
            msg.header.type = j["type"].get<uint16_t>();
            msg.header.length = j["length"].get<uint32_t>();
            msg.header.timestamp = j["timestamp"].get<uint64_t>();
            msg.body = j["body"].get<std::string>();
            return msg;
        }
        catch (const json::parse_error& e) {
            throw std::runtime_error("JSON解析错误: " + std::string(e.what()));
        }
    }

    // 网络字节序转换
    void to_network_order() {
        header.type = htons(header.type);
        header.length = htonl(header.length);
        header.timestamp = std::(header.timestamp);
    }

    // 主机字节序转换
    void to_host_order() {
        header.type = ntohs(header.type);
        header.length = ntohl(header.length);
        header.timestamp = ntohll(header.timestamp);
    }

private:
    // 辅助函数：64位网络字节序转换
    static uint64_t htonll(uint64_t value) {
        static const int num = 42;
        if (*reinterpret_cast<const char*>(&num) == num) { // 小端系统
            return static_cast<uint64_t>(htonl(value & 0xFFFFFFFF)) << 32 |
                htonl(value >> 32);
        }
        return value;
    }

    static uint64_t ntohll(uint64_t value) {
        return htonll(value);
    }
};