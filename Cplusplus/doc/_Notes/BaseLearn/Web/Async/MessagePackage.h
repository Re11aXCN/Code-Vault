#ifndef MESSAGEPACKAGE_H
#define MESSAGEPACKAGE_H

#include <json/json.h>  // 需要包含jsoncpp头文件
#include <string>
#include <vector>
#include <memory>
#include <cstring>
#include <stdexcept>
//#include <arpa/inet.h>  // 用于字节序转换

/**
 * @brief 消息头结构，定义消息的基本信息
 */
struct MessageHeader {
    uint32_t _messageId;     // 消息ID
    uint32_t _messageSize;   // 消息体大小（不包括头部）
    uint16_t _messageType;   // 消息类型
    uint16_t _reserved;      // 保留字段，用于对齐
    
    /**
     * @brief 转换为网络字节序（大端）
     */
    void ToNetworkOrder() {
        _messageId = htonl(_messageId);
        _messageSize = htonl(_messageSize);
        _messageType = htons(_messageType);
        _reserved = htons(_reserved);
    }
    
    /**
     * @brief 从网络字节序转换为主机字节序
     */
    void ToHostOrder() {
        _messageId = ntohl(_messageId);
        _messageSize = ntohl(_messageSize);
        _messageType = ntohs(_messageType);
        _reserved = ntohs(_reserved);
    }
};

/**
 * @brief 消息包类，处理消息的序列化和反序列化
 */
class MessagePackage {
public:
    /**
     * @brief 默认构造函数
     */
    MessagePackage() : _currentSize(0) {}
    
    /**
     * @brief 构造函数，从消息ID和JSON数据创建消息包
     * @param messageId 消息ID
     * @param messageType 消息类型
     * @param jsonData JSON格式的消息数据
     */
    MessagePackage(uint32_t messageId, uint16_t messageType, const Json::Value& jsonData) {
        SetData(messageId, messageType, jsonData);
    }
    
    /**
     * @brief 设置消息数据
     * @param messageId 消息ID
     * @param messageType 消息类型
     * @param jsonData JSON格式的消息数据
     */
    void SetData(uint32_t messageId, uint16_t messageType, const Json::Value& jsonData) {
        // 序列化JSON数据
        Json::FastWriter writer;
        std::string jsonStr = writer.write(jsonData);
        
        // 设置消息头
        _header._messageId = messageId;
        _header._messageType = messageType;
        _header._messageSize = static_cast<uint32_t>(jsonStr.size());
        _header._reserved = 0;
        
        // 准备缓冲区
        _buffer.resize(sizeof(MessageHeader) + jsonStr.size());
        
        // 复制消息头（转换为网络字节序）
        MessageHeader networkHeader = _header;
        networkHeader.ToNetworkOrder();
        std::memcpy(_buffer.data(), &networkHeader, sizeof(MessageHeader));
        
        // 复制消息体
        std::memcpy(_buffer.data() + sizeof(MessageHeader), jsonStr.c_str(), jsonStr.size());
        
        _currentSize = _buffer.size();
    }
    
    /**
     * @brief 从二进制数据解析消息包
     * @param data 二进制数据指针
     * @param size 数据大小
     * @return 解析是否成功
     */
    bool ParseFromBuffer(const char* data, size_t size) {
        if (size < sizeof(MessageHeader)) {
            return false; // 数据不足以包含消息头
        }
        
        // 复制消息头并转换字节序
        std::memcpy(&_header, data, sizeof(MessageHeader));
        _header.ToHostOrder();
        
        // 检查消息大小是否合理
        if (_header._messageSize > 10 * 1024 * 1024) { // 限制消息大小为10MB
            return false;
        }
        
        // 检查是否收到完整消息
        size_t totalSize = sizeof(MessageHeader) + _header._messageSize;
        if (size < totalSize) {
            return false; // 数据不完整
        }
        
        // 复制整个消息
        _buffer.resize(totalSize);
        std::memcpy(_buffer.data(), data, totalSize);
        _currentSize = totalSize;
        
        return true;
    }
    
    /**
     * @brief 获取消息的JSON数据
     * @return JSON对象
     */
    Json::Value GetJsonData() const {
        if (_currentSize <= sizeof(MessageHeader)) {
            return Json::Value(); // 返回空对象
        }
        
        // 提取JSON字符串
        std::string jsonStr(_buffer.data() + sizeof(MessageHeader), _header._messageSize);
        
        // 解析JSON
        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(jsonStr, root)) {
            return Json::Value(); // 解析失败，返回空对象
        }
        
        return root;
    }
    
    /**
     * @brief 获取消息ID
     * @return 消息ID
     */
    uint32_t GetMessageId() const {
        return _header._messageId;
    }
    
    /**
     * @brief 获取消息类型
     * @return 消息类型
     */
    uint16_t GetMessageType() const {
        return _header._messageType;
    }
    
    /**
     * @brief 获取消息大小
     * @return 消息大小（包括头部）
     */
    size_t GetSize() const {
        return _currentSize;
    }
    
    /**
     * @brief 获取消息缓冲区
     * @return 缓冲区数据指针
     */
    const char* GetBuffer() const {
        return _buffer.data();
    }
    
    /**
     * @brief 清空消息
     */
    void Clear() {
        _buffer.clear();
        _currentSize = 0;
        std::memset(&_header, 0, sizeof(MessageHeader));
    }
    
private:
    MessageHeader _header;       // 消息头
    std::vector<char> _buffer;   // 消息缓冲区（包括头部和消息体）
    size_t _currentSize;         // 当前消息大小
};

#endif //!MESSAGEPACKAGE_H