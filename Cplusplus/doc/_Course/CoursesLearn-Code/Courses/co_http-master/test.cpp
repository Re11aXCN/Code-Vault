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
	// ����Ĵ�����Ϣ������ʾ
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
		// ��дC���Բ���
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
	std::string m_body;			//��С�ĳ�����ȡ�����ģ�����еĻ���
	bool m_header_finished = false;

	[[nodiscard]] bool header_finished() {
		return !m_header_finished; // ���Ľ����ˣ�������Ҫ��������
	}

	void _extract_headers() {
		size_t pos = m_header.find("\r\n");
		while (pos != std::string::npos) {
			// ����"\r\n��
			pos += 2;
			// �ӵ�ǰλ�ÿ�ʼ�ң����ҵ���һ��λ�ã�����Ϊnp0s)
			size_t next_pos = m_header.find("\r\n", pos);
			size_t line_len = std::string::npos;
			if (next_pos != std::string::npos) {
				// �����һ�л����ǽ�������ôline_Len��Ϊ���п�ʼ����һ��֮��ľ���
				line_len = next_pos - pos;
			}
			// �������±���
			std::string line = m_header.substr(pos, line_len);
			size_t colon = line.find(":");
			if (colon != std::string::npos) {
				// ÿһ�ж��ǡ�����ֵ��
				std::string key = line.substr(0, colon);
				std::string value = line.substr(colon + 2);
				// ��ͳһת��Сд��ʵ�ִ�Сд������
				std::transform(key.begin(), key.end(), key.begin(), [](char c)
					{
						if ('A' <= c && c <= 'Z')
							c += 'a' - 'A';
						return c;
					});
				// �Ŵ�C + ��ʱ��д����m_header_keys[key] = valve
				// �ִ�C++17�ĸ�Чд����
				m_header_keys.insert_or_assign(std::move(key), value);
			}
			pos = next_pos;
		}
	}

	void push_chunk(std::string_view chunk) {
		assert(!m_header_finished);
		m_header.append(chunk);
		//������ڽ���ͷ���Ļ��������ж�ͷ���Ƿ����
		size_t header_len = m_header.find("\r\n\r\n");
		if (header_len != std::string::npos) {
			//ͷ���Ѿ�����
			m_header_finished = true;
			//�Ѳ�С�Ķ��ȡ����������
			m_body = m_header.substr(header_len + 4);
			m_header.resize(header_len);
			//��ʼ����ͷ����������ȡContent-Length�ֶ�
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

// ���մ��
template<class HeaderParser = http11_header_parser>
struct http_request_parser {
	HeaderParser m_header_parser;
	size_t content_length = 0;
	bool m_body_finished = false;
	[[nodiscard]] bool requset_finished() {
		return !m_body_finished; // ���Ľ����ˣ�������Ҫ��������
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
	std::format("���ڼ�����127.0.0.1:8080");
	auto entry = resolver.resolve("127.0.0.1", "8080");
	int listenfd = entry.create_socket_and_bind();
	CHECK_CALL(listen, listenfd, SOMAXCONN);
	while (true) {
		socket_address_storage addr;
		int connid = CHECK_CALL(accept, listenfd, &addr.m_addr, &addr.m_addrlen);
		// ע���̲߳���&������ֵ���������д����ı�����emplace_back���õ���ʾ���캯�������Բ�дthread[connid]�ؼ���
		pool.emplace_back([connid]
			{
				char buf[1024];
				http_request_parser req_parse;
				do {
					size_t n = CHECK_CALL(read, connid, buf, sizeof(buf));
					auto req = std::string_view(buf, n);
				} while (req_parse.requset_finished());
				std::format("�յ�����: {}", req_parse.headers_raw());
				std::format("�յ���������: {}", req_parse.body());

				std::string body = req_parse.body();

				// httpЭ�黻����\r\n����������ʱ��Ҫ��\r\n˵���������
				// Content-length: 5 ָ���ı�����Ϊ5����Hello����ΪTCP��մ��������Ҫָ���ı����ȣ�Ҫ��Ȼ��֪����ȡ��һ����ʲô
				std::string res = "HTTP/1.1 200 OK\r\nServer: co_http\r\nConnection: close\r\nContent-length: " + std::to_string(body.size()) + "\r\n\r\n" + body;

				std::format("�ҵķ�����: {}", res);
				CHECK_CALL(write, connid, res.data(), res.size());
				close(connid);
			});	// ʹ��detach���ڴ�й¶ std::thread([connid]{......}).detach();
	}

	// ʹ��vector�̳߳أ��ܵõ��̣߳��ڴ治��й¶��main�˳���ʱ��һ����join������ȫ
	for (auto& t : pool)
	{
		t.join();
	}
}


/* û�н��մ��
int main() {
	setlocale(LC_ALL, "zh_CN.UTF-8");
	address_resolver resolver;
	std::format("���ڼ�����127.0.0.1:8080");
	auto entry = resolver.resolve("127.0.0.1", "8080");
	int listenfd = entry.create_socket_and_bind();
	CHECK_CALL(listen, listenfd, SOMAXCONN);
	while (true) {
		socket_address_storage addr;
		int connid = CHECK_CALL(accept, listenfd, &addr.m_addr, &addr.m_addrlen);
		// ע���̲߳���&������ֵ���������д����ı�����emplace_back���õ���ʾ���캯�������Բ�дthread[connid]�ؼ���
		pool.emplace_back([connid]
			{
				char buf[1024];
				size_t n = CHECK_CALL(read, connid, buf, sizeof(buf));
				auto req = std::string(buf, n);
				std::format("�յ�����: {}", req);


				// httpЭ�黻����\r\n����������ʱ��Ҫ��\r\n˵���������
				// Content-length: 5 ָ���ı�����Ϊ5����Hello����ΪTCP��մ��������Ҫָ���ı����ȣ�Ҫ��Ȼ��֪����ȡ��һ����ʲô
				std::string res = "HTTP/1.1 200 OK\r\nServer: co_http\r\nConnection: close\r\nContent-length: 5\r\n\r\nHello";


				std::format("�ҵķ�����: {}", res);
				CHECK_CALL(write, connid, res.data(), res.size());
				close(connid);
			});	// ʹ��detach���ڴ�й¶ std::thread([connid]{......}).detach();
	}

	// ʹ��vector�̳߳أ��ܵõ��̣߳��ڴ治��й¶��main�˳���ʱ��һ����join������ȫ
	for (auto& t : pool)
	{
		t.join();
	}
}
*/


/*
// ���մ��

struct http_request_parser {
	std::string m_header;
	std::string m_body;
	size_t content_length = 0;
	bool m_header_finished = false;
	bool m_body_finished = false;
	[[nodiscard]] bool need_more_chunks()const {
		return !m_body_finished; // ���Ľ����ˣ�������Ҫ��������
	}

	void _extract_headers() {
		size_t pos = m_header.find("\r\n");
		while (pos != std::string::npos) {
			// ����"\r\n��
			pos += 2;
			// �ӵ�ǰλ�ÿ�ʼ�ң����ҵ���һ��λ�ã�����Ϊnp0s)
			size_t next_pos = m_header.find("\r\n", pos);
			size_t line_len = std::string::npos;
			if (next_pos != std::string::npos) {
				// �����һ�л����ǽ�������ôline_Len��Ϊ���п�ʼ����һ��֮��ľ���
				line_len = next_pos - pos;
			}
			// �������±���
			std::string line = m_header.substr(pos, line_len);
			size_t colon = line.find(":");
			if (colon != std::string::npos) {
				// ÿһ�ж��ǡ�����ֵ��
				std::string key = line.substr(0, colon);
				std::string value = line.substr(colon + 2);
				// ��ͳһת��Сд��ʵ�ִ�Сд������
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
			// ������ڽ���ͷ���Ļ��������ж�ͷ���Ƿ����
			size_t header_len = m_header.find("\r\n\r\n");
			if (header_len != std::string::npos) {
				// ͷ���Ѿ�����
				m_header_finished = true;
				// �Ѳ�С�Ķ��ȡ����������
				m_body = m_header.substr(header_len + 4);
				m_header.resize(header_len);
				// ����ͷ���е� Content - length�ֶ�
				_extract_headers();
			}
		}
		else {
			m_body.append(chunk);
		}
		// ��������������Ѿ��㹻������û�����ģ�����Ϊ�����Ѿ�����
		if (m_body.size() >= content_length) {
			m_body_finished = true;
		}
	}
};
*/