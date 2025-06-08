#ifndef ASYNCSESSION_H
#define ASYNCSESSION_H

#include <boost/asio.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <memory>
#include <string>
#include <deque>
#include <mutex>
#include <iostream>
#include <format>
#include "MessagePackage.h"

// 前向声明
class AsyncServer;

/**
 * @brief 异步会话类，处理单个客户端连接
 */
class AsyncSession : public std::enable_shared_from_this<AsyncSession> {
public:
    /**
     * @brief 构造函数
     * @param context IO上下文
     * @param server 服务器指针
     */
    AsyncSession(boost::asio::io_context& context, AsyncServer* server)
        : _socket(context)
        , _server(server)
        , _uuid(boost::uuids::to_string(boost::uuids::random_generator()()))
        , _timer(context)
        , _isConnected(false)
    {
        std::cout << std::format("AsyncSession created, UUID: {}", _uuid) << std::endl;
    }
    
    /**
     * @brief 析构函数
     */
    ~AsyncSession() {
        std::cout << std::format("AsyncSession destroyed, UUID: {}", _uuid) << std::endl;
    }
    
    /**
     * @brief 获取套接字引用
     * @return 套接字引用
     */
    boost::asio::ip::tcp::socket& GetSocket() {
        return _socket;
    }
    
    /**
     * @brief 获取会话UUID
     * @return UUID字符串
     */
    const std::string& GetUuid() const {
        return _uuid;
    }
    
    /**
     * @brief 启动会话
     */
    void Start() {
        _isConnected = true;
        
        // 设置套接字选项
        boost::system::error_code ec;
        _socket.set_option(boost::asio::ip::tcp::no_delay(true), ec);
        if (ec) {
            std::cerr << std::format("Failed to set TCP no delay: {}", ec.message()) << std::endl;
        }
        
        // 设置保活选项
        _socket.set_option(boost::asio::socket_base::keep_alive(true), ec);
        if (ec) {
            std::cerr << std::format("Failed to set keep alive: {}", ec.message()) << std::endl;
        }
        
        // 启动读取操作
        _asyncReadHeader();
        
        // 启动心跳定时器
        _startHeartbeatTimer();
    }
    
    /**
     * @brief 发送消息
     * @param message 消息包
     */
    void Send(const MessagePackage& message) {
        // 使用strand确保发送操作的顺序性
        boost::asio::post(_socket.get_executor(), [self = shared_from_this(), message]() {
            bool writeInProgress = !self->_sendQueue.empty();
            
            // 将消息添加到发送队列
            self->_sendQueue.push_back(message);
            
            // 如果没有正在进行的写操作，启动一个
            if (!writeInProgress) {
                self->_asyncWrite();
            }
        });
    }
    
    /**
     * @brief 关闭会话
     */
    void Close() {
        boost::asio::post(_socket.get_executor(), [self = shared_from_this()]() {
            self->_close();
        });
    }
    
    /**
     * @brief 检查会话是否已连接
     * @return 连接状态
     */
    bool IsConnected() const {
        return _isConnected;
    }
    
private:
    /**
     * @brief 异步读取消息头
     */
    void _asyncReadHeader() {
        if (!_isConnected) return;
        
        // 分配接收缓冲区
        _recvBuffer.resize(sizeof(MessageHeader));
        
        // 异步读取消息头
        boost::asio::async_read(
            _socket,
            boost::asio::buffer(_recvBuffer.data(), sizeof(MessageHeader)),
            [self = shared_from_this()](const boost::system::error_code& ec, std::size_t bytes_transferred) {
                self->_handleReadHeader(ec, bytes_transferred);
            }
        );
    }
    
    /**
     * @brief 处理消息头读取结果
     * @param ec 错误码
     * @param bytes_transferred 传输的字节数
     */
    void _handleReadHeader(const boost::system::error_code& ec, std::size_t bytes_transferred) {
        if (ec) {
            _handleError(ec, "Read header");
            return;
        }
        
        // 解析消息头
        MessageHeader header;
        std::memcpy(&header, _recvBuffer.data(), sizeof(MessageHeader));
        header.ToHostOrder();
        
        // 检查消息大小是否合理
        if (header._messageSize > 10 * 1024 * 1024) { // 限制消息大小为10MB
            std::cerr << std::format("Message too large: {} bytes", header._messageSize) << std::endl;
            _close();
            return;
        }
        
        // 调整接收缓冲区大小，准备接收消息体
        _recvBuffer.resize(sizeof(MessageHeader) + header._messageSize);
        
        // 异步读取消息体
        boost::asio::async_read(
            _socket,
            boost::asio::buffer(_recvBuffer.data() + sizeof(MessageHeader), header._messageSize),
            [self = shared_from_this()](const boost::system::error_code& ec, std::size_t bytes_transferred) {
                self->_handleReadBody(ec, bytes_transferred);
            }
        );
    }
    
    /**
     * @brief 处理消息体读取结果
     * @param ec 错误码
     * @param bytes_transferred 传输的字节数
     */
    void _handleReadBody(const boost::system::error_code& ec, std::size_t bytes_transferred) {
        if (ec) {
            _handleError(ec, "Read body");
            return;
        }
        
        // 解析完整消息
        MessagePackage message;
        if (message.ParseFromBuffer(_recvBuffer.data(), _recvBuffer.size())) {
            // 处理消息
            _processMessage(message);
        } else {
            std::cerr << "Failed to parse message" << std::endl;
        }
        
        // 继续读取下一个消息
        _asyncReadHeader();
    }
    
    /**
     * @brief 异步写入消息
     */
    void _asyncWrite() {
        if (_sendQueue.empty() || !_isConnected) return;
        
        // 获取队列中的第一个消息
        const MessagePackage& message = _sendQueue.front();
        
        // 异步写入消息
        boost::asio::async_write(
            _socket,
            boost::asio::buffer(message.GetBuffer(), message.GetSize()),
            [self = shared_from_this()](const boost::system::error_code& ec, std::size_t bytes_transferred) {
                self->_handleWrite(ec, bytes_transferred);
            }
        );
    }
    
    /**
     * @brief 处理写入结果
     * @param ec 错误码
     * @param bytes_transferred 传输的字节数
     */
    void _handleWrite(const boost::system::error_code& ec, std::size_t bytes_transferred) {
        if (ec) {
            _handleError(ec, "Write");
            return;
        }
        
        // 移除已发送的消息
        _sendQueue.pop_front();
        
        // 如果队列不为空，继续发送下一个消息
        if (!_sendQueue.empty()) {
            _asyncWrite();
        }
    }
    
    /**
     * @brief 处理错误
     * @param ec 错误码
     * @param operation 操作名称
     */
    void _handleError(const boost::system::error_code& ec, const std::string& operation) {
        if (ec == boost::asio::error::operation_aborted) {
            // 操作被取消，可能是因为会话被关闭
            return;
        }
        
        std::cerr << std::format("Session error during {}: {}", operation, ec.message()) << std::endl;
        
        if (ec == boost::asio::error::eof ||
            ec == boost::asio::error::connection_reset ||
            ec == boost::asio::error::connection_aborted) {
            // 连接已关闭或重置
            _close();
        }
    }
    
    /**
     * @brief 关闭会话
     */
    void _close() {
        if (!_isConnected) return;
        
        _isConnected = false;
        
        // 取消定时器
        boost::system::error_code ec;
        _timer.cancel();
        
        // 关闭套接字
        if (_socket.is_open()) {
            _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
            _socket.close(ec);
        }
        
        // 通知服务器移除会话
        _notifyServerClose();
    }
    
    /**
     * @brief 启动心跳定时器
     */
    void _startHeartbeatTimer() {
        if (!_isConnected) return;
        
        _timer.expires_after(std::chrono::seconds(30)); // 30秒超时
        _timer.async_wait([self = shared_from_this()](const boost::system::error_code& ec) {
            if (!ec) {
                // 超时，发送心跳包或关闭连接
                if (self->_isConnected) {
                    // 这里可以发送心跳包
                    // self->SendHeartbeat();
                    
                    // 重新启动定时器
                    self->_startHeartbeatTimer();
                }
            }
        });
    }
    
    /**
     * @brief 处理接收到的消息
     * @param message 消息包
     */
    void _processMessage(const MessagePackage& message) {
        // 将消息提交给逻辑系统处理
        LogicSystem::getInstance().SubmitMessage(shared_from_this(), message);
    }
    
    /**
     * @brief 通知服务器关闭会话
     */
    void _notifyServerClose() {
        if (_server) {
            _server->RemoveSession(_uuid);
        }
    }
    
    boost::asio::ip::tcp::socket _socket;          // 套接字
    AsyncServer* _server;                          // 服务器指针
    std::string _uuid;                             // 会话唯一标识符
    boost::asio::steady_timer _timer;              // 定时器，用于心跳检测
    std::vector<char> _recvBuffer;                 // 接收缓冲区
    std::deque<MessagePackage> _sendQueue;         // 发送队列
    std::atomic<bool> _isConnected;                // 连接状态标志
};

#endif //!ASYNCSESSION_H