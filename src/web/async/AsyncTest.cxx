#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <memory>
#include <csignal>

#include "ThreadPool.h"
#include "AsyncServer.h"
#include "AsyncClient.h"
#include "LogicSystem.h"
#include "MessagePackage.h"

// 信号处理标志
std::atomic<bool> g_running(true);

// 信号处理函数
void signalHandler(int signal) {
    std::cout << "Received signal: " << signal << std::endl;
    g_running = false;
}

// 注册消息处理函数
void registerMessageHandlers() {
    // 注册消息类型1的处理函数
    LogicSystem::getInstance().RegisterHandler(1, [](std::shared_ptr<AsyncSession> session, const MessagePackage& message) {
        std::cout << "Received message type 1 from client: " << session->GetUuid() << std::endl;

        // 获取消息内容
        Json::Value data = message.GetJsonData();
        std::cout << "Message content: " << data.toStyledString() << std::endl;

        // 构造回复消息
        Json::Value response;
        response["status"] = "ok";
        response["message"] = "Message received successfully";
        response["echo"] = data;

        // 发送回复
        MessagePackage responseMsg(message.GetMessageId(), 2, response);
        session->Send(responseMsg);
        });

    // 注册消息类型2的处理函数
    LogicSystem::getInstance().RegisterHandler(2, [](std::shared_ptr<AsyncSession> session, const MessagePackage& message) {
        std::cout << "Received message type 2 from client: " << session->GetUuid() << std::endl;

        // 获取消息内容
        Json::Value data = message.GetJsonData();
        std::cout << "Message content: " << data.toStyledString() << std::endl;

        // 构造广播消息
        Json::Value broadcast;
        broadcast["type"] = "broadcast";
        broadcast["from"] = session->GetUuid();
        broadcast["content"] = data["content"];

        // 创建广播消息包
        MessagePackage broadcastMsg(0, 3, broadcast);

        // 通过服务器广播给所有客户端
        // 注意：这里需要获取服务器实例，可以通过session获取
        // server->Broadcast(broadcastMsg);
        });
}

// 服务器测试函数
void runServer() {
}