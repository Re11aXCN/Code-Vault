#ifndef _CONCURRENT_QUEUE_HPP_
#define _CONCURRENT_QUEUE_HPP_

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <thread>
#include <type_traits>
#include <utility>
#include <bit>
#include <concepts>  // C++20
#include <span>      // C++20

/**
 * @brief 并发队列的并发策略枚举
 * 
 * 定义了四种并发策略：
 * - SPSC_LockFree: 单生产者单消费者无锁实现
 * - SPSC_Locked: 单生产者单消费者有锁实现
 * - MPMC_LockFree: 多生产者多消费者无锁实现
 * - MPMC_Locked: 多生产者多消费者有锁实现
 */
enum class ConcurrencyType {
    SPSC_LockFree,  // 单生产者单消费者无锁
    SPSC_Locked,   // 单生产者单消费者有锁
    MPMC_LockFree, // 多生产者多消费者无锁
    MPMC_Locked    // 多生产者多消费者有锁
};

/**
 * @brief 并发队列的特性标签结构体
 * 
 * 用于在编译时选择不同的并发策略实现
 */
template <ConcurrencyType Type>
struct ConcurrencyTrait {};

// 特化：单生产者单消费者无锁实现
template <>
struct ConcurrencyTrait<ConcurrencyType::SPSC_LockFree> {
    static constexpr bool is_lock_free = true;
    static constexpr bool is_multi_producer = false;
    static constexpr bool is_multi_consumer = false;
    static constexpr const char* name = "SPSC Lock-Free";
};

// 特化：单生产者单消费者有锁实现
template <>
struct ConcurrencyTrait<ConcurrencyType::SPSC_Locked> {
    static constexpr bool is_lock_free = false;
    static constexpr bool is_multi_producer = false;
    static constexpr bool is_multi_consumer = false;
    static constexpr const char* name = "SPSC Locked";
};

// 特化：多生产者多消费者无锁实现
template <>
struct ConcurrencyTrait<ConcurrencyType::MPMC_LockFree> {
    static constexpr bool is_lock_free = true;
    static constexpr bool is_multi_producer = true;
    static constexpr bool is_multi_consumer = true;
    static constexpr const char* name = "MPMC Lock-Free";
};

// 特化：多生产者多消费者有锁实现
template <>
struct ConcurrencyTrait<ConcurrencyType::MPMC_Locked> {
    static constexpr bool is_lock_free = false;
    static constexpr bool is_multi_producer = true;
    static constexpr bool is_multi_consumer = true;
    static constexpr const char* name = "MPMC Locked";
};

/**
 * @brief 检查类型是否满足队列元素要求的概念
 * 
 * 对于无锁实现，要求类型必须是无抛出移动构造和移动赋值的
 */
template <typename T, ConcurrencyType Type>
concept QueueElement = requires {
    requires std::is_nothrow_destructible_v<T>;
    requires !ConcurrencyTrait<Type>::is_lock_free || 
             (std::is_nothrow_move_constructible_v<T> && 
              std::is_nothrow_move_assignable_v<T>);
};

/**
 * @brief 并发队列的基类，提供公共接口和基本功能
 * 
 * @tparam T 队列元素类型
 * @tparam Type 并发策略类型
 * @tparam Traits 并发特性类型
 */
template <typename T, ConcurrencyType Type, typename Traits = ConcurrencyTrait<Type>>
class ConcurrentQueueBase {
 public:
    // 类型定义
    using value_type = T;
    using size_type = std::size_t;
    using reference = T&;
    using const_reference = const T&;
    using traits_type = Traits;

    // 禁止拷贝和移动
    ConcurrentQueueBase(const ConcurrentQueueBase&) = delete;
    ConcurrentQueueBase& operator=(const ConcurrentQueueBase&) = delete;
    ConcurrentQueueBase(ConcurrentQueueBase&&) = delete;
    ConcurrentQueueBase& operator=(ConcurrentQueueBase&&) = delete;

    /**
     * @brief 获取队列容量
     * @return 队列的容量（元素数量）
     */
    size_type capacity() const noexcept {
        return capacity_ - 1; // 实际可用容量
    }

    /**
     * @brief 获取队列名称
     * @return 队列的名称（基于并发策略）
     */
    static constexpr const char* name() noexcept {
        return Traits::name;
    }

    /**
     * @brief 检查队列是否为无锁实现
     * @return 如果是无锁实现则返回true
     */
    static constexpr bool is_lock_free() noexcept {
        return Traits::is_lock_free;
    }

    /**
     * @brief 检查队列是否支持多生产者
     * @return 如果支持多生产者则返回true
     */
    static constexpr bool is_multi_producer() noexcept {
        return Traits::is_multi_producer;
    }

    /**
     * @brief 检查队列是否支持多消费者
     * @return 如果支持多消费者则返回true
     */
    static constexpr bool is_multi_consumer() noexcept {
        return Traits::is_multi_consumer;
    }

protected:
    /**
     * @brief 构造函数
     * @param capacity 队列容量
     */
    explicit ConcurrentQueueBase(size_type capacity) noexcept
        : capacity_(calculateCapacity(capacity)) {}

    /**
     * @brief 析构函数
     */
    virtual ~ConcurrentQueueBase() = default;

    /**
     * @brief 计算大于等于请求容量的最小2的幂
     * @param requested 请求的容量
     * @return 调整后的容量（2的幂）
     */
    static size_type calculateCapacity(size_type requested) noexcept {
        if (requested < 1) requested = 1;
        return std::bit_ceil(requested + 1); // C++20 的 std::bit_ceil
    }

    // 队列容量（2的幂）
    const size_type capacity_;
};

/**
 * @brief 单生产者单消费者无锁队列实现
 * 
 * 这是一个高性能的无锁环形缓冲区实现，专为SPSC场景优化
 * 
 * @tparam T 队列元素类型
 */
template <typename T>
requires QueueElement<T, ConcurrencyType::SPSC_LockFree>
class ConcurrentQueue<T, ConcurrencyType::SPSC_LockFree> 
    : public ConcurrentQueueBase<T, ConcurrencyType::SPSC_LockFree> {
private:
    using Base = ConcurrentQueueBase<T, ConcurrencyType::SPSC_LockFree>;
    using size_type = typename Base::size_type;

    // 内部存储槽位（缓存行对齐）
    struct alignas(64) Slot {
        Slot() noexcept = default;

        // 禁用析构函数，由队列管理生命周期
        ~Slot() = delete;

        /**
         * @brief 在槽位构造对象
         * @tparam Args 构造参数类型
         * @param args 构造参数
         */
        template <typename... Args>
        void construct(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
            new (&storage) T(std::forward<Args>(args)...);
        }

        /**
         * @brief 析构槽位中的对象
         */
        void destroy() noexcept {
            if (auto ptr = std::launder(reinterpret_cast<T*>(&storage))) {
                ptr->~T();
            }
        }

        /**
         * @brief 获取槽位中的对象引用
         * @return 对象引用
         */
        T& value() noexcept {
            return *std::launder(reinterpret_cast<T*>(&storage));
        }

    private:
        // 使用aligned_storage存储对象，确保正确的内存对齐
        std::aligned_storage_t<sizeof(T), alignof(T)> storage;
    };

public:
    /**
     * @brief 构造函数
     * @param capacity 队列容量
     */
    explicit ConcurrentQueue(size_type capacity)
        : Base(capacity),
          mask_(this->capacity_ - 1),
          buffer_(std::allocator<Slot>().allocate(this->capacity_)) {}

    /**
     * @brief 析构函数
     * 
     * 析构所有有效对象并释放内存
     */
    ~ConcurrentQueue() override {
        // 析构所有有效对象
        size_type head = head_.load(std::memory_order_relaxed);
        size_type tail = tail_.load(std::memory_order_relaxed);

        while (head != tail) {
            buffer_[head & mask_].destroy();
            head = (head + 1) & (2 * this->capacity_ - 1);
        }

        // 释放内存
        std::allocator<Slot>().deallocate(buffer_, this->capacity_);
    }

    /**
     * @brief 推入元素（完美转发）
     * @tparam U 元素类型
     * @param item 要推入的元素
     * @return 如果成功推入则返回true，队列满则返回false
     */
    template <typename U>
    bool push(U&& item) noexcept(std::is_nothrow_constructible_v<T, U&&>) {
        return emplace(std::forward<U>(item));
    }

    /**
     * @brief 原地构造并推入元素
     * @tparam Args 构造参数类型
     * @param args 构造参数
     * @return 如果成功推入则返回true，队列满则返回false
     */
    template <typename... Args>
    bool emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
        // 1. 获取当前tail（宽松加载：只需原子性，无需同步）
        size_type tail = tail_.load(std::memory_order_relaxed);

        // 2. 使用缓存的消费者索引（上次获取的值）
        size_type actual_head = head_cache_;

        // 第一次检查：快速路径（90%情况命中）
        if ((tail - actual_head) >= this->capacity_) {
            // 3. 可能满，需获取最新head（获取语义：看到消费者所有修改）
            actual_head = head_.load(std::memory_order_acquire);
            head_cache_ = actual_head; // 更新缓存

            // 第二次检查：最终确认
            if ((tail - actual_head) >= this->capacity_) {
                return false; // 确认为满
            }
        }

        // 4. 写入数据
        Slot& slot = buffer_[tail & mask_];
        slot.construct(std::forward<Args>(args)...);

        // 5. 更新tail（释放语义：确保写入对消费者可见）
        tail_.store((tail + 1) & (2 * this->capacity_ - 1), std::memory_order_release); 
        return true;
    }

    /**
     * @brief 弹出元素
     * @param item 存储弹出元素的引用
     * @return 如果成功弹出则返回true，队列空则返回false
     */
    bool pop(T& item) noexcept(std::is_nothrow_move_assignable_v<T>) {
        const size_type head = head_.load(std::memory_order_relaxed);

        // 检查缓冲区是否为空
        if (head == tail_cache_) {
            // 刷新消费者缓存的tail值
            tail_cache_ = tail_.load(std::memory_order_acquire);
            if (head == tail_cache_) {
                return false; // 队列为空
            }
        }

        // 获取并移动数据
        Slot& slot = buffer_[head & mask_];
        item = std::move(slot.value());
        slot.destroy();

        // 更新head索引（释放语义：确保对生产者可见）
        head_.store((head + 1) & (2 * this->capacity_ - 1), std::memory_order_release);
        return true;
    }

    /**
     * @brief 尝试弹出元素（C++17 std::optional版本）
     * @return 如果成功则返回包含元素的optional，否则返回空optional
     */
    std::optional<T> try_pop() noexcept(std::is_nothrow_move_constructible_v<T>) {
        T item;
        if (pop(item)) {
            return std::optional<T>(std::move(item));
        }
        return std::nullopt;
    }

    /**
     * @brief 检查队列是否为空
     * @return 如果队列为空则返回true
     */
    bool empty() const noexcept {
        const size_type head = head_.load(std::memory_order_acquire);
        const size_type tail = tail_.load(std::memory_order_acquire);
        return head == tail;
    }

    /**
     * @brief 获取队列中的元素数量（近似值）
     * @return 队列中的元素数量
     * 
     * 注意：在并发环境中，此值可能在返回后立即过时
     */
    size_type size() const noexcept {
        const size_type head = head_.load(std::memory_order_acquire);
        const size_type tail = tail_.load(std::memory_order_acquire);
        return (tail - head) & (2 * this->capacity_ - 1);
    }

private:
    const size_type mask_;           // 索引掩码（capacity_-1）
    Slot* buffer_;                   // 存储槽位数组

    // 生产者相关数据（缓存行对齐）
    alignas(64) std::atomic<size_type> tail_{ 0 };  // 生产者索引
    size_type head_cache_ = 0;                      // 生产者缓存的消费者索引

    // 消费者相关数据（缓存行对齐）
    alignas(64) std::atomic<size_type> head_{ 0 };  // 消费者索引
    size_type tail_cache_ = 0;                      // 消费者缓存的生产者索引
};

/**
 * @brief 单生产者单消费者有锁队列实现
 * 
 * 使用互斥锁和条件变量实现的SPSC队列
 * 
 * @tparam T 队列元素类型
 */
template <typename T>
requires QueueElement<T, ConcurrencyType::SPSC_Locked>
class ConcurrentQueue<T, ConcurrencyType::SPSC_Locked>
    : public ConcurrentQueueBase<T, ConcurrencyType::SPSC_Locked> {
private:
    using Base = ConcurrentQueueBase<T, ConcurrencyType::SPSC_Locked>;
    using size_type = typename Base::size_type;

public:
    /**
     * @brief 构造函数
     * @param capacity 队列容量
     */
    explicit ConcurrentQueue(size_type capacity)
        : Base(capacity),
          buffer_(new T[this->capacity_]),
          mask_(this->capacity_ - 1) {}

    /**
     * @brief 析构函数
     */
    ~ConcurrentQueue() override {
        std::lock_guard<std::mutex> lock(mutex_);
        // 析构所有有效对象
        while (head_ != tail_) {
            buffer_[head_ & mask_].~T();
            head_ = (head_ + 1) & (2 * this->capacity_ - 1);
        }
        delete[] buffer_;
    }

    /**
     * @brief 推入元素
     * @tparam U 元素类型
     * @param item 要推入的元素
     * @return 如果成功推入则返回true，队列满则返回false
     */
    template <typename U>
    bool push(U&& item) {
        return emplace(std::forward<U>(item));
    }

    /**
     * @brief 原地构造并推入元素
     * @tparam Args 构造参数类型
     * @param args 构造参数
     * @return 如果成功推入则返回true，队列满则返回false
     */
    template <typename... Args>
    bool emplace(Args&&... args) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // 检查队列是否已满
        if ((tail_ - head_) >= this->capacity_) {
            return false;
        }
        
        // 在尾部位置构造新元素
        new (&buffer_[tail_ & mask_]) T(std::forward<Args>(args)...);
        
        // 更新尾部索引
        tail_ = (tail_ + 1) & (2 * this->capacity_ - 1);
        
        // 通知可能等待的消费者
        not_empty_cv_.notify_one();
        return true;
    }

    /**
     * @brief 弹出元素
     * @param item 存储弹出元素的引用
     * @return 如果成功弹出则返回true，队列空则返回false
     */
    bool pop(T& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // 检查队列是否为空
        if (head_ == tail_) {
            return false;
        }
        
        // 移动元素到输出参数
        item = std::move(buffer_[head_ & mask_]);
        
        // 析构原位置的元素
        buffer_[head_ & mask_].~T();
        
        // 更新头部索引
        head_ = (head_ + 1) & (2 * this->capacity_ - 1);
        
        // 通知可能等待的生产者
        not_full_cv_.notify_one();
        return true;
    }

    /**
     * @brief 尝试弹出元素（C++17 std::optional版本）
     * @return 如果成功则返回包含元素的optional，否则返回空optional
     */
    std::optional<T> try_pop() {
        T item;
        if (pop(item)) {
            return std::optional<T>(std::move(item));
        }
        return std::nullopt;
    }

    /**
     * @brief 等待并弹出元素
     * @param item 存储弹出元素的引用
     * 
     * 此方法会阻塞直到队列中有元素可弹出
     */
    void wait_and_pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // 等待队列非空
        not_empty_cv_.wait(lock, [this] { return head_ != tail_; });
        
        // 移动元素到输出参数
        item = std::move(buffer_[head_ & mask_]);
        
        // 析构原位置的元素
        buffer_[head_ & mask_].~T();
        
        // 更新头部索引
        head_ = (head_ + 1) & (2 * this->capacity_ - 1);
        
        // 通知可能等待的生产者
        not_full_cv_.notify_one();
    }

    /**
     * @brief 等待并推入元素
     * @tparam U 元素类型
     * @param item 要推入的元素
     * 
     * 此方法会阻塞直到队列有空间可用
     */
    template <typename U>
    void wait_and_push(U&& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // 等待队列非满
        not_full_cv_.wait(lock, [this] { return (tail_ - head_) < this->capacity_; });
        
        // 在尾部位置构造新元素
        new (&buffer_[tail_ & mask_]) T(std::forward<U>(item));
        
        // 更新尾部索引
        tail_ = (tail_ + 1) & (2 * this->capacity_ - 1);
        
        // 通知可能等待的消费者
        not_empty_cv_.notify_one();
    }

    /**
     * @brief 检查队列是否为空
     * @return 如果队列为空则返回true
     */
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return head_ == tail_;
    }

    /**
     * @brief 获取队列中的元素数量
     * @return 队列中的元素数量
     */
    size_type size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return (tail_ - head_) & (2 * this->capacity_ - 1);
    }

private:
    T* buffer_;                      // 存储数组
    const size_type mask_;           // 索引掩码
    size_type head_ = 0;             // 头部索引（读取位置）
    size_type tail_ = 0;             // 尾部索引（写入位置）
    
    mutable std::mutex mutex_;       // 互斥锁
    std::condition_variable not_empty_cv_; // 非空条件变量
    std::condition_variable not_full_cv_;  // 非满条件变量
};

/**
 * @brief 多生产者多消费者无锁队列实现
 * 
 * 使用原子操作实现的高性能MPMC队列
 * 
 * @tparam T 队列元素类型
 */
template <typename T>
requires QueueElement<T, ConcurrencyType::MPMC_LockFree>
class ConcurrentQueue<T, ConcurrencyType::MPMC_LockFree>
    : public ConcurrentQueueBase<T, ConcurrencyType::MPMC_LockFree> {
private:
    using Base = ConcurrentQueueBase<T, ConcurrencyType::MPMC_LockFree>;
    using size_type = typename Base::size_type;

    // 内部存储槽位（缓存行对齐）
    struct alignas(64) Slot {
        /**
         * @brief 构造函数
         */
        Slot() noexcept : sequence(0) {}

        /**
         * @brief 析构函数
         */
        ~Slot() = default;

        /**
         * @brief 在槽位构造对象
         * @tparam Args 构造参数类型
         * @param seq 序列号
         * @param args 构造参数
         */
        template <typename... Args>
        void construct(size_type seq, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
            new (&storage) T(std::forward<Args>(args)...);
            // 内存屏障确保对象构造在序列号更新之前完成
            std::atomic_thread_fence(std::memory_order_release);
            sequence.store(seq, std::memory_order_release);
        }

        /**
         * @brief 从槽位获取对象
         * @param seq 期望的序列号
         * @param item 存储获取对象的引用
         * @return 如果成功获取则返回true
         */
        bool consume(size_type seq, T& item, const size_type& capacity) noexcept(std::is_nothrow_move_assignable_v<T>) {
            if (sequence.load(std::memory_order_acquire) != seq) {
                return false;
            }
            
            item = std::move(*std::launder(reinterpret_cast<T*>(&storage)));
            std::launder(reinterpret_cast<T*>(&storage))->~T();
            
            // 更新序列号，标记为下一个可写入的位置
            sequence.store(seq + 1 + capacity, std::memory_order_release);
            return true;
        }

    private:
        std::aligned_storage_t<sizeof(T), alignof(T)> storage; // 存储对象的内存
        std::atomic<size_type> sequence;                       // 序列号
    };

public:
    /**
     * @brief 构造函数
     * @param capacity 队列容量
     */
    explicit ConcurrentQueue(size_type capacity)
        : Base(capacity),
          mask_(this->capacity_ - 1),
          buffer_(new Slot[this->capacity_]) {
        // 初始化所有槽位的序列号
        for (size_type i = 0; i < this->capacity_; ++i) {
            buffer_[i].sequence.store(i, std::memory_order_relaxed);
        }
        
        enqueue_pos_.store(0, std::memory_order_relaxed);
        dequeue_pos_.store(0, std::memory_order_relaxed);
    }

    /**
     * @brief 析构函数
     */
    ~ConcurrentQueue() override {
        // 注意：这里可能有竞态条件，实际应用中应确保析构前队列不再使用
        // 消费剩余元素
        T temp;
        while (pop(temp)) {}  // 利用现有pop机制安全析构
        delete[] buffer_;
    }

    /**
     * @brief 推入元素
     * @tparam U 元素类型
     * @param item 要推入的元素
     * @return 如果成功推入则返回true，队列满则返回false
     */
    template <typename U>
    bool push(U&& item) noexcept(std::is_nothrow_constructible_v<T, U&&>) {
        return emplace(std::forward<U>(item));
    }

    /**
     * @brief 原地构造并推入元素
     * @tparam Args 构造参数类型
     * @param args 构造参数
     * @return 如果成功推入则返回true，队列满则返回false
     */
    template <typename... Args>
    bool emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
        size_type pos = enqueue_pos_.load(std::memory_order_relaxed);
        
        for (;;) {
            // 获取当前位置的槽位
            Slot& slot = buffer_[pos & mask_];
            // 获取槽位的序列号
            size_type seq = slot.sequence.load(std::memory_order_acquire);
            // 计算差值
            intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos);
            
            // 如果序列号等于位置，说明槽位可写入
            if (diff == 0) {
                // 尝试原子地更新入队位置
                if (enqueue_pos_.compare_exchange_weak(
                    pos, pos + 1, std::memory_order_relaxed)) {
                    // 成功获取槽位，构造对象
                    slot.construct(pos + 1, std::forward<Args>(args)...);
                    return true;
                }
                // 失败则重试
            }
            // 如果序列号小于位置，说明队列已满
            else if (diff < 0) {
                return false;
            }
            // 序列号大于位置，说明其他线程已经更新了入队位置，更新本地位置
            else {
                pos = enqueue_pos_.load(std::memory_order_relaxed);
            }
        }
    }

    /**
     * @brief 弹出元素
     * @param item 存储弹出元素的引用
     * @return 如果成功弹出则返回true，队列空则返回false
     */
    bool pop(T& item) noexcept(std::is_nothrow_move_assignable_v<T>) {
        size_type pos = dequeue_pos_.load(std::memory_order_relaxed);
        
        for (;;) {
            // 获取当前位置的槽位
            Slot& slot = buffer_[pos & mask_];
            // 获取槽位的序列号
            size_type seq = slot.sequence.load(std::memory_order_acquire);
            // 计算差值
            intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos + 1);
            
            // 如果序列号等于位置+1，说明槽位有数据可读
            if (diff == 0) {
                // 尝试原子地更新出队位置
                if (dequeue_pos_.compare_exchange_weak(
                    pos, pos + 1, std::memory_order_relaxed)) {
                    // 成功获取槽位，消费对象
                    slot.consume(pos + 1, item, this->capacity_);
                    return true;
                }
                // 失败则重试
            }
            // 如果序列号小于位置+1，说明队列为空
            else if (diff < 0) {
                return false;
            }
            // 序列号大于位置+1，说明其他线程已经更新了出队位置，更新本地位置
            else {
                pos = dequeue_pos_.load(std::memory_order_relaxed);
            }
        }
    }

    /**
     * @brief 尝试弹出元素（C++17 std::optional版本）
     * @return 如果成功则返回包含元素的optional，否则返回空optional
     */
    std::optional<T> try_pop() noexcept(std::is_nothrow_move_constructible_v<T>) {
        T item;
        if (pop(item)) {
            return std::optional<T>(std::move(item));
        }
        return std::nullopt;
    }

    /**
     * @brief 检查队列是否为空（近似值）
     * @return 如果队列可能为空则返回true
     * 
     * 注意：在并发环境中，此值可能在返回后立即过时
     */
    bool empty() const noexcept {
        size_type enqueue = enqueue_pos_.load(std::memory_order_relaxed);
        size_type dequeue = dequeue_pos_.load(std::memory_order_relaxed);
        return enqueue == dequeue;
    }

    /**
     * @brief 获取队列中的元素数量（近似值）
     * @return 队列中的元素数量
     * 
     * 注意：在并发环境中，此值可能在返回后立即过时
     */
    size_type size() const noexcept {
        size_type enqueue = enqueue_pos_.load(std::memory_order_acquire);
        size_type dequeue = dequeue_pos_.load(std::memory_order_acquire);
        return enqueue - dequeue;
    }

private:
    const size_type mask_;                   // 索引掩码
    Slot* buffer_;                           // 存储槽位数组
    alignas(64) std::atomic<size_type> enqueue_pos_; // 入队位置
    alignas(64) std::atomic<size_type> dequeue_pos_; // 出队位置
};

/**
 * @brief 多生产者多消费者有锁队列实现
 * 
 * 使用读写锁实现的MPMC队列，适合读写不均衡的场景
 * 
 * @tparam T 队列元素类型
 */
template <typename T>
requires QueueElement<T, ConcurrencyType::MPMC_Locked>
class ConcurrentQueue<T, ConcurrencyType::MPMC_Locked>
    : public ConcurrentQueueBase<T, ConcurrencyType::MPMC_Locked> {
private:
    using Base = ConcurrentQueueBase<T, ConcurrencyType::MPMC_Locked>;
    using size_type = typename Base::size_type;

public:
    /**
     * @brief 构造函数
     * @param capacity 队列容量
     */
    explicit ConcurrentQueue(size_type capacity)
        : Base(capacity),
          buffer_(new T[this->capacity_]),
          mask_(this->capacity_ - 1) {}

    /**
     * @brief 析构函数
     */
    ~ConcurrentQueue() override {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        // 析构所有有效对象
        while (head_ != tail_) {
            buffer_[head_ & mask_].~T();
            head_ = (head_ + 1) & (2 * this->capacity_ - 1);
        }
        delete[] buffer_;
    }

    /**
     * @brief 推入元素
     * @tparam U 元素类型
     * @param item 要推入的元素
     * @return 如果成功推入则返回true，队列满则返回false
     */
    template <typename U>
    bool push(U&& item) {
        return emplace(std::forward<U>(item));
    }

    /**
     * @brief 原地构造并推入元素
     * @tparam Args 构造参数类型
     * @param args 构造参数
     * @return 如果成功推入则返回true，队列满则返回false
     */
    template <typename... Args>
    bool emplace(Args&&... args) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        // 检查队列是否已满
        if ((tail_ - head_) >= this->capacity_) {
            return false;
        }
        
        // 在尾部位置构造新元素
        new (&buffer_[tail_ & mask_]) T(std::forward<Args>(args)...);
        
        // 更新尾部索引
        tail_ = (tail_ + 1) & (2 * this->capacity_ - 1);
        
        // 通知可能等待的消费者
        lock.unlock();
        cv_.notify_all();
        return true;
    }

    /**
     * @brief 弹出元素
     * @param item 存储弹出元素的引用
     * @return 如果成功弹出则返回true，队列空则返回false
     */
    bool pop(T& item) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        // 检查队列是否为空
        if (head_ == tail_) {
            return false;
        }
        
        // 移动元素到输出参数
        item = std::move(buffer_[head_ & mask_]);
        
        // 析构原位置的元素
        buffer_[head_ & mask_].~T();
        
        // 更新头部索引
        head_ = (head_ + 1) & (2 * this->capacity_ - 1);
        
        return true;
    }

    /**
     * @brief 尝试弹出元素（C++17 std::optional版本）
     * @return 如果成功则返回包含元素的optional，否则返回空optional
     */
    std::optional<T> try_pop() {
        T item;
        if (pop(item)) {
            return std::optional<T>(std::move(item));
        }
        return std::nullopt;
    }

    /**
     * @brief 等待并弹出元素
     * @param item 存储弹出元素的引用
     * 
     * 此方法会阻塞直到队列中有元素可弹出
     */
    void wait_and_pop(T& item) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        // 等待队列非空
        cv_.wait(lock, [this] { return head_ != tail_; });
        
        // 移动元素到输出参数
        item = std::move(buffer_[head_ & mask_]);
        
        // 析构原位置的元素
        buffer_[head_ & mask_].~T();
        
        // 更新头部索引
        head_ = (head_ + 1) & (2 * this->capacity_ - 1);
    }

    /**
     * @brief 等待并推入元素
     * @tparam U 元素类型
     * @param item 要推入的元素
     * 
     * 此方法会阻塞直到队列有空间可用
     */
    template <typename U>
    void wait_and_push(U&& item) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        // 等待队列非满
        cv_.wait(lock, [this] { return (tail_ - head_) < this->capacity_; });
        
        // 在尾部位置构造新元素
        new (&buffer_[tail_ & mask_]) T(std::forward<U>(item));
        
        // 更新尾部索引
        tail_ = (tail_ + 1) & (2 * this->capacity_ - 1);
        
        // 通知可能等待的消费者
        lock.unlock();
        cv_.notify_all();
    }

    /**
     * @brief 批量弹出元素
     * @param output 输出迭代器
     * @param max_items 最大弹出元素数量
     * @return 实际弹出的元素数量
     */
    template <typename OutputIt>
    size_type pop_bulk(OutputIt output, size_type max_items) {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        
        size_type available = size_locked();
        size_type to_pop = std::min(available, max_items);
        
        if (to_pop == 0) {
            return 0;
        }
        
        // 升级为独占锁
        lock.unlock();
        std::unique_lock<std::shared_mutex> unique_lock(mutex_);
        
        // 再次检查大小（可能在锁升级期间发生变化）
        available = size_locked();
        to_pop = std::min(available, max_items);
        
        for (size_type i = 0; i < to_pop; ++i) {
            *output++ = std::move(buffer_[head_ & mask_]);
            buffer_[head_ & mask_].~T();
            head_ = (head_ + 1) & (2 * this->capacity_ - 1);
        }
        
        return to_pop;
    }

    /**
     * @brief 批量推入元素
     * @tparam InputIt 输入迭代器类型
     * @param first 输入范围起始迭代器
     * @param last 输入范围结束迭代器
     * @return 实际推入的元素数量
     */
    template <typename InputIt>
    size_type push_bulk(InputIt first, InputIt last) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        size_type available = this->capacity_ - size_locked();
        size_type count = 0;
        
        while (first != last && count < available) {
            new (&buffer_[tail_ & mask_]) T(*first++);
            tail_ = (tail_ + 1) & (2 * this->capacity_ - 1);
            ++count;
        }
        
        if (count > 0) {
            lock.unlock();
            cv_.notify_all();
        }
        
        return count;
    }

    /**
     * @brief 检查队列是否为空
     * @return 如果队列为空则返回true
     */
    bool empty() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return head_ == tail_;
    }

    /**
     * @brief 获取队列中的元素数量
     * @return 队列中的元素数量
     */
    size_type size() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return size_locked();
    }

private:
    /**
     * @brief 获取队列大小（已加锁版本）
     * @return 队列中的元素数量
     */
    size_type size_locked() const noexcept {
        return (tail_ - head_) & (2 * this->capacity_ - 1);
    }

private:
    T* buffer_;                      // 存储数组
    const size_type mask_;           // 索引掩码
    size_type head_ = 0;             // 头部索引（读取位置）
    size_type tail_ = 0;             // 尾部索引（写入位置）
    
    mutable std::shared_mutex mutex_; // 读写锁
    std::condition_variable_any cv_;  // 条件变量
};

/**
 * @brief 并发队列类模板
 * 
 * 这是一个通用的并发队列模板，支持不同的并发策略
 * 
 * @tparam T 队列元素类型
 * @tparam Type 并发策略类型，默认为MPMC_Locked
 */
template <typename T, ConcurrencyType Type = ConcurrencyType::MPMC_Locked>
class ConcurrentQueue;

// 使用示例
/*
// 创建一个SPSC无锁队列
ConcurrentQueue<int, ConcurrencyType::SPSC_LockFree> spsc_queue(1024);

// 创建一个MPMC有锁队列
ConcurrentQueue<std::string, ConcurrencyType::MPMC_Locked> mpmc_queue(128);

// 生产者线程
std::thread producer([&]() {
    for (int i = 0; i < 1000; ++i) {
        while (!spsc_queue.push(i)) {
            std::this_thread::yield();
        }
    }
});

// 消费者线程
std::thread consumer([&]() {
    int value;
    for (int i = 0; i < 1000; ++i) {
        while (!spsc_queue.pop(value)) {
            std::this_thread::yield();
        }
        std::cout << value << " ";
    }
});

producer.join();
consumer.join();
*/

#endif // !_CONCURRENT_QUEUE_HPP_