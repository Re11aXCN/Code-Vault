#include "template_blob.hpp"
#include "template_autodump.hpp"
#include "template_type_traits.h"

#include "ringbuffer.hpp"

#include <iostream>
#include <string>
#include <thread>
#include <chrono>

void test_blob() {
    // 创建一个字符串Blob对象
    Blob<std::string> b1{ "Hello", "World", "C++", "Templates" };

    // 创建共享同一数据的Blob对象
    Blob<std::string> b2 = b1;  // b1和b2共享数据

    // 修改b2，同时会影响b1（因为共享数据）
    b2.push_back("Shared");
    std::cout << "b1的最后一个元素: " << b1.back() << std::endl;  // 输出: Shared

    // 使用BlobPtr遍历Blob对象
    BlobPtr<std::string> ptr1(b1);
    std::cout << "Blob内容: ";
    for (size_t i = 0; i < b1.size(); ++i) {
        std::cout << *ptr1 << " ";
        ++ptr1;
    }
    std::cout << std::endl;

    // 演示weak_ptr的作用
    {
        Blob<int> tempBlob{ 1, 2, 3 };
        BlobPtr<int> tempPtr(tempBlob);

        // 在这个作用域内可以使用tempPtr
        std::cout << "临时Blob的第一个元素: " << *tempPtr << std::endl;
    } // tempBlob在这里被销毁

    // 如果尝试使用指向已销毁Blob的BlobPtr，会抛出异常
    try {
        Blob<double> tempBlob{ 1.1, 2.2, 3.3 };
        BlobPtr<double> dangling(tempBlob);
        tempBlob = Blob<double>(); // 重新赋值，原始数据可能被释放

        // 尝试访问可能已释放的数据
        std::cout << *dangling << std::endl;
    }
    catch (const std::exception& e) {
        std::cout << "捕获异常: " << e.what() << std::endl;
    }
}

// ========================= 示例使用 =========================
struct Point {
    int x, y;
    friend std::ostream& operator<<(std::ostream& os, const Point& p) {
        return os << "Point(" << p.x << "," << p.y << ")";
    }
};
void test_autodump() {
    // 基本类型
    AUTO_DUMP_COUT(42);
    AUTO_DUMP_FORMAT(3.14, "PI = {}");

    /*
    // 时间戳输出
    AUTO_DUMP_TIME("程序启动", "{}");
    // 调试信息
    AUTO_DUMP_DEBUG("这是一个调试消息");
    */
    // 容器输出
    std::vector<int> vec{ 1, 2, 3, 4, 5 };
    AUTO_DUMP_CONTAINER(vec);

    std::map<std::string, int> scores{ {"Alice", 95}, {"Bob", 87} };
    AUTO_DUMP_FORMAT(scores.size(), "学生数量: {}");
}

void test_spsc()
{
    RingBuffer<int> rb(1024); // 创建容量为1024的环形缓冲区
    using namespace std::chrono;
    steady_clock::time_point start = steady_clock::now(); // 记录开始时间
    // 生产者线程
    std::thread producer([&] {
        for (int i = 0; i < 1000; ++i) {
            while (!rb.push(i)) {
                std::this_thread::yield(); // 缓冲区满时让步
            }
        }
        });

    // 消费者线程
    std::thread consumer([&] {
        int value;
        for (int i = 0; i < 1000; ++i) {
            while (!rb.pop(value)) {
                std::this_thread::yield(); // 缓冲区空时让步
            }
            std::cout << value << " ";
        }
        });

    producer.join();
    consumer.join();
    steady_clock::time_point end = steady_clock::now(); // 记录结束时间
    std::cout << "总耗时: " << duration_cast<milliseconds>(end - start).count() << "ms" << std::endl; // 输出总耗时
}
int main() {
    ///< test_blob
    test_blob();

    ///< test_autodump
    test_autodump();

    ///< test template_type_traits
    determine_type();
    print_type_info();

    ///< test_spsc
    test_spsc();
    return 0;
}