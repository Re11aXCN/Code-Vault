# CPP-HTTPLIB 库

## 1. 概述与设计理念

### 1.1 项目定位与核心特性

**cpp-httplib** 是一个现代、高性能的 C++11/14/17/20 HTTP 库，以其**单头文件**的便捷性而闻名。该库由 Yuji Hirose 开发并维护，提供了完整的 HTTP/1.1 服务器和客户端实现。

#### **核心设计特点**：
- **零依赖**（除可选的 OpenSSL、zlib、brotli、zstd）
- **线程安全**的客户端实现
- **跨平台**支持（Windows、Linux、macOS）
- **易于集成**（单头文件）
- **丰富的功能集**：路由、中间件、文件服务、SSL/TLS、压缩等

### 1.2 项目结构概览

```cpp
namespace httplib {
    // 核心数据结构
    struct Request;     // HTTP 请求
    struct Response;    // HTTP 响应
    class Server;       // HTTP 服务器
    class Client;       // HTTP 客户端
    class ClientImpl;   // 客户端实现基类
    class SSLClient;    // SSL 客户端
    class SSLServer;    // SSL 服务器
    
    // 辅助组件
    class ThreadPool;   // 线程池
    class Stream;       // 流抽象
    class TaskQueue;    // 任务队列接口
}
```

## 2. 核心数据结构详解

### 2.1 `Request` 结构体

`Request` 结构体封装了 HTTP 请求的所有信息，支持服务器端和客户端使用。

#### **主要成员**：
```cpp
struct Request {
    std::string method;                     // HTTP 方法
    std::string path;                       // 请求路径
    std::string matched_route;              // 匹配的路由模式
    Params params;                          // 查询参数
    Headers headers;                        // 请求头
    std::string body;                       // 请求体
    
    // 网络信息
    std::string remote_addr;
    int remote_port = -1;
    std::string local_addr;
    int local_port = -1;
    
    // 服务器特有
    std::string version;
    std::string target;
    MultipartFormDataMap files;            // 上传文件
    Ranges ranges;                         // Range 请求
    Match matches;                         // 正则匹配结果
    std::unordered_map<std::string, std::string> path_params; // 路径参数
    
    // 客户端特有
    ResponseHandler response_handler;
    ContentReceiverWithProgress content_receiver;
    Progress progress;
    
    // 方法...
};
```

#### **关键方法**：

1. **头部操作**：
```cpp
bool has_header(const std::string& key) const;
std::string get_header_value(const std::string& key, 
                             const char* def = "", 
                             size_t id = 0) const;
void set_header(const std::string& key, const std::string& val);
```

2. **参数操作**：
```cpp
bool has_param(const std::string& key) const;
std::string get_param_value(const std::string& key, size_t id = 0) const;
```

3. **文件上传处理**：
```cpp
bool is_multipart_form_data() const;
bool has_file(const std::string& key) const;
MultipartFormData get_file_value(const std::string& key) const;
```

### 2.2 `Response` 结构体

`Response` 结构体用于构建 HTTP 响应，支持多种内容设置方式。

#### **内容设置方法**：

1. **直接设置文本内容**：
```cpp
void set_content(const char* s, size_t n, const std::string& content_type);
void set_content(const std::string& s, const std::string& content_type);
void set_content(std::string&& s, const std::string& content_type);
```

2. **使用内容提供器**（流式响应）：
```cpp
// 已知长度的内容提供器
void set_content_provider(size_t length, 
                          const std::string& content_type,
                          ContentProvider provider,
                          ContentProviderResourceReleaser resource_releaser = nullptr);

// 未知长度的内容提供器（分块传输）
void set_content_provider(const std::string& content_type,
                          ContentProviderWithoutLength provider,
                          ContentProviderResourceReleaser resource_releaser = nullptr);

// 分块传输编码
void set_chunked_content_provider(const std::string& content_type,
                                  ContentProviderWithoutLength provider,
                                  ContentProviderResourceReleaser resource_releaser = nullptr);
```

3. **文件内容响应**：
```cpp
void set_file_content(const std::string& path, 
                      const std::string& content_type = "");
```

### 2.3 `Headers` 类型与设计

#### **不区分大小写的头部映射**：
```cpp
using Headers = std::unordered_multimap<
    std::string, std::string,
    detail::case_ignore::hash,
    detail::case_ignore::equal_to>;
```

**设计亮点**：
- 使用自定义哈希和相等比较器实现**大小写不敏感**的头部查找
- 支持**多值头部**（如 `Set-Cookie`）
- 符合 HTTP/1.1 规范对头部的处理要求

### 2.4 `MultipartFormData` 结构

```cpp
struct MultipartFormData {
    std::string name;         // 字段名
    std::string content;      // 内容
    std::string filename;     // 文件名
    std::string content_type; // MIME 类型
    Headers headers;          // 额外的头部
};
```

## 3. 服务器端（Server）架构设计

### 3.1 Server 类核心架构

#### **3.1.1 处理流程设计**

```cpp
class Server {
public:
    // 主处理循环
    bool process_and_close_socket(socket_t sock);
    
private:
    // 请求处理流程
    bool routing(Request& req, Response& res, Stream& strm);
    bool process_request(Stream& strm, ...);
    bool write_response(Stream& strm, ...);
    
    // 路由表
    Handlers get_handlers_;
    Handlers post_handlers_;
    HandlersForContentReader post_handlers_for_content_reader_;
    // ... 其他方法的处理器
};
```

#### **3.1.2 线程模型**

```cpp
class ThreadPool final : public TaskQueue {
private:
    std::vector<std::thread> threads_;
    std::list<std::function<void()>> jobs_;
    std::condition_variable cond_;
    std::mutex mutex_;
    
    // 工作线程逻辑
    void operator()() {
        while (!shutdown_) {
            std::function<void()> fn;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cond_.wait(lock, [&] { 
                    return !jobs_.empty() || shutdown_; 
                });
                if (shutdown_ && jobs_.empty()) break;
                fn = jobs_.front();
                jobs_.pop_front();
            }
            fn(); // 执行任务
        }
    }
};
```

**线程池特点**：
- 动态任务队列
- 优雅关闭机制
- 可配置的最大队列长度

### 3.2 路由系统设计

#### **3.2.1 路由匹配器抽象**

```cpp
class MatcherBase {
public:
    virtual bool match(Request& request) const = 0;
    virtual ~MatcherBase() = default;
};

// 路径参数匹配器（如 /users/:id）
class PathParamsMatcher : public MatcherBase {
private:
    std::vector<std::string> static_fragments_;
    std::vector<std::string> param_names_;
};

// 正则表达式匹配器
class RegexMatcher : public MatcherBase {
private:
    std::regex regex_;
};
```

#### **3.2.2 路由注册 API**

```cpp
// 基本路由
Server& Get(const std::string& pattern, Handler handler);
Server& Post(const std::string& pattern, Handler handler);
Server& Put(const std::string& pattern, Handler handler);
Server& Delete(const std::string& pattern, Handler handler);
Server& Patch(const std::string& pattern, Handler handler);
Server& Options(const std::string& pattern, Handler handler);

// 支持内容读取器的路由
Server& Post(const std::string& pattern, 
             HandlerWithContentReader handler);
```

### 3.3 中间件与处理器系统

#### **3.3.1 处理器类型**

1. **前置路由处理器**：
```cpp
Server& set_pre_routing_handler(HandlerWithResponse handler);
```
在路由匹配前执行，可用于身份验证、日志记录等

2. **后置路由处理器**：
```cpp
Server& set_post_routing_handler(Handler handler);
```
在路由处理后执行，可用于添加公共头部等

3. **预请求处理器**：
```cpp
Server& set_pre_request_handler(HandlerWithResponse handler);
```
在具体处理器执行前调用

4. **错误处理器**：
```cpp
template<class ErrorHandlerFunc>
Server& set_error_handler(ErrorHandlerFunc&& handler);
```

5. **异常处理器**：
```cpp
Server& set_exception_handler(ExceptionHandler handler);
```

#### **3.3.2 处理器链执行顺序**

```
pre_routing_handler → 路由匹配 → pre_request_handler → 
请求处理器 → post_routing_handler → 错误处理 → 响应写入
```

### 3.4 静态文件服务

#### **3.4.1 挂载点配置**

```cpp
bool set_mount_point(const std::string& mount_point,
                     const std::string& dir,
                     Headers headers = Headers());
```

**示例**：
```cpp
server.set_mount_point("/static", "./public", {
    {"Cache-Control", "max-age=3600"}
});
```

#### **3.4.2 文件扩展名映射**

```cpp
Server& set_file_extension_and_mimetype_mapping(
    const std::string& ext,
    const std::string& mime);
```

### 3.5 配置系统

#### **3.5.1 超时配置**

```cpp
// 读取超时
Server& set_read_timeout(time_t sec, time_t usec = 0);
template<class Rep, class Period>
Server& set_read_timeout(const std::chrono::duration<Rep, Period>& duration);

// 写入超时
Server& set_write_timeout(time_t sec, time_t usec = 0);

// 空闲间隔
Server& set_idle_interval(time_t sec, time_t usec = 0);
```

#### **3.5.2 连接管理**

```cpp
// 保持连接设置
Server& set_keep_alive_max_count(size_t count);  // 最大请求数
Server& set_keep_alive_timeout(time_t sec);      // 超时时间

// TCP 优化
Server& set_tcp_nodelay(bool on);      // 禁用 Nagle 算法
Server& set_address_family(int family); // AF_INET/AF_INET6/AF_UNSPEC
```

#### **3.5.3 安全限制**

```cpp
Server& set_payload_max_length(size_t length);  // 最大请求体大小
Server& set_default_headers(Headers headers);   // 默认响应头
```

## 4. 客户端端（Client）架构设计

### 4.1 客户端类层次结构

```
ClientImpl (基类)
    ├── Client (通用客户端)
    └── SSLClient (SSL客户端)
```

### 4.2 连接管理与线程安全

#### **4.2.1 套接字管理**

```cpp
class ClientImpl {
protected:
    struct Socket {
        socket_t sock = INVALID_SOCKET;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        SSL* ssl = nullptr;
#endif
        bool is_open() const { return sock != INVALID_SOCKET; }
    };
    
    Socket socket_;
    mutable std::mutex socket_mutex_;
    std::recursive_mutex request_mutex_;
    
    // 线程安全计数器
    size_t socket_requests_in_flight_ = 0;
    std::thread::id socket_requests_are_from_thread_ = std::thread::id();
    bool socket_should_be_closed_when_request_is_done_ = false;
};
```

#### **4.2.2 请求发送流程**

```cpp
bool ClientImpl::send(Request& req, Response& res, Error& error) {
    std::lock_guard<std::recursive_mutex> request_mutex_guard(request_mutex_);
    
    {
        std::lock_guard<std::mutex> guard(socket_mutex_);
        // 检查套接字状态，必要时重建连接
        if (!socket_.is_open() || !detail::is_socket_alive(socket_.sock)) {
            // 重新创建连接
            if (!create_and_connect_socket(socket_, error)) {
                return false;
            }
        }
        
        // 标记套接字正在使用
        socket_requests_in_flight_ += 1;
        socket_requests_are_from_thread_ = std::this_thread::get_id();
    }
    
    // 发送请求并处理响应
    auto ret = process_socket(socket_, req.start_time_, 
        [&](Stream& strm) {
            return handle_request(strm, req, res, close_connection, error);
        });
    
    // 清理
    {
        std::lock_guard<std::mutex> guard(socket_mutex_);
        socket_requests_in_flight_ -= 1;
        if (socket_should_be_closed_when_request_is_done_ || 
            close_connection || !ret) {
            shutdown_ssl(socket_, true);
            shutdown_socket(socket_);
            close_socket(socket_);
        }
    }
    
    return ret;
}
```

### 4.3 请求构建与发送

#### **4.3.1 便捷 API 设计**

```cpp
// GET 请求
Result Get(const std::string& path);
Result Get(const std::string& path, const Headers& headers);
Result Get(const std::string& path, ContentReceiver content_receiver);
Result Get(const std::string& path, Progress progress);

// POST 请求
Result Post(const std::string& path);
Result Post(const std::string& path, const std::string& body,
            const std::string& content_type);
Result Post(const std::string& path, size_t content_length,
            ContentProvider content_provider,
            const std::string& content_type);
```

#### **4.3.2 内容提供器支持**

```cpp
Result ClientImpl::send_with_content_provider(
    const std::string& method,
    const std::string& path,
    const Headers& headers,
    const char* body,
    size_t content_length,
    ContentProvider content_provider,
    ContentProviderWithoutLength content_provider_without_length,
    const std::string& content_type,
    Progress progress) {
    // 构建请求
    Request req;
    req.method = method;
    req.path = path;
    req.headers = headers;
    req.progress = progress;
    
    // 设置内容提供器
    if (content_provider) {
        req.content_length_ = content_length;
        req.content_provider_ = std::move(content_provider);
    } else if (content_provider_without_length) {
        req.content_length_ = 0;
        req.content_provider_ = detail::ContentProviderAdapter(
            std::move(content_provider_without_length));
        req.is_chunked_content_provider_ = true;
        req.set_header("Transfer-Encoding", "chunked");
    } else {
        req.body.assign(body, content_length);
    }
    
    // 发送请求
    return send_(std::move(req));
}
```

### 4.4 高级特性

#### **4.4.1 认证支持**

```cpp
// 基本认证
void set_basic_auth(const std::string& username, 
                    const std::string& password);

// Bearer Token 认证
void set_bearer_token_auth(const std::string& token);

// 摘要认证（仅 SSL）
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
void set_digest_auth(const std::string& username,
                     const std::string& password);
#endif
```

#### **4.4.2 代理支持**

```cpp
void set_proxy(const std::string& host, int port);
void set_proxy_basic_auth(const std::string& username,
                          const std::string& password);
void set_proxy_bearer_token_auth(const std::string& token);
```

#### **4.4.3 SSL/TLS 配置**

```cpp
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
// CA 证书配置
void set_ca_cert_path(const std::string& ca_cert_file_path,
                      const std::string& ca_cert_dir_path = std::string());
void set_ca_cert_store(X509_STORE* ca_cert_store);

// 证书验证
void enable_server_certificate_verification(bool enabled);
void enable_server_hostname_verification(bool enabled);
void set_server_certificate_verifier(
    std::function<SSLVerifierResponse(SSL* ssl)> verifier);
#endif
```

## 5. 网络层与流抽象

### 5.1 Stream 抽象类

```cpp
class Stream {
public:
    virtual ~Stream() = default;
    
    virtual bool is_readable() const = 0;
    virtual bool wait_readable() const = 0;
    virtual bool wait_writable() const = 0;
    
    virtual ssize_t read(char* ptr, size_t size) = 0;
    virtual ssize_t write(const char* ptr, size_t size) = 0;
    
    virtual void get_remote_ip_and_port(std::string& ip, int& port) const = 0;
    virtual void get_local_ip_and_port(std::string& ip, int& port) const = 0;
    
    virtual socket_t socket() const = 0;
    virtual time_t duration() const = 0;
};
```

### 5.2 具体实现类

#### **5.2.1 SocketStream**

```cpp
class SocketStream final : public Stream {
public:
    SocketStream(socket_t sock,
                 time_t read_timeout_sec, time_t read_timeout_usec,
                 time_t write_timeout_sec, time_t write_timeout_usec,
                 time_t max_timeout_msec = 0,
                 std::chrono::time_point<std::chrono::steady_clock> start_time = 
                     (std::chrono::steady_clock::time_point::min)());
    
private:
    socket_t sock_;
    time_t read_timeout_sec_, read_timeout_usec_;
    time_t write_timeout_sec_, write_timeout_usec_;
    std::vector<char> read_buff_;
    size_t read_buff_off_ = 0;
    size_t read_buff_content_size_ = 0;
};
```

#### **5.2.2 SSLSocketStream**

```cpp
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
class SSLSocketStream final : public Stream {
public:
    SSLSocketStream(socket_t sock, SSL* ssl,
                    time_t read_timeout_sec, time_t read_timeout_usec,
                    time_t write_timeout_sec, time_t write_timeout_usec,
                    time_t max_timeout_msec = 0,
                    std::chrono::time_point<std::chrono::steady_clock> start_time = 
                        (std::chrono::steady_clock::time_point::min)());
    
private:
    socket_t sock_;
    SSL* ssl_;
    // ... 超时配置
};
#endif
```

### 5.3 协议解析器

#### **5.3.1 请求行解析**

```cpp
bool Server::parse_request_line(const char* s, Request& req) const {
    // 格式: METHOD TARGET VERSION\r\n
    auto len = strlen(s);
    if (len < 2 || s[len - 2] != '\r' || s[len - 1] != '\n') return false;
    len -= 2;
    
    size_t count = 0;
    detail::split(s, s + len, ' ', [&](const char* b, const char* e) {
        switch (count) {
        case 0: req.method = std::string(b, e); break;
        case 1: req.target = std::string(b, e); break;
        case 2: req.version = std::string(b, e); break;
        default: break;
        }
        count++;
    });
    
    // 验证方法
    thread_local const std::set<std::string> methods{
        "GET", "HEAD", "POST", "PUT", "DELETE",
        "CONNECT", "OPTIONS", "TRACE", "PATCH", "PRI"};
        
    if (methods.find(req.method) == methods.end()) return false;
    if (req.version != "HTTP/1.1" && req.version != "HTTP/1.0") return false;
    
    // 解析路径和查询参数
    detail::divide(req.target, '?', [&](const char* lhs_data, size_t lhs_size,
                                        const char* rhs_data, size_t rhs_size) {
        req.path = detail::decode_url(std::string(lhs_data, lhs_size), false);
        detail::parse_query_text(rhs_data, rhs_size, req.params);
    });
    
    return true;
}
```

#### **5.3.2 分块传输解码**

```cpp
template <typename T>
bool read_content_chunked(Stream& strm, T& x, ContentReceiverWithProgress out) {
    const auto bufsiz = 16;
    char buf[bufsiz];
    stream_line_reader line_reader(strm, buf, bufsiz);
    
    if (!line_reader.getline()) return false;
    
    unsigned long chunk_len;
    while (true) {
        char* end_ptr;
        chunk_len = std::strtoul(line_reader.ptr(), &end_ptr, 16);
        
        if (end_ptr == line_reader.ptr()) return false;
        if (chunk_len == ULONG_MAX) return false;
        if (chunk_len == 0) break;
        
        // 读取块数据
        if (!read_content_with_length(strm, chunk_len, nullptr, out)) {
            return false;
        }
        
        // 读取块尾 CRLF
        if (!line_reader.getline()) return false;
        if (strcmp(line_reader.ptr(), "\r\n") != 0) return false;
        
        if (!line_reader.getline()) return false;
    }
    
    // 读取尾部头部
    if (!line_reader.getline()) return true;
    while (strcmp(line_reader.ptr(), "\r\n") != 0) {
        // 解析尾部头部
        parse_header(line_reader.ptr(), 
                     line_reader.ptr() + line_reader.size() - 2,
                     [&](const std::string& key, const std::string& val) {
                         x.headers.emplace(key, val);
                     });
        
        if (!line_reader.getline()) return false;
    }
    
    return true;
}
```

## 6. 压缩与编码支持

### 6.1 压缩抽象接口

```cpp
class compressor {
public:
    virtual ~compressor() = default;
    typedef std::function<bool(const char* data, size_t data_len)> Callback;
    virtual bool compress(const char* data, size_t data_length, bool last,
                          Callback callback) = 0;
};

class decompressor {
public:
    virtual ~decompressor() = default;
    virtual bool is_valid() const = 0;
    virtual bool decompress(const char* data, size_t data_length,
                            Callback callback) = 0;
};
```

### 6.2 支持的压缩算法

#### **6.2.1 GZIP 支持（需 zlib）**

```cpp
#ifdef CPPHTTPLIB_ZLIB_SUPPORT
class gzip_compressor final : public compressor {
public:
    gzip_compressor();
    ~gzip_compressor() override;
    
    bool compress(const char* data, size_t data_length, bool last,
                  Callback callback) override;
    
private:
    bool is_valid_ = false;
    z_stream strm_;
};

class gzip_decompressor final : public decompressor {
public:
    gzip_decompressor();
    ~gzip_decompressor() override;
    
    bool is_valid() const override;
    bool decompress(const char* data, size_t data_length,
                    Callback callback) override;
    
private:
    bool is_valid_ = false;
    z_stream strm_;
};
#endif
```

#### **6.2.2 Brotli 支持**

```cpp
#ifdef CPPHTTPLIB_BROTLI_SUPPORT
class brotli_compressor final : public compressor {
public:
    brotli_compressor();
    ~brotli_compressor();
    
    bool compress(const char* data, size_t data_length, bool last,
                  Callback callback) override;
    
private:
    BrotliEncoderState* state_ = nullptr;
};

class brotli_decompressor final : public decompressor {
public:
    brotli_decompressor();
    ~brotli_decompressor();
    
    bool is_valid() const override;
    bool decompress(const char* data, size_t data_length,
                    Callback callback) override;
    
private:
    BrotliDecoderResult decoder_r;
    BrotliDecoderState* decoder_s = nullptr;
};
#endif
```

#### **6.2.3 Zstandard 支持**

```cpp
#ifdef CPPHTTPLIB_ZSTD_SUPPORT
class zstd_compressor : public compressor {
public:
    zstd_compressor();
    ~zstd_compressor();
    
    bool compress(const char* data, size_t data_length, bool last,
                  Callback callback) override;
    
private:
    ZSTD_CCtx* ctx_ = nullptr;
};

class zstd_decompressor : public decompressor {
public:
    zstd_decompressor();
    ~zstd_decompressor();
    
    bool is_valid() const override;
    bool decompress(const char* data, size_t data_length,
                    Callback callback) override;
    
private:
    ZSTD_DCtx* ctx_ = nullptr;
};
#endif
```

### 6.3 自动内容类型检测

```cpp
inline std::string find_content_type(
    const std::string& path,
    const std::map<std::string, std::string>& user_data,
    const std::string& default_content_type) {
    
    auto ext = file_extension(path);
    auto it = user_data.find(ext);
    if (it != user_data.end()) return it->second;
    
    // 使用编译时计算的标签进行快速匹配
    using udl::operator""_t;
    
    switch (str2tag(ext)) {
    default: return default_content_type;
    case "css"_t: return "text/css";
    case "html"_t: return "text/html";
    case "js"_t: return "text/javascript";
    case "json"_t: return "application/json";
    case "xml"_t: return "application/xml";
    case "png"_t: return "image/png";
    case "jpg"_t:
    case "jpeg"_t: return "image/jpeg";
    case "pdf"_t: return "application/pdf";
    // ... 更多类型
    }
}
```

## 7. 错误处理与日志

### 7.1 错误码设计

```cpp
enum class Error {
    Success = 0,
    Unknown,
    Connection,
    BindIPAddress,
    Read,
    Write,
    ExceedRedirectCount,
    Canceled,
    SSLConnection,
    SSLLoadingCerts,
    SSLServerVerification,
    SSLServerHostnameVerification,
    UnsupportedMultipartBoundaryChars,
    Compression,
    ConnectionTimeout,
    ProxyConnection,
    
    // 内部使用
    SSLPeerCouldBeClosed_,
};
```

### 7.2 Result 类型

```cpp
class Result {
public:
    Result() = default;
    Result(std::unique_ptr<Response>&& res, Error err,
           Headers&& request_headers = Headers{});
    
    // 响应访问
    operator bool() const { return res_ != nullptr; }
    const Response& value() const { return *res_; }
    Response& value() { return *res_; }
    const Response& operator*() const { return *res_; }
    Response& operator*() { return *res_; }
    const Response* operator->() const { return res_.get(); }
    Response* operator->() { return res_.get(); }
    
    // 错误信息
    Error error() const { return err_; }
    
    // 请求头部访问
    bool has_request_header(const std::string& key) const;
    std::string get_request_header_value(const std::string& key,
                                         const char* def = "",
                                         size_t id = 0) const;
    
private:
    std::unique_ptr<Response> res_;
    Error err_ = Error::Unknown;
    Headers request_headers_;
};
```

### 7.3 日志系统

```cpp
using Logger = std::function<void(const Request&, const Response&)>;

// 服务器端设置
Server& set_logger(Logger logger);

// 客户端端设置
void set_logger(Logger logger);
```

## 8. 性能优化特性

### 8.1 内存优化

#### **8.1.1 零拷贝文件发送**

```cpp
void Response::set_file_content(const std::string& path,
                                const std::string& content_type) {
    file_content_path_ = path;
    file_content_content_type_ = content_type;
}

// 实际发送时使用 mmap
auto mm = std::make_shared<detail::mmap>(path.c_str());
res.set_content_provider(
    mm->size(),
    content_type,
    [mm](size_t offset, size_t length, DataSink& sink) -> bool {
        sink.write(mm->data() + offset, length);
        return true;
    });
```

#### **8.1.2 流式缓冲区**

```cpp
class stream_line_reader {
public:
    stream_line_reader(Stream& strm, char* fixed_buffer,
                       size_t fixed_buffer_size);
    
private:
    Stream& strm_;
    char* fixed_buffer_;
    const size_t fixed_buffer_size_;
    size_t fixed_buffer_used_size_ = 0;
    std::string growable_buffer_;
};
```

### 8.2 连接复用

```cpp
inline bool keep_alive(const std::atomic<socket_t>& svr_sock, 
                       socket_t sock,
                       time_t keep_alive_timeout_sec) {
    using namespace std::chrono;
    
    const auto interval_usec = 
        CPPHTTPLIB_KEEPALIVE_TIMEOUT_CHECK_INTERVAL_USECOND;
    
    // 避免首次调用时昂贵的 steady_clock::now()
    if (select_read(sock, 0, interval_usec) > 0) return true;
    
    const auto start = steady_clock::now() - microseconds{interval_usec};
    const auto timeout = seconds{keep_alive_timeout_sec};
    
    while (true) {
        if (svr_sock == INVALID_SOCKET) break; // 服务器套接字已关闭
        
        auto val = select_read(sock, 0, interval_usec);
        if (val < 0) {
            break; // 套接字错误
        } else if (val == 0) {
            if (steady_clock::now() - start > timeout) {
                break; // 超时
            }
        } else {
            return true; // 可读
        }
    }
    
    return false;
}
```

## 9. 现代 C++ 特性应用

### 9.1 类型安全的枚举

```cpp
enum StatusCode {
    // 信息响应
    Continue_100 = 100,
    SwitchingProtocol_101 = 101,
    
    // 成功响应
    OK_200 = 200,
    Created_201 = 201,
    
    // 客户端错误
    BadRequest_400 = 400,
    Unauthorized_401 = 401,
    
    // 服务器错误
    InternalServerError_500 = 500,
    NotImplemented_501 = 501,
};
```

### 9.2 RAII 资源管理

```cpp
namespace detail {
struct scope_exit {
    explicit scope_exit(std::function<void(void)>&& f)
        : exit_function(std::move(f)), execute_on_destruction{true} {}
    
    ~scope_exit() {
        if (execute_on_destruction) { this->exit_function(); }
    }
    
    void release() { this->execute_on_destruction = false; }
    
private:
    std::function<void(void)> exit_function;
    bool execute_on_destruction;
};
} // namespace detail
```

### 9.3 编译时字符串哈希

```cpp
namespace udl {
inline constexpr unsigned int operator""_t(const char* s, size_t l) {
    return str2tag_core(s, l, 0);
}
} // namespace udl

inline unsigned int str2tag(const std::string& s) {
    return str2tag_core(s.data(), s.size(), 0);
}

// 使用示例
switch (str2tag(ext)) {
case "css"_t: return "text/css";
case "html"_t: return "text/html";
// ...
}
```

## 10. 完整示例：现代 REST API 服务器与客户端

### 10.1 高级 REST API 服务器示例

```cpp
#include "httplib.h"
#include <iostream>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <optional>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace httplib;

// 数据模型
struct User {
    int id;
    std::string name;
    std::string email;
    std::string role;
    std::chrono::system_clock::time_point created_at;
    
    json to_json() const {
        return {
            {"id", id},
            {"name", name},
            {"email", email},
            {"role", role},
            {"created_at", 
             std::chrono::duration_cast<std::chrono::seconds>(
                 created_at.time_since_epoch()).count()}
        };
    }
    
    static User from_json(const json& j) {
        User user;
        user.id = j.value("id", 0);
        user.name = j.value("name", "");
        user.email = j.value("email", "");
        user.role = j.value("role", "user");
        auto timestamp = j.value("created_at", 0LL);
        user.created_at = std::chrono::system_clock::time_point(
            std::chrono::seconds(timestamp));
        return user;
    }
};

// 用户存储服务
class UserService {
private:
    std::unordered_map<int, User> users_;
    std::mutex mutex_;
    std::atomic<int> next_id_{1};
    
public:
    std::optional<User> get_user(int id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = users_.find(id);
        if (it != users_.end()) {
            return it->second;
        }
        return std::nullopt;
    }
    
    std::vector<User> get_all_users() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<User> result;
        result.reserve(users_.size());
        for (const auto& [id, user] : users_) {
            result.push_back(user);
        }
        return result;
    }
    
    User create_user(const User& user) {
        std::lock_guard<std::mutex> lock(mutex_);
        User new_user = user;
        new_user.id = next_id_++;
        new_user.created_at = std::chrono::system_clock::now();
        users_[new_user.id] = new_user;
        return new_user;
    }
    
    bool update_user(int id, const User& updates) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = users_.find(id);
        if (it == users_.end()) return false;
        
        User& existing = it->second;
        if (!updates.name.empty()) existing.name = updates.name;
        if (!updates.email.empty()) existing.email = updates.email;
        if (!updates.role.empty()) existing.role = updates.role;
        
        return true;
    }
    
    bool delete_user(int id) {
        std::lock_guard<std::mutex> lock(mutex_);
        return users_.erase(id) > 0;
    }
};

// JWT 认证中间件
class JWTAuthMiddleware {
private:
    std::string secret_key_;
    
public:
    explicit JWTAuthMiddleware(const std::string& secret_key)
        : secret_key_(secret_key) {}
    
    HandlerResponse operator()(const Request& req, Response& res) {
        // 检查公开路由
        if (req.path == "/api/auth/login" || req.path == "/api/health") {
            return HandlerResponse::Unhandled;
        }
        
        // 检查 Authorization 头
        auto auth_header = req.get_header_value("Authorization");
        if (auth_header.empty()) {
            res.status = StatusCode::Unauthorized_401;
            res.set_content(json{{"error", "Missing Authorization header"}}.dump(),
                           "application/json");
            return HandlerResponse::Handled;
        }
        
        // 验证 Bearer Token（简化实现）
        const std::string bearer_prefix = "Bearer ";
        if (auth_header.find(bearer_prefix) != 0) {
            res.status = StatusCode::Unauthorized_401;
            res.set_content(json{{"error", "Invalid token format"}}.dump(),
                           "application/json");
            return HandlerResponse::Handled;
        }
        
        std::string token = auth_header.substr(bearer_prefix.length());
        
        // 这里应该验证 JWT 令牌
        // 简化实现：检查令牌是否有效
        if (!is_valid_token(token)) {
            res.status = StatusCode::Unauthorized_401;
            res.set_content(json{{"error", "Invalid token"}}.dump(),
                           "application/json");
            return HandlerResponse::Handled;
        }
        
        return HandlerResponse::Unhandled;
    }
    
private:
    bool is_valid_token(const std::string& token) {
        // 实际实现应该验证 JWT 签名和过期时间
        // 这里返回 true 作为示例
        return !token.empty();
    }
};

// 流式响应示例
class StreamingService {
public:
    void register_routes(Server& svr) {
        svr.Get("/api/stream/events", [this](const Request& req, Response& res) {
            res.set_chunked_content_provider(
                "text/event-stream",
                [this](size_t offset, DataSink& sink) {
                    return generate_server_sent_events(sink);
                });
        });
        
        svr.Get("/api/stream/logs", [this](const Request& req, Response& res) {
            res.set_content_provider(
                "text/plain",
                [this](size_t offset, DataSink& sink) {
                    return stream_logs(sink);
                });
        });
    }
    
private:
    bool generate_server_sent_events(DataSink& sink) {
        for (int i = 0; i < 10; ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            json event = {
                {"id", i},
                {"event", "message"},
                {"data", {{"message", "Event " + std::to_string(i)},
                         {"timestamp", 
                          std::chrono::duration_cast<std::chrono::milliseconds>(
                              std::chrono::system_clock::now().time_since_epoch()
                          ).count()}}}
            };
            
            std::string event_str = 
                "id: " + std::to_string(i) + "\n" +
                "event: message\n" +
                "data: " + event["data"].dump() + "\n\n";
            
            if (!sink.write(event_str.data(), event_str.size())) {
                return false;
            }
            
            sink.os.flush();
        }
        
        sink.done();
        return true;
    }
    
    bool stream_logs(DataSink& sink) {
        std::vector<std::string> logs = {
            "INFO: Server starting up",
            "INFO: Database connection established",
            "WARN: High memory usage detected",
            "INFO: Request processed successfully",
            "ERROR: Failed to connect to external service"
        };
        
        for (const auto& log : logs) {
            std::string log_line = log + "\n";
            if (!sink.write(log_line.data(), log_line.size())) {
                return false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        
        sink.done();
        return true;
    }
};

// 文件上传处理器
class FileUploadHandler {
public:
    void register_routes(Server& svr) {
        svr.Post("/api/upload", [this](const Request& req, Response& res) {
            return handle_file_upload(req, res);
        });
        
        svr.Post("/api/upload/multipart", 
                [this](const Request& req, Response& res) {
            return handle_multipart_upload(req, res);
        });
    }
    
private:
    HandlerResponse handle_file_upload(const Request& req, Response& res) {
        if (!req.has_file("file")) {
            res.status = StatusCode::BadRequest_400;
            res.set_content(json{{"error", "No file uploaded"}}.dump(),
                           "application/json");
            return HandlerResponse::Handled;
        }
        
        auto file = req.get_file_value("file");
        if (file.filename.empty()) {
            res.status = StatusCode::BadRequest_400;
            res.set_content(json{{"error", "File has no name"}}.dump(),
                           "application/json");
            return HandlerResponse::Handled;
        }
        
        // 保存文件（实际应用中应该保存到文件系统）
        json response = {
            {"success", true},
            {"filename", file.filename},
            {"content_type", file.content_type},
            {"size", file.content.size()},
            {"message", "File uploaded successfully"}
        };
        
        res.set_content(response.dump(), "application/json");
        return HandlerResponse::Handled;
    }
    
    HandlerResponse handle_multipart_upload(const Request& req, Response& res) {
        json files_info = json::array();
        
        for (const auto& [field_name, file] : req.files) {
            files_info.push_back({
                {"field", field_name},
                {"filename", file.filename},
                {"content_type", file.content_type},
                {"size", file.content.size()}
            });
        }
        
        json response = {
            {"success", true},
            {"files", files_info},
            {"total_files", req.files.size()},
            {"message", "Files uploaded successfully"}
        };
        
        res.set_content(response.dump(), "application/json");
        return HandlerResponse::Handled;
    }
};

int main() {
    Server svr;
    UserService user_service;
    StreamingService streaming_service;
    FileUploadHandler upload_handler;
    
    // 配置服务器
    svr.set_tcp_nodelay(true);
    svr.set_keep_alive_max_count(100);
    svr.set_keep_alive_timeout(30);
    svr.set_read_timeout(10);
    svr.set_write_timeout(10);
    svr.set_payload_max_length(10 * 1024 * 1024); // 10MB
    
    // 设置静态文件服务
    svr.set_mount_point("/static", "./public", {
        {"Cache-Control", "public, max-age=3600"}
    });
    
    svr.set_file_extension_and_mimetype_mapping("wasm", "application/wasm");
    svr.set_file_extension_and_mimetype_mapping("woff2", "font/woff2");
    
    // 设置认证中间件
    JWTAuthMiddleware auth_middleware("your-secret-key-here");
    svr.set_pre_routing_handler(auth_middleware);
    
    // 设置日志中间件
    svr.set_logger([](const Request& req, const Response& res) {
        std::cout << "[" << httplib::to_string(res.status) << "] "
                  << req.method << " " << req.path
                  << " - " << req.remote_addr << ":" << req.remote_port
                  << std::endl;
    });
    
    // 健康检查端点
    svr.Get("/api/health", [](const Request&, Response& res) {
        json health = {
            {"status", "healthy"},
            {"timestamp", 
             std::chrono::duration_cast<std::chrono::milliseconds>(
                 std::chrono::system_clock::now().time_since_epoch()
             ).count()},
            {"version", "1.0.0"}
        };
        res.set_content(health.dump(), "application/json");
    });
    
    // 用户管理 API
    svr.Get("/api/users", [&](const Request& req, Response& res) {
        auto users = user_service.get_all_users();
        json users_json = json::array();
        for (const auto& user : users) {
            users_json.push_back(user.to_json());
        }
        res.set_content(users_json.dump(), "application/json");
    });
    
    svr.Get(R"(/api/users/(\d+))", [&](const Request& req, Response& res) {
        int user_id = std::stoi(req.matches[1]);
        auto user = user_service.get_user(user_id);
        
        if (user) {
            res.set_content(user->to_json().dump(), "application/json");
        } else {
            res.status = StatusCode::NotFound_404;
            res.set_content(json{{"error", "User not found"}}.dump(),
                           "application/json");
        }
    });
    
    svr.Post("/api/users", [&](const Request& req, Response& res) {
        try {
            auto body_json = json::parse(req.body);
            User new_user = User::from_json(body_json);
            
            if (new_user.name.empty() || new_user.email.empty()) {
                res.status = StatusCode::BadRequest_400;
                res.set_content(json{{"error", "Name and email are required"}}.dump(),
                               "application/json");
                return;
            }
            
            User created = user_service.create_user(new_user);
            res.status = StatusCode::Created_201;
            res.set_content(created.to_json().dump(), "application/json");
            
        } catch (const json::exception& e) {
            res.status = StatusCode::BadRequest_400;
            res.set_content(json{{"error", "Invalid JSON: " + std::string(e.what())}}.dump(),
                           "application/json");
        }
    });
    
    svr.Put(R"(/api/users/(\d+))", [&](const Request& req, Response& res) {
        try {
            int user_id = std::stoi(req.matches[1]);
            auto body_json = json::parse(req.body);
            User updates = User::from_json(body_json);
            
            bool success = user_service.update_user(user_id, updates);
            if (success) {
                auto updated_user = user_service.get_user(user_id);
                res.set_content(updated_user->to_json().dump(), "application/json");
            } else {
                res.status = StatusCode::NotFound_404;
                res.set_content(json{{"error", "User not found"}}.dump(),
                               "application/json");
            }
            
        } catch (const json::exception& e) {
            res.status = StatusCode::BadRequest_400;
            res.set_content(json{{"error", "Invalid JSON: " + std::string(e.what())}}.dump(),
                           "application/json");
        }
    });
    
    svr.Delete(R"(/api/users/(\d+))", [&](const Request& req, Response& res) {
        int user_id = std::stoi(req.matches[1]);
        bool success = user_service.delete_user(user_id);
        
        if (success) {
            res.set_content(json{{"success", true}, 
                                {"message", "User deleted"}}.dump(),
                           "application/json");
        } else {
            res.status = StatusCode::NotFound_404;
            res.set_content(json{{"error", "User not found"}}.dump(),
                           "application/json");
        }
    });
    
    // 注册其他服务
    streaming_service.register_routes(svr);
    upload_handler.register_routes(svr);
    
    // 错误处理器
    svr.set_error_handler([](const Request& req, Response& res) {
        json error_response = {
            {"error", httplib::status_message(res.status)},
            {"status_code", res.status},
            {"path", req.path},
            {"method", req.method}
        };
        
        res.set_content(error_response.dump(), "application/json");
        return HandlerResponse::Handled;
    });
    
    // 异常处理器
    svr.set_exception_handler([](const Request& req, Response& res,
                                 std::exception_ptr ep) {
        try {
            if (ep) std::rethrow_exception(ep);
        } catch (const std::exception& e) {
            json error_response = {
                {"error", "Internal server error"},
                {"message", e.what()},
                {"type", typeid(e).name()}
            };
            
            res.status = StatusCode::InternalServerError_500;
            res.set_content(error_response.dump(), "application/json");
        } catch (...) {
            json error_response = {
                {"error", "Unknown internal server error"}
            };
            
            res.status = StatusCode::InternalServerError_500;
            res.set_content(error_response.dump(), "application/json");
        }
    });
    
    // 启动服务器
    std::cout << "Starting server on http://localhost:8080" << std::endl;
    std::cout << "API endpoints:" << std::endl;
    std::cout << "  GET  /api/health" << std::endl;
    std::cout << "  GET  /api/users" << std::endl;
    std::cout << "  GET  /api/users/{id}" << std::endl;
    std::cout << "  POST /api/users" << std::endl;
    std::cout << "  PUT  /api/users/{id}" << std::endl;
    std::cout << "  DEL  /api/users/{id}" << std::endl;
    std::cout << "  GET  /api/stream/events" << std::endl;
    std::cout << "  GET  /api/stream/logs" << std::endl;
    std::cout << "  POST /api/upload" << std::endl;
    std::cout << "  POST /api/upload/multipart" << std::endl;
    
    if (!svr.listen("0.0.0.0", 8080)) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }
    
    return 0;
}
```

### 10.2 高级 HTTP 客户端示例

```cpp
#include "httplib.h"
#include <iostream>
#include <memory>
#include <chrono>
#include <fstream>
#include <nlohmann/json.hpp>
#include <concurrentqueue.h> // 线程安全队列

using json = nlohmann::json;
using namespace httplib;

// 异步 HTTP 客户端封装
class AsyncHttpClient {
private:
    Client client_;
    moodycamel::ConcurrentQueue<std::function<void()>> task_queue_;
    std::atomic<bool> running_{true};
    std::thread worker_thread_;
    
public:
    AsyncHttpClient(const std::string& host, int port)
        : client_(host, port) {
        
        // 配置客户端
        client_.set_connection_timeout(30);
        client_.set_read_timeout(30);
        client_.set_write_timeout(30);
        client_.set_keep_alive(true);
        client_.set_compress(true);
        client_.set_decompress(true);
        client_.set_follow_location(true);
        
        // 启动工作线程
        worker_thread_ = std::thread([this]() { worker_loop(); });
    }
    
    ~AsyncHttpClient() {
        running_ = false;
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
    }
    
    // 异步 GET 请求
    template<typename Callback>
    void get_async(const std::string& path, 
                   const Headers& headers,
                   Callback&& callback) {
        
        enqueue_task([this, path, headers, callback = std::forward<Callback>(callback)]() {
            auto result = client_.Get(path, headers);
            callback(std::move(result));
        });
    }
    
    // 异步 POST 请求
    template<typename Callback>
    void post_async(const std::string& path,
                    const std::string& body,
                    const std::string& content_type,
                    const Headers& headers,
                    Callback&& callback) {
        
        enqueue_task([this, path, body, content_type, headers, 
                     callback = std::forward<Callback>(callback)]() {
            auto result = client_.Post(path, headers, body, content_type);
            callback(std::move(result));
        });
    }
    
    // 流式上传
    void upload_file_async(const std::string& path,
                           const std::string& file_path,
                           const std::string& field_name,
                           std::function<void(Result)> callback) {
        
        enqueue_task([this, path, file_path, field_name, callback]() {
            std::ifstream file(file_path, std::ios::binary | std::ios::ate);
            if (!file) {
                Result result(nullptr, Error::Unknown);
                callback(std::move(result));
                return;
            }
            
            auto size = file.tellg();
            file.seekg(0, std::ios::beg);
            
            // 创建 multipart 表单数据
            MultipartFormDataItems items = {
                {field_name, "", file_path, "application/octet-stream"}
            };
            
            // 使用内容提供器流式上传
            auto result = client_.Post(path, Headers{}, items);
            callback(std::move(result));
        });
    }
    
    // 批量请求
    template<typename Callback>
    void batch_get_async(const std::vector<std::string>& urls,
                         size_t concurrency,
                         Callback&& callback) {
        
        enqueue_task([this, urls, concurrency, 
                     callback = std::forward<Callback>(callback)]() {
            std::vector<Result> results;
            results.reserve(urls.size());
            
            // 使用并行处理（简化实现）
            for (const auto& url : urls) {
                auto result = client_.Get(url);
                results.push_back(std::move(result));
            }
            
            callback(std::move(results));
        });
    }
    
private:
    void worker_loop() {
        while (running_) {
            std::function<void()> task;
            if (task_queue_.try_dequeue(task)) {
                try {
                    task();
                } catch (const std::exception& e) {
                    std::cerr << "Task execution failed: " << e.what() << std::endl;
                }
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    }
    
    template<typename Task>
    void enqueue_task(Task&& task) {
        task_queue_.enqueue(std::forward<Task>(task));
    }
};

// API 客户端封装
class UserAPIClient {
private:
    Client client_;
    std::string base_url_;
    std::string auth_token_;
    
public:
    UserAPIClient(const std::string& host, int port)
        : client_(host, port), base_url_("http://" + host + ":" + std::to_string(port)) {
        
        // 配置客户端
        client_.set_default_headers({
            {"User-Agent", "UserAPIClient/1.0"},
            {"Accept", "application/json"},
            {"Accept-Encoding", "gzip, deflate, br"}
        });
        
        client_.set_connection_timeout(10);
        client_.set_read_timeout(30);
        client_.set_compress(true);
    }
    
    // 设置认证令牌
    void set_auth_token(const std::string& token) {
        auth_token_ = token;
        client_.set_bearer_token_auth(token);
    }
    
    // 登录获取令牌
    bool login(const std::string& username, const std::string& password) {
        json credentials = {
            {"username", username},
            {"password", password}
        };
        
        auto res = client_.Post("/api/auth/login", 
                               credentials.dump(), 
                               "application/json");
        
        if (res && res->status == StatusCode::OK_200) {
            try {
                auto json_res = json::parse(res->body);
                auth_token_ = json_res["token"];
                client_.set_bearer_token_auth(auth_token_);
                return true;
            } catch (...) {
                return false;
            }
        }
        
        return false;
    }
    
    // 获取所有用户（带分页）
    std::optional<json> get_users(int page = 1, int per_page = 20) {
        std::string path = "/api/users?page=" + std::to_string(page) + 
                          "&per_page=" + std::to_string(per_page);
        
        auto res = client_.Get(path);
        if (res && res->status == StatusCode::OK_200) {
            try {
                return json::parse(res->body);
            } catch (...) {
                return std::nullopt;
            }
        }
        
        return std::nullopt;
    }
    
    // 创建用户
    std::optional<json> create_user(const std::string& name,
                                    const std::string& email,
                                    const std::string& role = "user") {
        json user_data = {
            {"name", name},
            {"email", email},
            {"role", role}
        };
        
        auto res = client_.Post("/api/users", 
                               user_data.dump(), 
                               "application/json");
        
        if (res && res->status == StatusCode::Created_201) {
            try {
                return json::parse(res->body);
            } catch (...) {
                return std::nullopt;
            }
        }
        
        return std::nullopt;
    }
    
    // 流式下载
    bool download_file(const std::string& url,
                      const std::string& save_path,
                      std::function<void(uint64_t, uint64_t)> progress_callback = nullptr) {
        
        std::ofstream file(save_path, std::ios::binary);
        if (!file) return false;
        
        // 创建新客户端实例避免干扰主客户端状态
        Client download_client(client_.host(), client_.port());
        download_client.set_follow_location(true);
        download_client.set_compress(true);
        
        auto res = download_client.Get(url, 
            [&](const char* data, size_t data_length, 
                uint64_t offset, uint64_t total_length) {
                file.write(data, data_length);
                
                if (progress_callback && total_length > 0) {
                    progress_callback(offset + data_length, total_length);
                }
                
                return true;
            },
            [&](uint64_t current, uint64_t total) {
                if (progress_callback && total > 0) {
                    progress_callback(current, total);
                }
                return true;
            });
        
        file.close();
        return res && res->status == StatusCode::OK_200;
    }
    
    // 监控服务器事件（Server-Sent Events）
    void monitor_events(std::function<void(const json&)> event_handler,
                       std::function<void()> on_complete = nullptr) {
        
        std::thread([this, event_handler, on_complete]() {
            Client event_client(client_.host(), client_.port());
            event_client.set_read_timeout(0); // 无超时
            
            auto res = event_client.Get("/api/stream/events",
                [event_handler](const char* data, size_t data_length, 
                               uint64_t /*offset*/, uint64_t /*total*/) {
                    // 解析 Server-Sent Events
                    std::string chunk(data, data_length);
                    parse_sse_chunk(chunk, event_handler);
                    return true;
                });
            
            if (on_complete) on_complete();
        }).detach();
    }
    
private:
    static void parse_sse_chunk(const std::string& chunk,
                               std::function<void(const json&)> event_handler) {
        std::istringstream stream(chunk);
        std::string line;
        std::string event_data;
        
        while (std::getline(stream, line)) {
            if (line.empty() || line == "\r") {
                // 空行表示事件结束
                if (!event_data.empty()) {
                    try {
                        auto json_data = json::parse(event_data);
                        event_handler(json_data);
                    } catch (...) {
                        // 忽略解析错误
                    }
                    event_data.clear();
                }
            } else if (line.compare(0, 6, "data: ") == 0) {
                event_data = line.substr(6);
            }
        }
    }
};

// 使用示例
int main() {
    std::cout << "=== HTTP Client Examples ===" << std::endl;
    
    // 1. 基本客户端使用
    {
        std::cout << "\n1. Basic Client Usage:" << std::endl;
        
        Client client("httpbin.org", 80);
        
        // GET 请求
        auto res = client.Get("/get");
        if (res) {
            std::cout << "GET Response: " << res->status << std::endl;
            std::cout << "Body size: " << res->body.size() << " bytes" << std::endl;
        }
        
        // POST 请求
        json post_data = {{"name", "John"}, {"age", 30}};
        res = client.Post("/post", 
                         post_data.dump(), 
                         "application/json");
        
        if (res) {
            std::cout << "POST Response: " << res->status << std::endl;
        }
    }
    
    // 2. 用户 API 客户端
    {
        std::cout << "\n2. User API Client:" << std::endl;
        
        UserAPIClient api_client("localhost", 8080);
        
        // 登录
        if (api_client.login("admin", "password")) {
            std::cout << "Login successful" << std::endl;
            
            // 获取用户列表
            auto users = api_client.get_users(1, 10);
            if (users) {
                std::cout << "Total users: " << users->size() << std::endl;
            }
            
            // 创建新用户
            auto new_user = api_client.create_user("Alice", "alice@example.com");
            if (new_user) {
                std::cout << "Created user ID: " << (*new_user)["id"] << std::endl;
            }
            
            // 下载文件
            api_client.download_file(
                "http://localhost:8080/static/sample.pdf",
                "downloaded.pdf",
                [](uint64_t current, uint64_t total) {
                    double percent = (total > 0) ? 
                        (static_cast<double>(current) / total * 100.0) : 0.0;
                    std::cout << "\rDownloading: " << std::fixed << std::setprecision(1) 
                              << percent << "%" << std::flush;
                });
            std::cout << std::endl;
        }
    }
    
    // 3. 异步客户端
    {
        std::cout << "\n3. Async Client:" << std::endl;
        
        AsyncHttpClient async_client("localhost", 8080);
        
        // 发起多个异步请求
        std::atomic<int> completed_requests{0};
        int total_requests = 5;
        
        for (int i = 0; i < total_requests; ++i) {
            async_client.get_async("/api/health", {}, 
                [i, &completed_requests, total_requests](Result result) {
                    if (result) {
                        std::cout << "Request " << i << " completed: " 
                                  << result->status << std::endl;
                    } else {
                        std::cout << "Request " << i << " failed" << std::endl;
                    }
                    
                    completed_requests++;
                    if (completed_requests == total_requests) {
                        std::cout << "All requests completed" << std::endl;
                    }
                });
        }
        
        // 等待请求完成
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    
    // 4. 监控服务器事件
    {
        std::cout << "\n4. Server Event Monitoring:" << std::endl;
        
        UserAPIClient api_client("localhost", 8080);
        
        api_client.monitor_events(
            [](const json& event) {
                std::cout << "Received event: " << event.dump() << std::endl;
            },
            []() {
                std::cout << "Event stream closed" << std::endl;
            });
        
        // 监控一段时间
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
    
    return 0;
}
```

### 10.3 性能监控中间件示例

```cpp
// 性能监控中间件
class PerformanceMonitor {
private:
    struct RequestStats {
        std::string method;
        std::string path;
        int status_code;
        std::chrono::microseconds duration;
        std::chrono::system_clock::time_point timestamp;
        size_t request_size;
        size_t response_size;
    };
    
    std::vector<RequestStats> recent_requests_;
    std::mutex stats_mutex_;
    size_t max_records_ = 1000;
    
public:
    HandlerResponse operator()(const Request& req, Response& res) {
        auto start_time = std::chrono::steady_clock::now();
        auto start_size = req.body.size();
        
        // 继续处理
        return HandlerResponse::Unhandled;
    }
    
    void record_response(const Request& req, const Response& res,
                        std::chrono::steady_clock::time_point start_time) {
        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end_time - start_time);
        
        RequestStats stats{
            .method = req.method,
            .path = req.path,
            .status_code = res.status,
            .duration = duration,
            .timestamp = std::chrono::system_clock::now(),
            .request_size = req.body.size(),
            .response_size = res.body.size()
        };
        
        {
            std::lock_guard<std::mutex> lock(stats_mutex_);
            recent_requests_.push_back(stats);
            if (recent_requests_.size() > max_records_) {
                recent_requests_.erase(recent_requests_.begin());
            }
        }
    }
    
    json get_performance_report() const {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        
        if (recent_requests_.empty()) {
            return {{"error", "No requests recorded"}};
        }
        
        // 计算统计数据
        std::chrono::microseconds total_duration{0};
        size_t total_requests = recent_requests_.size();
        std::map<int, size_t> status_counts;
        std::map<std::string, size_t> endpoint_counts;
        
        for (const auto& stats : recent_requests_) {
            total_duration += stats.duration;
            status_counts[stats.status_code]++;
            endpoint_counts[stats.method + " " + stats.path]++;
        }
        
        auto avg_duration = total_duration / total_requests;
        
        // 构建报告
        json report = {
            {"total_requests", total_requests},
            {"avg_response_time_ms", 
             std::chrono::duration_cast<std::chrono::milliseconds>(
                 avg_duration).count()},
            {"status_distribution", json::object()},
            {"endpoint_distribution", json::object()},
            {"recent_requests", json::array()}
        };
        
        for (const auto& [status, count] : status_counts) {
            report["status_distribution"][std::to_string(status)] = count;
        }
        
        for (const auto& [endpoint, count] : endpoint_counts) {
            report["endpoint_distribution"][endpoint] = count;
        }
        
        // 添加最近请求详情（限制数量）
        size_t limit = std::min(size_t(10), recent_requests_.size());
        for (size_t i = 0; i < limit; ++i) {
            const auto& stats = recent_requests_[recent_requests_.size() - 1 - i];
            report["recent_requests"].push_back({
                {"method", stats.method},
                {"path", stats.path},
                {"status", stats.status_code},
                {"duration_ms", 
                 std::chrono::duration_cast<std::chrono::milliseconds>(
                     stats.duration).count()},
                {"timestamp", 
                 std::chrono::duration_cast<std::chrono::seconds>(
                     stats.timestamp.time_since_epoch()).count()}
            });
        }
        
        return report;
    }
};

// 在服务器中使用
int main() {
    Server svr;
    PerformanceMonitor perf_monitor;
    
    // 设置前置处理器记录开始时间
    svr.set_pre_request_handler([&](const Request& req, Response& res) {
        req.start_time_ = std::chrono::steady_clock::now();
        return HandlerResponse::Unhandled;
    });
    
    // 设置后置处理器记录性能数据
    svr.set_post_routing_handler([&](const Request& req, Response& res) {
        perf_monitor.record_response(req, res, req.start_time_);
    });
    
    // 添加性能报告端点
    svr.Get("/api/performance", [&](const Request&, Response& res) {
        auto report = perf_monitor.get_performance_report();
        res.set_content(report.dump(), "application/json");
    });
    
    // ... 其他路由
    
    svr.listen("localhost", 8080);
    return 0;
}
```

## 11. 总结

### 11.1 设计亮点总结

1. **极简的集成方式**：单头文件设计，零依赖（可选）
2. **现代 C++ 特性充分利用**：RAII、lambda、智能指针、类型安全枚举
3. **性能优化**：零拷贝文件发送、连接复用、流式处理
4. **完整的 HTTP/1.1 支持**：分块传输、范围请求、多部分表单等
5. **灵活的中间件系统**：支持前置/后置处理器、错误处理链
6. **线程安全**：客户端支持多线程并发使用
7. **丰富的功能**：SSL/TLS、压缩、代理、认证等

### 11.2 适用场景

1. **微服务**：轻量级的 HTTP 通信
2. **嵌入式系统**：资源受限环境下的 Web 服务
3. **快速原型开发**：简单的 API 服务器
4. **内部工具**：管理界面、监控面板
5. **网关/代理**：简单的请求转发和处理

### 11.3 性能建议

1. **连接池**：客户端启用 keep-alive
2. **压缩**：启用 gzip/brotli 压缩减少带宽
3. **文件服务**：使用 `set_file_content` 进行零拷贝发送
4. **异步处理**：耗时操作使用异步处理器
5. **监控**：添加性能监控中间件

### 11.4 扩展建议

1. **添加 WebSocket 支持**
2. **HTTP/2 支持**
3. **更完善的连接池管理**
4. **请求/响应拦截器链**
5. **更细粒度的超时控制**

这个完整的解析展示了 cpp-httplib 作为现代 C++ HTTP 库的强大功能和优雅设计。它平衡了易用性、性能和功能完整性，是许多 C++ 项目的理想选择。