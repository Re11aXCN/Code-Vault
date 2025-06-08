#include <boost/asio.hpp>
#include <iostream>
#include <set>
#include <memory>
constexpr size_t MAX_BUFFER_SIZE = 1024;
using SocketPtr = std::shared_ptr<boost::asio::ip::tcp::socket>;
std::set<std::shared_ptr<std::thread>> threads;
void Seesion(SocketPtr socket) {
    try {
        while (true) {
            boost::system::error_code ec;
            char buffer[MAX_BUFFER_SIZE];
            memset(buffer, '\0', MAX_BUFFER_SIZE);

            //如果用户发送3个字节，那么就一直等接受到1024不友好
            //size_t length = boost::asio::read(socket, boost::asio::buffer(buffer, MAX_BUFFER_SIZE), ec);

            size_t length = socket->read_some(boost::asio::buffer(buffer, MAX_BUFFER_SIZE), ec);
            if (ec == boost::asio::error::eof) {
                std::cout << "Client disconnected" << std::endl;
                break;
            }
            else if (ec) {
                throw boost::system::system_error(ec, "Error reading from socket");
            }
            std::cout << "Received: " << socket->remote_endpoint().address().to_string() << ":" << socket->remote_endpoint().port() << ". Data: " << buffer << std::endl;
            boost::asio::write(*socket, boost::asio::buffer(buffer, length));
        }
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

void Server(boost::asio::io_context& context, const std::string& port)
{
    boost::asio::ip::tcp::acceptor acceptor(context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), std::stoi(port)));
    while (true) {
        SocketPtr socket(new boost::asio::ip::tcp::socket(context));
        acceptor.accept(*socket);
        std::cout << "New connection from " << socket->remote_endpoint().address().to_string() << ":" << socket->remote_endpoint().port() << std::endl;
        //  Seesion(socket); // 这样写，不开线程，会串行执行，阻塞了
        auto session = std::make_shared<std::thread>(Seesion, socket);
        threads.insert(session); // 防止被释放，通过引用计数
    }
}
int main()
{
    using namespace boost::asio;
    try {
        io_context contex;
        Server(contex, "8080");
        for (auto& t : threads) {
            t->join();
        }//主线程必须等待所有子线程完成结束，如果主线程直接退出，可能会导致子线程未完成的情况
    }
    catch (std::exception& e) {

    }
    return 0;
}