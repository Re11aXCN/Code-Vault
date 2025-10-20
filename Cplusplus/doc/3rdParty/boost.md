# asio

协程 服务器更简单

```
#include <asio.hpp>
using asio::ip::tcp;

const size_t port = 6970;

asio::io_context ioc;

asio::awaitable<void> echo(tcp::socket socket)
{
    char data[1024];
    std::size_t n = co_await socket.async_read_some(asio::buffer(data), asio::use_awaitable);
    co_await socket.async_write_some(asio::buffer(data, n), asio::use_awaitable);
}

asio::awaitable<void> accept_loop(tcp::acceptor server)
{
    while (true) {
        tcp::socket socker = co_await server.async_accept(io, asio::use_awaitable);

        asio::co_spawn(io, echo(std::move(socker)), asio::detached);
    }
}

int main()
{
    tcp::acceptor server(io, tcp::endpoint(tcp::v4(), port));
    asio::co_spawn(io, accept_loop(std::move(server)), asio::detached);

    io.run();
    return 0;
}
```

