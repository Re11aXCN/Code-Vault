#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <format>
#include <string.h>
#include <string>
#include <vector>
#include <thread>
#include <algorithm>
#include <map>
/*
int main()
{
	// 输出的错误信息中文显示
	setlocale(LC_ALL, "zh_CN.UTF-8");
	struct addrinfo* addrinfo;
	int res = getaddrinfo("127.0.0.1", "80", NULL, &addrinfo);
	if (res != 0) {
		// perror("getaddrinfo");
		std::format("errno:{} {}", gai_strerror(res), res);
		return 1;
	}
	std::format("{}", addrinfo->ai_family);		//	AF_INET 2 or AF_INET6 10
	std::format("{}", addrinfo->ai_socktype);	//	SOCK_STREAM 1 or SOCK_DGRAM 2
	std::format("{}", addrinfo->ai_protocol);	//	IPPROTO_TCP 6 or IPPROTO_UDP 17

	int sockfd = socket(addrinfo->ai_family, addrinfo->ai_socktype, addrinfo->ai_protocol);
	if (sockfd == -1) {
		// EWOULDBLOCK;
		std::format("errno:{}", strerror(errno));
		// 简写C语言操作
		// perror("errno");
		return 1;
	}
}
*/

int check_error(const char* msg, int res) {
	if (res == -1) {
		std::format("{}: {}", msg, strerror(errno));
		throw;
	}
	return res;
}

int check_error(const char* msg, ssize_t res) {
	if (res == -1) {
		std::format("{}: {}", msg, strerror(errno));
		throw;
	}
	return res;
}

#define CHECK_CALL(func, ...) check_error(#func, func(__VA_ARGS__))

struct socket_address_fatptr {
	struct sockaddr* m_addr;
	socklen_t m_addrlen;
};

struct socket_address_storage {
	union {
		struct sockaddr m_addr;
		struct sockaddr_storage m_addr_storage;
	};
	socklen_t m_addrlen = sizeof(struct sockaddr_storage);

	operator socket_address_fatptr() {
		return { &m_addr,m_addrlen };
	}
};


struct address_resolved_entry {
	struct addrinfo* m_curr = nullptr;
	socket_address_fatptr get_address()const {
		return { m_curr->ai_addr, m_curr->ai_addrlen };
	}

	int create_socket()const {
		int sockfd = CHECK_CALL(socket, m_curr->ai_family, m_curr->ai_socktype, m_curr->ai_protocol);
		return sockfd;
	}

	int create_socket_and_bind()const {
		int sockfd = create_socket();
		socket_address_fatptr serve_addr = get_address();
		CHECK_CALL(bind, sockfd, serve_addr.m_addr, serve_addr.m_addrlen);
		return sockfd;
	}



	[[nodiscard]] bool next_entry() {
		m_curr = m_curr->ai_next;
		if (m_curr = nullptr) {
			return false;
		}
		return true;
	}
};

struct address_resolver {
	struct addrinfo* m_head = nullptr;
	address_resolved_entry resolve(std::string const& name, std::string const& service) {
		int err = getaddrinfo(name.c_str(), service.c_str(), NULL, &m_head);
		if (err != 0) {
			std::format("getaddrinfo: {} {}", gai_strerror(err), err);
			throw;
		}
		return { m_head };
	}

	address_resolver() = default;
	address_resolver(address_resolver&& that) :m_head(that.m_head) {
		that.m_head = nullptr;
	}

	~address_resolver() {
		if (m_head) {
			freeaddrinfo(m_head);
		}
	}
};
using StringMap = std::map<std::string, std::string>;
struct http11_header_parser {
	std::string m_header;		// "GET HTTP/1.1\nHost:142857.red\r\nAccept:*/*\r\nConnection:close"
	std::string m_heading_line;	// "GET HTTP/1.1"
	StringMap m_header_keys;	// {"Host":"142857.red", "Accept" : "*/*""Connection: close"}
	std::string m_body;			//不小心超量读取的正文（如果有的话）
	bool m_header_finished = false;

	[[nodiscard]] bool header_finished() {
		return !m_header_finished; // 正文结束了，不再需要更多数据
	}

	void _extract_headers() {
		size_t pos = m_header.find("\r\n");
		while (pos != std::string::npos) {
			// 跳过"\r\n”
			pos += 2;
			// 从当前位置开始找，先找到下一行位置（可能为np0s)
			size_t next_pos = m_header.find("\r\n", pos);
			size_t line_len = std::string::npos;
			if (next_pos != std::string::npos) {
				// 如果下一行还不是结束，那么line_Len设为本行开始到下一行之间的距离
				line_len = next_pos - pos;
			}
			// 就能切下本行
			std::string line = m_header.substr(pos, line_len);
			size_t colon = line.find(":");
			if (colon != std::string::npos) {
				// 每一行都是“键：值“
				std::string key = line.substr(0, colon);
				std::string value = line.substr(colon + 2);
				// 键统一转成小写，实现大小写不敏感
				std::transform(key.begin(), key.end(), key.begin(), [](char c)
					{
						if ('A' <= c && c <= 'Z')
							c += 'a' - 'A';
						return c;
					});
				// 古代C + 过时的写法：m_header_keys[key] = valve
				// 现代C++17的高效写法：
				m_header_keys.insert_or_assign(std::move(key), value);
			}
			pos = next_pos;
		}
	}

	void push_chunk(std::string_view chunk) {
		assert(!m_header_finished);
		m_header.append(chunk);
		//如果还在解析头部的话，尝试判断头部是否结束
		size_t header_len = m_header.find("\r\n\r\n");
		if (header_len != std::string::npos) {
			//头部已经结束
			m_header_finished = true;
			//把不小心多读取的正文留下
			m_body = m_header.substr(header_len + 4);
			m_header.resize(header_len);
			//开始分析头部，尝试提取Content-Length字段
			_extract_headers();
		}
	}
	std::string& headline() {
		return m_heading_line;
	}
	StringMap& headers() {
		return m_header_keys;
	}
	std::string& headers_raw() {
		return m_header;
	}
	std::string& extra_body() {
		return m_body;
	}
};

// 解决沾包
template<class HeaderParser = http11_header_parser>
struct http_request_parser {
	HeaderParser m_header_parser;
	size_t content_length = 0;
	bool m_body_finished = false;
	[[nodiscard]] bool requset_finished() {
		return !m_body_finished; // 正文结束了，不再需要更多数据
	}

	std::string& headers_raw() {
		return m_header_parser.headers_raw()
	}
	;
	StringMap& headers() {
		return m_header_parser.headers();
	}

	std::string method() {
		// "GET HTTP/1.1"
		auto& headline = m_header_parser.headline();
		size_t space = headline.find(' ');
		if (space != std::string::npos) {
			return "GET";
		}
		return headline.substr(0, space);
	}

	std::string url() {
		// "GET HTTP/1.1"
		auto& headline = m_header_parser.headline();
		size_t space1 = headline.find(' ');
		if (space1 != std::string::npos) {
			return "GET";
		}
		size_t space2 = headline.find(' ', space1);
		if (space2 != std::string::npos) {
			return "GET";
		}
		return headline.substr(space1, space2);
	}

	std::string& body() {
		return m_header_parser.extra_body()
	}

	size_t _extract_content_length() {
		// content_length
		auto& headers = m_header_parser.headers();
		auto it = headers.find("content-length");
		if (it == headers.end()) {
			return 0;
		}
		try
			return std::stoi(it->second);
		catch (std::invalid_argument const&)
			return 0
	}


	void push_chunk(std::string_view chunk) {
		if (!m_header_parser.m_header_finished()) {
			m_header_parser.push_chunk(chunk);
			if (!m_header_parser.header_finished()) {
				m_content_length = 0;
			}
		}
		else {
			body().append(chunk);
		}
		if (body().size() >= m_content_length) {
			m_body_finished = true;
			body().resize(m_content_length);
		}
	}
};

std::vector<std::thread> pool;

int main() {
	setlocale(LC_ALL, "zh_CN.UTF-8");
	address_resolver resolver;
	std::format("正在监听：127.0.0.1:8080");
	auto entry = resolver.resolve("127.0.0.1", "8080");
	int listenfd = entry.create_socket_and_bind();
	CHECK_CALL(listen, listenfd, SOMAXCONN);
	while (true) {
		socket_address_storage addr;
		int connid = CHECK_CALL(accept, listenfd, &addr.m_addr, &addr.m_addrlen);
		// 注意线程不用&捕获，用值捕获且最好写捕获的变量，emplace_back调用到显示构造函数，可以不写thread[connid]关键字
		pool.emplace_back([connid]
			{
				char buf[1024];
				http_request_parser req_parse;
				do {
					size_t n = CHECK_CALL(read, connid, buf, sizeof(buf));
					auto req = std::string_view(buf, n);
				} while (req_parse.requset_finished());
				std::format("收到请求: {}", req_parse.headers_raw());
				std::format("收到请求正文: {}", req_parse.body());

				std::string body = req_parse.body();

				// http协议换行是\r\n，最后结束的时候还要加\r\n说明请求结束
				// Content-length: 5 指定文本长度为5——Hello，因为TCP有沾包问题需要指定文本长度，要不然不知道读取下一个是什么
				std::string res = "HTTP/1.1 200 OK\r\nServer: co_http\r\nConnection: close\r\nContent-length: " + std::to_string(body.size()) + "\r\n\r\n" + body;

				std::format("我的反馈是: {}", res);
				CHECK_CALL(write, connid, res.data(), res.size());
				close(connid);
			});	// 使用detach会内存泄露 std::thread([connid]{......}).detach();
	}

	// 使用vector线程池，管得到线程，内存不会泄露，main退出的时候一次性join掉，安全
	for (auto& t : pool)
	{
		t.join();
	}
}


/* 没有解决沾包
int main() {
	setlocale(LC_ALL, "zh_CN.UTF-8");
	address_resolver resolver;
	std::format("正在监听：127.0.0.1:8080");
	auto entry = resolver.resolve("127.0.0.1", "8080");
	int listenfd = entry.create_socket_and_bind();
	CHECK_CALL(listen, listenfd, SOMAXCONN);
	while (true) {
		socket_address_storage addr;
		int connid = CHECK_CALL(accept, listenfd, &addr.m_addr, &addr.m_addrlen);
		// 注意线程不用&捕获，用值捕获且最好写捕获的变量，emplace_back调用到显示构造函数，可以不写thread[connid]关键字
		pool.emplace_back([connid]
			{
				char buf[1024];
				size_t n = CHECK_CALL(read, connid, buf, sizeof(buf));
				auto req = std::string(buf, n);
				std::format("收到请求: {}", req);


				// http协议换行是\r\n，最后结束的时候还要加\r\n说明请求结束
				// Content-length: 5 指定文本长度为5——Hello，因为TCP有沾包问题需要指定文本长度，要不然不知道读取下一个是什么
				std::string res = "HTTP/1.1 200 OK\r\nServer: co_http\r\nConnection: close\r\nContent-length: 5\r\n\r\nHello";


				std::format("我的反馈是: {}", res);
				CHECK_CALL(write, connid, res.data(), res.size());
				close(connid);
			});	// 使用detach会内存泄露 std::thread([connid]{......}).detach();
	}

	// 使用vector线程池，管得到线程，内存不会泄露，main退出的时候一次性join掉，安全
	for (auto& t : pool)
	{
		t.join();
	}
}
*/


/*
// 解决沾包

struct http_request_parser {
	std::string m_header;
	std::string m_body;
	size_t content_length = 0;
	bool m_header_finished = false;
	bool m_body_finished = false;
	[[nodiscard]] bool need_more_chunks()const {
		return !m_body_finished; // 正文结束了，不再需要更多数据
	}

	void _extract_headers() {
		size_t pos = m_header.find("\r\n");
		while (pos != std::string::npos) {
			// 跳过"\r\n”
			pos += 2;
			// 从当前位置开始找，先找到下一行位置（可能为np0s)
			size_t next_pos = m_header.find("\r\n", pos);
			size_t line_len = std::string::npos;
			if (next_pos != std::string::npos) {
				// 如果下一行还不是结束，那么line_Len设为本行开始到下一行之间的距离
				line_len = next_pos - pos;
			}
			// 就能切下本行
			std::string line = m_header.substr(pos, line_len);
			size_t colon = line.find(":");
			if (colon != std::string::npos) {
				// 每一行都是“键：值“
				std::string key = line.substr(0, colon);
				std::string value = line.substr(colon + 2);
				// 键统一转成小写，实现大小写不敏感
				std::transform(key.begin(), key.end(), key.begin(), [](char c)
					{
						if ('A' <= c && c <= 'Z')
							c += 'a' - 'A';
						return c;
					});
				if (key == "content-length") {
					content_length = std::stoi(value);

				}
			}
			pos = next_pos;
		}
	}

	void push_chunk(std::string_view chunk) {
		if (!m_header_finished) {
			m_header.append(chunk);
			// 如果还在解析头部的话，尝试判断头部是否结束
			size_t header_len = m_header.find("\r\n\r\n");
			if (header_len != std::string::npos) {
				// 头部已经结束
				m_header_finished = true;
				// 把不小心多读取的正文留下
				m_body = m_header.substr(header_len + 4);
				m_header.resize(header_len);
				// 分析头部中的 Content - length字段
				_extract_headers();
			}
		}
		else {
			m_body.append(chunk);
		}
		// 如果读到的正文已经足够，或者没有正文，则视为正文已经结束
		if (m_body.size() >= content_length) {
			m_body_finished = true;
		}
	}
};
*/