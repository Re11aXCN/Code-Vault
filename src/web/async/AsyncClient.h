#ifndef ASYNCCLIENT_H
#define ASYNCCLIENT_H

#include <boost/asio.hpp>
#include <string>
#include <memory>
#include <deque>
#include <mutex>
#include <functional>
#include <atomic>
#include <iostream>
#include <format>
#include "MessagePackage.h"
#include "ThreadPool.h"

/**
 * @brief 异步客户端类，连接服务器
 */
class AsyncClient : public std::enable_shared_from_this<AsyncClient> {
public:
    // 连接状态回调函数类型
    using ConnectCallback = std::function<void(bool)>;
    // 消息接收回调函数类型
    using MessageCallback = std::function<void(const MessagePackage&)>;
    // 断开连接回调函数类型
    using DisconnectCallback = std::function<void()>;
    
    /**
     * @brief 构造函数
     */
    AsyncClient()
        : _socket(ThreadPool::getInstance().GetNextContext())
        , _timer(ThreadPool::getInstance().GetNextContext())
        , _isConnected(false)
        , _isConnecting(false)
    {
    }
    
    /**
     * @brief 析构函数
     */
    ~AsyncClient() {
        Disconnect();
    }
    
    /**
     * @brief 连接服务器
     * @param host 服务器地址
     * @param port 服务器端口
     * @param callback 连接结果回调函数
     */
    void Connect(const std::string& host, unsigned short port, ConnectCallback callback = nullptr) {
        if (_isConnected || _isConnecting) {
            if (callback) callback(false);
            return;
        }
        
        _isConnecting = true;
        
        // 确保线程池已初始化
        if (!ThreadPool::getInstance().IsRunning()) {
            ThreadPool::getInstance().Initialize();
        }
        
        // 解析地址
        auto resolver = std::make_shared<boost::asio::ip::tcp::resolver>(_socket.get_executor());
        resolver->async_resolve(
            host,
            std::to_string(port),
            [this, resolver, callback](const boost::system::error_code& ec, boost::asio::ip::tcp::resolver::results_type results) {
                if (ec) {
                    std::cerr << std::format("Resolve error: {}", ec.message()) << std::endl;
                    _isConnecting = false;
                    if (callback) callback(false);
                    return;
                }
                
                // 连接到解析的地址
                boost::asio::async_connect(
                    _socket,
                    results,
                    [this, callback](const boost::system::error_code& ec, const boost::asio::ip::tcp::endpoint& endpoint) {
                        _isConnecting = false;
                        
                        if (ec) {
                            std::cerr << std::format("Connect error: {}", ec.message()) << std::endl;
                            if (callback) callback(false);
                            return;
                        }
                        
                        // 连接成功
                        _isConnected = true;
                        
                        // 设置套接字选项
                        boost::system::error_code optEc;
                        _socket.set_option(boost::asio::ip::tcp::no_delay(true), optEc);
                        _socket.set_option(boost::asio::socket_base::keep_alive(true), optEc);
                        
                        // 开始接收数据
                        _asyncReadHeader();
                        
                        // 启动心跳定时器
                        _startHeartbeatTimer();
                        
                        std::cout << std::format("Connected to {}:{}", endpoint.address().to_string(), endpoint.port()) << std::endl;
                        
                        if (callback) callback(true);
                    }
                );
            }
        );
    }
    
    /**
     * @brief 断开连接
     */
    void Disconnect() {
        if (!_isConnected && !_isConnecting) return;
        
        _isConnected = false;
        _isConnecting = false;
        
        // 取消定时器
        boost::system::error_code ec;
        _timer.cancel();
        
        // 关闭套接字
        if (_socket.is_open()) {
            _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
            _socket.close(ec);
        }
        
        // 调用断开连接回调
        if (_disconnectCallback) {
            _disconnectCallback();
        }
        
        std::cout << "Disconnected from server" << std::endl;
    }
    
    /**
     * @brief 发送消息
     * @param message 消息包
     */
    void Send(const MessagePackage& message) {
        if (!_isConnected) return;
        
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
     * @brief 设置消息接收回调
     * @param callback 回调函数
     */
    void SetMessageCallback(MessageCallback callback) {
        _messageCallback = std::move(callback);
    }
    
    /**
     * @brief 设置断开连接回调
     * @param callback 回调函数
     */
    void SetDisconnectCallback(DisconnectCallback callback) {
        _disconnectCallback = std::move(callback);
    }
    
    /**
     * @brief 检查是否已连接
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
            Disconnect();
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
            // 调用消息回调
            if (_messageCallback) {
                _messageCallback(message);
            }
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
            // 操作被取消，可能是因为客户端被关闭
            return;
        }
        
        std::cerr << std::format("Client error during {}: {}", operation, ec.message()) << std::endl;
        
        if (ec == boost::asio::error::eof ||
            ec == boost::asio::error::connection_reset ||
            ec == boost::asio::error::connection_aborted) {
            // 连接已关闭或重置
            Disconnect();
        }
    }
    
    /**
     * @brief 启动心跳定时器
     */
    void _startHeartbeatTimer() {
        if (!_isConnected) return;
        
        _timer.expires_after(std::chrono::seconds(30)); // 30秒超时
        _timer.async_wait([self = shared_from_this()](const boost::system::error_code& ec) {
            if (!ec) {
                // 超时，发送心跳包或检查连接状态
                if (self->_isConnected) {
                    // 这里可以发送心跳包
                    // self->SendHeartbeat();
                    
                    // 重新启动定时器
                    self->_startHeartbeatTimer();
                }
            }
        });
    }
    
    boost::asio::ip::tcp::socket _socket;          // 套接字
    boost::asio::steady_timer _timer;              // 定时器，用于心跳检测
    std::vector<char> _recvBuffer;                 // 接收缓冲区
    std::deque<MessagePackage> _sendQueue;         // 发送队列
    std::atomic<bool> _isConnected;                // 连接状态标志
    std::atomic<bool> _isConnecting;               // 连接中标志
    MessageCallback _messageCallback;              // 消息接收回调
    DisconnectCallback _disconnectCallback;        // 断开连接回调
};

#endif //!ASYNCCLIENT_H