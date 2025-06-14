#ifndef _RingBuffer_HPP_
#define _RingBuffer_HPP_
#include <atomic>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <bit> 
//SPSC 单生产者单消费者模式，无锁队列
template <typename T>
class RingBuffer {
public:
static_assert(std::is_nothrow_move_assignable_v<T>,
    "T must be nothrow move assignable");
static_assert(std::is_nothrow_move_constructible_v<T>,
    "T must be nothrow move constructible");

    explicit RingBuffer(size_t capacity)
        : capacity_(calculateCapacity(capacity)),
        mask_(capacity_ - 1),
        buffer_(std::allocator<Slot>().allocate(capacity_))
    {
    }

    ~RingBuffer() {
        // 析构所有有效对象
        size_t head = head_.load(std::memory_order_relaxed);
        size_t tail = tail_.load(std::memory_order_relaxed);

        while (head != tail) {
            buffer_[head & mask_].destroy();
            head = (head + 1) & (2 * capacity_ - 1);
        }
        std::allocator<Slot>().deallocate(buffer_, capacity_);
    }

    // 禁止拷贝和移动
    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;

    // 推入元素（完美转发）
    template <typename U>
    bool push(U&& item) {
        return emplace(std::forward<U>(item));
    }

    // 弹出元素
    bool pop(T& item) {
        const size_t head = head_.load(std::memory_order_relaxed);

        // 检查缓冲区是否为空
        if (head == tail_cache_) {
            // 刷新消费者缓存的tail值
            tail_cache_ = tail_.load(std::memory_order_acquire);
            if (head == tail_cache_) {
                return false;
            }
        }

        Slot& slot = buffer_[head & mask_];

        // 直接移动对象，避免额外构造
        item = std::move(slot.value());

        // 标记槽位为空，延迟析构
        slot.destroy();

        // 更新head索引
        head_.store((head + 1) & (2 * capacity_ - 1), std::memory_order_release);
        return true;
    }

    // 检查缓冲区是否为空
    bool empty() const {
        const size_t head = head_.load(std::memory_order_acquire);
        const size_t tail = tail_.load(std::memory_order_acquire);
        return head == tail;
    }

    // 获取缓冲区容量
    size_t capacity() const {
        return capacity_ - 1;  // 实际可用容量
    }

private:
    // 内部存储槽位（缓存行对齐）
    struct alignas(64) Slot {
        Slot() noexcept = default;

        // 移除析构函数
        ~Slot() = delete;

        template <typename... Args>
        void construct(Args&&... args) {
            new (&storage) T(std::forward<Args>(args)...);
        }

        void destroy() noexcept {
            if (auto ptr = std::launder(reinterpret_cast<T*>(&storage))) {
                ptr->~T();
            }
        }

        T& value() noexcept {
            return *std::launder(reinterpret_cast<T*>(&storage));
        }

    private:
        std::aligned_storage_t<sizeof(T), alignof(T)> storage;
    };

    // 计算大于等于请求容量的最小2的幂
    static size_t calculateCapacity(size_t requested) noexcept {
        if (requested < 1) requested = 1;
        return std::bit_ceil(requested + 1);
    }

    template <typename... Args>
    bool emplace(Args&&... args) {
        // 1. 获取当前tail（宽松加载：只需原子性，无需同步）
        size_t tail = tail_.load(std::memory_order_relaxed);

        // 2. 使用缓存的消费者索引（上次获取的值）
        size_t actual_head = head_cache_;

        // 第一次检查：快速路径（90%情况命中）
        if ((tail - actual_head) >= capacity_) {
            // 3. 可能满，需获取最新head（获取语义：看到消费者所有修改）
            actual_head = head_.load(std::memory_order_acquire);
            head_cache_ = actual_head; // 更新缓存

            // 第二次检查：最终确认
            if ((tail - actual_head) >= capacity_) {
                return false; // 确认为满
            }
        }

        // 4. 写入数据...
        Slot& slot = buffer_[tail & mask_];
        slot.construct(std::forward<Args>(args)...);

        // 5. 更新tail（释放语义：确保写入对消费者可见）
        tail_.store((tail + 1) & (2 * capacity_ - 1), std::memory_order_release); 
        return true;
    }

private:
    const size_t capacity_;     // 实际分配的容量（2的幂）
    const size_t mask_;         // 索引掩码（capacity_-1）
    Slot* buffer_;              // 存储槽位数组

    // 生产者相关数据（缓存行对齐）
    alignas(64) std::atomic<size_t> tail_{ 0 };      // 生产者索引
    size_t head_cache_ = 0;                        // 生产者缓存的消费者索引

    // 消费者相关数据（缓存行对齐）
    alignas(64) std::atomic<size_t> head_{ 0 };      // 消费者索引
    size_t tail_cache_ = 0;                        // 消费者缓存的生产者索引
};
#endif // !_RingBuffer_HPP_

/*
Use Example
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
*/