#include <boost/asio.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <iostream>
#include <memory>
#include <atomic>

namespace asio = boost::asio;
using asio::ip::tcp;
using namespace asio::experimental::awaitable_operators;
/*
class session : public std::enable_shared_from_this<session> {
public:
    session(tcp::socket socket) : socket_(std::move(socket)) {}

    void start() {
        std::weak_ptr<session> weak_self = shared_from_this();
        co_spawn(socket_.get_executor(),
            [self = shared_from_this()] { return self->run_session(); },
            [weak_self](std::exception_ptr ep) {
                // 尝试将弱引用转为强引用
                if (auto self = weak_self.lock()) {
                    if (ep) {
                        try { std::rethrow_exception(ep); }
                        catch (const std::exception& e) {
                            std::cerr << "Session exception: " << e.what() << "\n";
                        }
                    }
                    self->handle_disconnect();
                }
                else {
                    std::cerr << "Session already destroyed\n";
                }
            });
    }

private:
    asio::awaitable<void> run_session() {
        try {
            // 设置超时和断开连接检测
            co_await(receive_data() || watchdog_timer());
        }
        catch (const std::exception& e) {
            std::cerr << "Session error: " << e.what() << "\n";
        }
        handle_disconnect();
    }

    asio::awaitable<void> receive_data() {
        try {
            while (socket_.is_open()) {
                // 设置读取超时
                timer_.expires_after(std::chrono::seconds(30));

                // 异步读取
                std::size_t n = co_await asio::async_read_until(
                    socket_, asio::dynamic_buffer(buffer_), '\n',
                    asio::redirect_error(asio::use_awaitable, ec_));

                if (ec_) {
                    if (ec_ != asio::error::operation_aborted) {
                        throw boost::system::system_error(ec_);
                    }
                    co_return;
                }

                // 处理接收到的消息
                std::string message = buffer_.substr(0, n);
                buffer_.erase(0, n);

                // 异步写入回复
                co_await asio::async_write(
                    socket_, asio::buffer("ECHO: " + message),
                    asio::redirect_error(asio::use_awaitable, ec_));

                if (ec_) throw boost::system::system_error(ec_);
            }
        }
        catch (const boost::system::system_error& e) {
            handle_network_error(e.code());
        }
    }

    asio::awaitable<void> watchdog_timer() {
        while (socket_.is_open()) {
            timer_.expires_after(std::chrono::seconds(30));
            co_await timer_.async_wait(asio::redirect_error(asio::use_awaitable, ec_));
            if (ec_) co_return; // 正常取消

            if (socket_.is_open()) {
                std::cerr << "Connection timeout. Forcing disconnect.\n";
                force_disconnect();
            }
        }
    }

    void handle_network_error(const boost::system::error_code& ec) {
        if (ec == asio::error::eof) {
            std::cout << "Client disconnected gracefully\n";
        }
        else if (ec == asio::error::connection_reset) {
            std::cerr << "Client connection reset\n";
        }
        else if (ec == asio::error::operation_aborted) {
            std::cout << "Operation cancelled\n";
        }
        else if (ec == asio::error::timed_out) {
            std::cerr << "Network operation timed out\n";
        }
        else {
            std::cerr << "Network error: " << ec.message() << "\n";
        }
    }

    void force_disconnect() {
        boost::system::error_code ec;
        socket_.shutdown(tcp::socket::shutdown_both, ec);
        if (ec && ec != asio::error::not_connected) {
            std::cerr << "Shutdown error: " << ec.message() << "\n";
        }
        socket_.close(ec);
        timer_.cancel();
    }

    void handle_disconnect() {
        force_disconnect();
        std::cout << "Session ended. Total sessions: " << --session_count << "\n";
    }

    tcp::socket socket_;
    asio::steady_timer timer_{ socket_.get_executor() };
    std::string buffer_;
    boost::system::error_code ec_;
    inline static std::atomic<int> session_count{ 0 };
    friend class server;
};

class server {
public:
    server(asio::io_context& io_context, unsigned short port)
        : acceptor_(io_context, { tcp::v4(), port }) {
        accept_connections();
    }

private:
    void accept_connections() {
        co_spawn(acceptor_.get_executor(), [this] {
            return async_accept();
            }, [](std::exception_ptr ep) {
                if (ep) try { std::rethrow_exception(ep); }
                catch (const std::exception& e) {
                    std::cerr << "Acceptor error: " << e.what() << "\n";
                }
                });
    }

    asio::awaitable<void> async_accept() {
        while (acceptor_.is_open()) {
            try {
                auto socket = co_await acceptor_.async_accept(asio::use_awaitable);

                std::make_shared<session>(std::move(socket))->start();
                std::cout << "New connection. Total sessions: "
                    << ++session::session_count << "\n";
            }
            catch (const boost::system::system_error& e) {
                if (e.code() != asio::error::operation_aborted) {
                    std::cerr << "Accept failed: " << e.what() << "\n";
                }
            }
        }
    }

    tcp::acceptor acceptor_;
};

int main() {
    try {
        asio::io_context io_context;
        server srv(io_context, 12345);

        // 处理操作系统信号
        asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) {
            std::cout << "Shutting down server...\n";
            io_context.stop();
            });

        std::cout << "Server started on port 12345\n";
        io_context.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Server fatal error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
*/

class session : public std::enable_shared_from_this<session> {
public:
    session(tcp::socket socket) : socket_(std::move(socket)) {}

    void start() {
        std::weak_ptr<session> weak_self = shared_from_this();
        co_spawn(socket_.get_executor(),
            [self = shared_from_this()] { return self->run_session(); },
            [weak_self](std::exception_ptr ep) {
                if (auto self = weak_self.lock()) {
                    if (ep) {
                        try { std::rethrow_exception(ep); }
                        catch (const std::exception& e) {
                            std::cerr << "Session exception: " << e.what() << "\n";
                        }
                    }
                    // 不再调用 handle_disconnect() 避免重复
                }
                else {
                    std::cerr << "Session already destroyed\n";
                }
            });
    }

private:
    asio::awaitable<void> run_session() {
        try {
            co_await(receive_data() || watchdog_timer());
        }
        catch (const std::exception& e) {
            std::cerr << "Session error: " << e.what() << "\n";
        }
        handle_disconnect();
    }

    asio::awaitable<void> receive_data() {
        const size_t MAX_BUFFER_SIZE = 1024 * 1024; // 1MB限制
        const std::chrono::seconds READ_TIMEOUT(10); // 读超时10秒
        try {
            while (socket_.is_open()) {
                // 检查缓冲区大小
                if (buffer_.size() > MAX_BUFFER_SIZE) {
                    throw std::runtime_error("Buffer overflow");
                }
                timer_.expires_after(READ_TIMEOUT);
                boost::system::error_code ec;
                std::size_t n = co_await asio::async_read_until(
                    socket_, asio::dynamic_buffer(buffer_), '\r\n',
                    asio::redirect_error(asio::use_awaitable, ec));

                if (ec) {
                    if (ec != asio::error::operation_aborted) {
                        throw boost::system::system_error(ec);
                    }
                    co_return; // 正常取消
                }

                // 检查是否超时
                if (timer_.expiry() <= asio::steady_timer::clock_type::now()) {
                    throw std::runtime_error("Read operation timed out");
                }

                // 重置看门狗定时器
                timer_.cancel();

                // 处理接收到的消息
                std::string message = buffer_.substr(0, n);
                buffer_.erase(0, n);

                // 异步写入回复
                co_await asio::async_write(
                    socket_, asio::buffer("ECHO: " + message),
                    asio::redirect_error(asio::use_awaitable, ec));

                if (ec) throw boost::system::system_error(ec);
            }
        }
        catch (const std::exception&) {
            throw; // 传播异常到外层处理
        }
    }

    asio::awaitable<void> watchdog_timer() {
        boost::system::error_code ec;
        while (socket_.is_open()) {
            timer_.expires_after(std::chrono::seconds(30));
            ec.clear();
            co_await timer_.async_wait(asio::redirect_error(asio::use_awaitable, ec));

            if (ec == asio::error::operation_aborted) {
                continue; // 正常取消，重新等待
            }

            if (ec) {
                break; // 其他错误退出
            }

            // 超时处理
            if (socket_.is_open()) {
                std::cerr << "Connection timeout. Forcing disconnect.\n";
                force_disconnect();
                break;
            }
        }
    }

    void handle_network_error(const boost::system::error_code& ec) {
        if (ec == asio::error::eof) {
            std::cout << "Client disconnected gracefully\n";
        }
        else if (ec == asio::error::connection_reset) {
            std::cerr << "Client connection reset\n";
        }
        else if (ec == asio::error::operation_aborted) {
            std::cout << "Operation cancelled\n";
        }
        else if (ec == asio::error::timed_out) {
            std::cerr << "Network operation timed out\n";
        }
        else {
            std::cerr << "Network error: " << ec.message() << "\n";
        }
    }

    void force_disconnect() {
        boost::system::error_code ec;
        socket_.shutdown(tcp::socket::shutdown_both, ec);
        if (ec && ec != asio::error::not_connected) {
            std::cerr << "Shutdown error: " << ec.message() << "\n";
        }
        socket_.close(ec);
        timer_.cancel();
    }

    void handle_disconnect() {
        force_disconnect();
        std::cout << "Session ended. Total sessions: " << --session_count << "\n";
    }

    tcp::socket socket_;
    asio::steady_timer timer_{ socket_.get_executor() };
    std::string buffer_;
    inline static std::atomic<int> session_count{ 0 };
    friend class server;
};

class server {
public:
    server(asio::io_context& io_context, unsigned short port)
        : acceptor_(io_context, { tcp::v4(), port }) {
        accept_connections();
    }

private:
    void accept_connections() {
        co_spawn(acceptor_.get_executor(), [this] {
            return async_accept();
            }, [](std::exception_ptr ep) {
                if (ep) try { std::rethrow_exception(ep); }
                catch (const std::exception& e) {
                    std::cerr << "Acceptor error: " << e.what() << "\n";
                }
                });
    }

    asio::awaitable<void> async_accept() {
        while (acceptor_.is_open()) {
            try {
                auto socket = co_await acceptor_.async_accept(asio::use_awaitable);

                std::make_shared<session>(std::move(socket))->start();
                std::cout << "New connection. Total sessions: "
                    << ++session::session_count << "\n";
            }
            catch (const boost::system::system_error& e) {
                if (e.code() != asio::error::operation_aborted) {
                    std::cerr << "Accept failed: " << e.what() << "\n";
                }
                else {
                    break; // 被主动取消时退出循环
                }
            }
        }
    }

    tcp::acceptor acceptor_;
};

int main() {
    try {
        asio::io_context io_context;
        server srv(io_context, 12345);

        asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) {
            std::cout << "Shutting down server...\n";
            io_context.stop();
            });

        std::cout << "Server started on port 12345\n";
        io_context.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Server fatal error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
