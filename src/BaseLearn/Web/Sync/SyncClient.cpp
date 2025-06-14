
#include <boost/asio.hpp>
#include <iostream>
int main()
{
    using namespace boost::asio;
    try {
        // 1. 创建上下文
        io_context context;
        // 2. 构造端点
        ip::tcp::endpoint endpoint(ip::make_address("127.0.0.1"), 10086);
        // 3. 创建套接字
        ip::tcp::socket socket(context);
        // 4. Error_code
        boost::system::error_code ec = boost::asio::error::host_not_found;
        // 5. 连接
        socket.connect(endpoint, ec);
        if (ec)  throw boost::system::system_error(ec, "Connection failed, Code is");
        // 6. 发送数据
        std::cout << "Enter message: ";
        std::string message;
        std::getline(std::cin, message);
        boost::asio::write(socket, boost::asio::buffer(message));
        // 7. 接收数据
        std::string response(1024, '\0');
        boost::asio::read(socket, boost::asio::buffer(response));
        std::cout << "Received: " << response << std::endl;
        // 8. 关闭套接字
        socket.close();
    }
    catch (std::exception& e) {

    }
    return 0;
}