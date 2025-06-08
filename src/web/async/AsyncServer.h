#ifndef ASYNCSERVER_H
#define ASYNCSERVER_H

#include <boost/asio.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <format>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>
#include "ThreadPool.h"
#include "AsyncSession.h"
#include "LogicSystem.h"

/**
 * @brief 异步服务器类，管理客户端连接
 */
class AsyncServer : public std::enable_shared_from_this<AsyncServer> {
public:
    /**
     * @brief 构造函数
     * @param port 监听端口
     */
    AsyncServer(unsigned int port)
        : _port(port)
        , _acceptor(ThreadPool::getInstance().GetNextContext())
        , _isRunning(false)
    {
        std::cout << std::format("AsyncServer created, port: {}", port) << std::endl;
    }
    
    /**
     * @brief 析构函数
     */
    ~AsyncServer() {
        Stop();
        std::cout << std::format("AsyncServer destroyed") << std::endl;
    }
    
    /**
     * @brief 启动服务器
     * @return 启动是否成功
     */
    bool Start() {
        if (_isRunning) return true;
        
        try {
            // 确保线程池已初始化
            if (!ThreadPool::getInstance().IsRunning()) {
                ThreadPool::getInstance().Initialize();
            }
            
            // 确保逻辑系统已初始化
            if (!LogicSystem::getInstance().IsRunning()) {
                LogicSystem::getInstance().Initialize();
            }
            
            // 创建接受器并绑定端点
            boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), _port);
            _acceptor.open(endpoint.protocol());
            
            // 设置接受器选项
            _acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
            _acceptor.bind(endpoint);
            _acceptor.listen();
            
            _isRunning = true;
            
            // 开始接受连接
            _startAccept();
            
            std::cout << std::format("AsyncServer started, listening on port: {}", _port) << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << std::format("Failed to start server: {}", e.what()) << std::endl;
            return false;
        }
    }
    
    /**
     * @brief 停止服务器
     */
    void Stop() {
        if (!_isRunning) return;
        
        _isRunning = false;
        
        // 关闭接受器
        boost::system::error_code ec;
        _acceptor.close(ec);
        
        // 关闭所有会话
        std::unordered_map<std::string, std::shared_ptr<AsyncSession>> sessions;
        {
            std::lock_guard<std::mutex> lock(_sessionsMutex);
            sessions = std::move(_sessions);
        }
        
        for (auto& [uuid, session] : sessions) {
            session->Close();
        }
        
        std::cout << std::format("AsyncServer stopped") << std::endl;
    }
    
    /**
     * @brief 广播消息给所有客户端
     * @param message 消息包
     */
    void Broadcast(const MessagePackage& message) {
        std::lock_guard<std::mutex> lock(_sessionsMutex);
        for (auto& [uuid, session] : _sessions) {
            if (session && session->IsConnected()) {
                session->Send(message);
            }
        }
    }
    
    /**
     * @brief 发送消息给指定客户端
     * @param uuid 客户端UUID
     * @param message 消息包
     * @return 发送是否成功
     */
    bool SendToClient(const std::string& uuid, const MessagePackage& message) {
        std::lock_guard<std::mutex> lock(_sessionsMutex);
        auto it = _sessions.find(uuid);
        if (it != _sessions.end() && it->second && it->second->IsConnected()) {
            it->second->Send(message);
            return true;
        }
        return false;
    }
    
    /**
     * @brief 关闭指定客户端连接
     * @param uuid 客户端UUID
     * @return 关闭是否成功
     */
    bool CloseClient(const std::string& uuid) {
        std::shared_ptr<AsyncSession> session;
        {
            std::lock_guard<std::mutex> lock(_sessionsMutex);
            auto it = _sessions.find(uuid);
            if (it != _sessions.end()) {
                session = it->second;
            }
        }
        
        if (session) {
            session->Close();
            return true;
        }
        return false;
    }
    
    /**
     * @brief 获取当前连接数
     * @return 连接数
     */
    size_t GetConnectionCount() const {
        std::lock_guard<std::mutex> lock(_sessionsMutex);
        return _sessions.size();
    }
    
    /**
     * @brief 移除会话
     * @param uuid 会话UUID
     */
    void RemoveSession(const std::string& uuid) {
        std::lock_guard<std::mutex> lock(_sessionsMutex);
        _sessions.erase(uuid);
    }
    
private:
    /**
     * @brief 开始接受新连接
     */
    void _startAccept() {
        if (!_isRunning) return;
        
        // 创建新会话
        auto session = std::make_shared<AsyncSession>(
            ThreadPool::getInstance().GetNextContext(),
            this
        );
        
        // 异步接受连接
        _acceptor.async_accept(
            session->GetSocket(),
            [this, session](const boost::system::error_code& ec) {
                _handleAccept(session, ec);
            }
        );
    }
    
    /**
     * @brief 处理接受连接结果
     * @param session 新会话
     * @param ec 错误码
     */
    void _handleAccept(std::shared_ptr<AsyncSession> session, const boost::system::error_code& ec) {
        if (!_isRunning) return;
        
        if (!ec) {
            // 连接成功，启动会话
            std::cout << std::format("New connection accepted, UUID: {}", session->GetUuid()) << std::endl;
            
            // 添加到会话映射表
            {
                std::lock_guard<std::mutex> lock(_sessionsMutex);
                _sessions[session->GetUuid()] = session;
            }
            
            // 启动会话
            session->Start();
        } else {
            std::cerr << std::format("Accept error: {}", ec.message()) << std::endl;
        }
        
        // 继续接受下一个连接
        _startAccept();
    }
    
    unsigned int _port;                                                // 监听端口
    boost::asio::ip::tcp::acceptor _acceptor;                          // 接受器
    std::unordered_map<std::string, std::shared_ptr<AsyncSession>> _sessions;  // 会话映射表
    mutable std::mutex _sessionsMutex;                                 // 会话映射表互斥锁
    std::atomic<bool> _isRunning;                                      // 运行状态标志
};

#endif // !ASYNCSERVER_H