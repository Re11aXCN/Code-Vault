#pragma once
#include <boost/asio.hpp>
#include <string>
#include <iostream>

extern int ClientEndPoint() {
	std::string raw_ip_address = "127.4.8.1";
	unsigned short port = 8080;
	boost::system::error_code ecode;
	boost::asio::ip::address ip_address = boost::asio::ip::make_address(raw_ip_address, ecode);
	if (ecode.value() != 0) {
		std::cerr << "Failed to parse IP address. Error Code: " <<  ecode.value() << "Message: " << ecode.message() << std::endl;
		return ecode.value();
	}
	boost::asio::ip::tcp::endpoint endpoint(ip_address, port);
	return 0;
}

extern int ServerEndPoint() {
	unsigned short port = 8080;
	boost::asio::ip::address ip_address = boost::asio::ip::address_v4::any(); // or address_v6::any()
	boost::asio::ip::tcp::endpoint endpoint(ip_address, port);
	return 0;
}

extern int CreateTcpSocket() {
	boost::asio::io_context io_context;
	boost::asio::ip::tcp protocol = boost::asio::ip::tcp::v4(); // or v6()
	boost::asio::ip::tcp::socket socket(io_context);
	boost::system::error_code ecode;
	socket.open(protocol, ecode);
	if (ecode.value() != 0) {
		std::cerr << "Failed to open socket. Error Code: " <<  ecode.value() << "Message: " << ecode.message() << std::endl;
		return ecode.value();
	}
	return 0;
}

extern int CreateAcceptorSocket() {
	boost::asio::io_context io_context;
	/*
	boost::asio::ip::tcp protocol = boost::asio::ip::tcp::v4(); // or v6()
	boost::asio::ip::tcp::acceptor acceptor(io_context);
	boost::system::error_code ecode;
	acceptor.open(protocol, ecode);
	if (ecode.value() != 0) {
		std::cerr << "Failed to open acceptor socket. Error Code: " << ecode.value() << "Message: " << ecode.message() << std::endl;
		return ecode.value();
	}
	*/
	// simplied version
	boost::asio::ip::tcp::acceptor acceptor(io_context, 
		boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 8080));
	return 0;
}

extern int BindAcceptorSocket() {
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address_v4::any(), 8080);

	boost::asio::io_context io_context;
	boost::asio::ip::tcp::acceptor acceptor(io_context, endpoint.protocol());
	boost::system::error_code ecode;
	acceptor.bind(endpoint, ecode);
	if (ecode.value() != 0) {
		std::cerr << "Failed to bind acceptor socket. Error Code: " << ecode.value() << "Message: " << ecode.message() << std::endl;
		return ecode.value();
	}
	return 0;
}

extern int ConnectToEnd() {
	std::string raw_ip_address = "127.4.8.1";
	unsigned short port = 8080;
	try {
		boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address(raw_ip_address), port);
		boost::asio::io_context io_context;
		boost::asio::ip::tcp::socket socket(io_context, endpoint.protocol());
		socket.connect(endpoint);
	}
	catch (boost::system::system_error& e) {
		std::cerr << "Failed to connect to endpoint. Error Code: " << e.code().value() << "Message: " << e.what() << std::endl;
		return e.code().value();
	}
	return 0;
}

extern int DNSConnectToEnd() {
	std::string hostname = "www.google.com";
	std::string port = "80";
	boost::asio::io_context io_context;
	boost::asio::ip::tcp::resolver resolver(io_context);

	try {
		boost::asio::ip::tcp::resolver::results_type endpoints =
			resolver.resolve(hostname, port);
		boost::asio::ip::tcp::socket socket(io_context);
		boost::asio::connect(socket, endpoints);	
	}
	catch (boost::system::system_error& e) {
		std::cerr << "Failed to connect to endpoint. Error Code: " << e.code().value() << "Message: " << e.what() << std::endl;
		return e.code().value();
	}
	return 0;
}

extern int AcceptNewConnection() {
	const int max_connections = 10;
	unsigned short port = 8080;
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address_v4::any(), port);
	boost::asio::io_context io_context;
	try {
		boost::asio::ip::tcp::acceptor acceptor(io_context, endpoint.protocol());
		acceptor.bind(endpoint);
		acceptor.listen(max_connections);

		boost::asio::ip::tcp::socket socket(io_context);
		acceptor.accept(socket);
	}
	catch (boost::system::system_error& e) {
		std::cerr << "Failed to create acceptor. Error Code: " << e.code().value() << "Message: " << e.what() << std::endl;
		return e.code().value();
	}
	return 0;
}

extern void RunServer() {
	// 步骤 1. 创建IO执行上下文
	boost::asio::io_context io_context;

	try {
		// 步骤 2. 创建TCP监听器(Acceptor)
		boost::asio::ip::tcp::acceptor acceptor(io_context);

		// 步骤 3. 创建监听端点(IP+Port)
		boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), 8080);

		// 步骤 4. 打开监听器并绑定端口
		acceptor.open(endpoint.protocol());
		acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
		acceptor.bind(endpoint);

		// 步骤 5. 开始监听连接请求
		acceptor.listen();
		std::cout << "Server listening on port 8080..." << std::endl;

		while (true) {
			// 步骤 6. 创建客户端通信套接字
			boost::asio::ip::tcp::socket client_socket(io_context);

			// 步骤 7. 等待并接受客户端连接
			acceptor.accept(client_socket);
			std::cout << "Client connected: "
				<< client_socket.remote_endpoint().address().to_string()
				<< std::endl;

			try {
				// 步骤 8. 创建缓冲区接收数据
				std::vector<char> receive_buffer(1024);

				// 步骤 9. 同步读取客户端数据
				boost::system::error_code ecode;
				size_t bytes_received = client_socket.read_some(
					boost::asio::buffer(receive_buffer), ecode);

				if (ecode) throw boost::system::system_error(ecode);

				// 步骤 10. 处理接收数据
				std::string received_data(receive_buffer.begin(),
					receive_buffer.begin() + bytes_received);
				std::cout << "Received: " << received_data << std::endl;

				// 步骤 11. 准备响应数据
				std::string response = "SERVER ACK: " + received_data;

				// 步骤 12. 同步发送响应
				size_t bytes_sent = client_socket.write_some(
					boost::asio::buffer(response), ecode);

				if (ecode) throw boost::system::system_error(ecode);

				// 步骤 13. 关闭客户端套接字(RAII自动关闭)
			}
			catch (std::exception& e) {
				std::cerr << "Connection error: " << e.what() << std::endl;
			}
		}
	}
	catch (std::exception& e) {
		std::cerr << "Server exception: " << e.what() << std::endl;
	}
}

extern void RunClient() {
	// 步骤 1. 创建IO执行上下文
	boost::asio::io_context io_context;

	try {
		// 步骤 2. 创建解析器
		boost::asio::ip::tcp::resolver resolver(io_context);

		// 步骤 3. 解析服务器地址
		boost::asio::ip::tcp::resolver::results_type endpoints =
			resolver.resolve("127.0.0.1", "8080");

		// 步骤 4. 创建客户端套接字
		boost::asio::ip::tcp::socket client_socket(io_context);

		// 步骤 5. 连接服务器
		boost::asio::connect(client_socket, endpoints);
		std::cout << "Connected to server" << std::endl;

		// 步骤 6. 准备发送数据
		std::string send_data = "HELLO SERVER";

		// 步骤 7. 同步发送数据
		boost::system::error_code ecode;
		size_t bytes_sent = client_socket.write_some(
			boost::asio::buffer(send_data), ecode);

		if (ecode) throw boost::system::system_error(ecode);

		// 步骤 8. 创建接收缓冲区
		std::vector<char> receive_buffer(1024);

		// 步骤 9. 同步接收响应
		size_t bytes_received = client_socket.read_some(
			boost::asio::buffer(receive_buffer), ecode);

		if (ecode) throw boost::system::system_error(ecode);

		// 步骤 10. 处理响应数据
		std::string received_data(receive_buffer.begin(),
			receive_buffer.begin() + bytes_received);
		std::cout << "Received: " << received_data << std::endl;

		// 步骤 11. 关闭套接字(RAII自动关闭)
	}
	catch (std::exception& e) {
		std::cerr << "Client exception: " << e.what() << std::endl;
	}
}

extern void UseConstBuffer() {
	std::string buffer = "Hello, world!";
	/*
	boost::asio::const_buffer cbuffer(buffer.data(), buffer.size());
	std::vector<boost::asio::const_buffer> buffer_sequence{ cbuffer };
	*/
	/*
	boost::asio::const_buffer cbuffer = boost::asio::buffer(buffer);
	*/
	constexpr size_t buffer_size = 1024;
	std::unique_ptr<char[]> buffer_ptr(new char[buffer_size]);
	auto cbuffer = boost::asio::buffer(static_cast<void*>(buffer_ptr.get()), buffer_size);
}

extern void WriteToSocket(boost::asio::ip::tcp::socket& socket) {
	std::string send_data = "Hello, world!";
	size_t total_bytes_wriiten = 0;
	boost::system::error_code ecode;
	// 循环写入，write_some返回每次写入的字节数
	// send_data可能一次不能发送完，每次发送一部分，total_bytes_wriiten用来记录已经发送的part长度
	while (total_bytes_wriiten != send_data.length()) {
		total_bytes_wriiten += socket.write_some(
			boost::asio::buffer(send_data.c_str() + total_bytes_wriiten,
				send_data.length() - total_bytes_wriiten), ecode);
	}
}

extern int SendDataByXX() {
	std::string raw_ip_address = "127.0.0.1";
	unsigned short port = 8080;
	try {
		boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address(raw_ip_address), port);
		boost::asio::io_context io_context;
		boost::asio::ip::tcp::socket socket(io_context, endpoint.protocol());
		socket.connect(endpoint);

		//WriteToSocket(socket); //SendDataByWriteSome
		
		// SendDataBySend
		//如果TCP没发完给定的数据，那么一直阻塞直至发完,啥时候发完TCP你告诉我
		//int sent_length = socket.send(boost::asio::buffer("Hello, world!"));
		//if (sent_length <= 0) { //发送失败
		//	return 0;
		//}

		// SendDataByWrite
		int sent_length = boost::asio::write(socket, boost::asio::buffer("Hello, world!"));
		//if (sent_length <= 0) { //发送失败
		//	return 0;
		//}
	}
	catch (boost::system::system_error& e) {
		std::cerr << "Failed to connect to endpoint. Error Code: " << e.code().value() << "Message: " << e.what() << std::endl;
		return e.code().value();
	}
	return 0;
}

//同步读
extern std::string ReadFromSocket(boost::asio::ip::tcp::socket& socket) {
	constexpr size_t buffer_size = 1024;
	char buffer[buffer_size];
	size_t total_bytes_read = 0;
	while (total_bytes_read != buffer_size) {
		size_t bytes_read = socket.read_some(
			boost::asio::buffer(buffer + total_bytes_read, buffer_size - total_bytes_read));
		total_bytes_read += bytes_read;
	}
	return std::string(buffer, total_bytes_read);
}

extern int ReadDataByReadSome() {
	std::string raw_ip_address = "127.0.0.1";
	unsigned short port = 8080;
	try {
		boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address(raw_ip_address), port);
		boost::asio::io_context io_context;
		boost::asio::ip::tcp::socket socket(io_context, endpoint.protocol());
		socket.connect(endpoint);

		// 读写读写，太麻烦
		std::string received_data = ReadFromSocket(socket);
		std::cout << "Received: " << received_data << std::endl;
	
		// 一次性读完再写
		constexpr size_t buffer_size = 1024;
		char buffer[buffer_size];
		//size_t bytes_read = socket.receive(boost::asio::buffer(buffer, buffer_size));
		// 等价于
		size_t bytes_read = boost::asio::read(socket, boost::asio::buffer(buffer, buffer_size));
		if (bytes_read <= 0) { //接收失败
			return 0;
		}
	}
	catch (boost::system::system_error& e) {
		std::cerr << "Failed to connect to endpoint. Error Code: " << e.code().value() << "Message: " << e.what() << std::endl;
		return e.code().value();
	}
	return 0;
}