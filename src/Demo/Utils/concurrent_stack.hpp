#ifndef _CONCURRENT_STACK_HPP_
#define _CONCURRENT_STACK_HPP_

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
#include <array>     // 用于风险指针
#include <algorithm> // 用于风险指针

/**
 * @brief 并发栈的并发策略枚举
 * 
 * 定义了四种并发策略：
 * - SPSC_LockFree: 单生产者单消费者无锁实现
 * - SPSC_Locked: 单生产者单消费者有锁实现
 * - MPMC_LockFree: 多生产者多消费者无锁实现（使用风险指针）
 * - MPMC_Locked: 多生产者多消费者有锁实现
 */
enum class ConcurrencyType {
    SPSC_LockFree,  // 单生产者单消费者无锁
    SPSC_Locked,   // 单生产者单消费者有锁
    MPMC_LockFree, // 多生产者多消费者无锁
    MPMC_Locked    // 多生产者多消费者有锁
};

/**
 * @brief 并发栈的特性标签结构体
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
 * @brief 检查类型是否满足栈元素要求的概念
 * 
 * 对于无锁实现，要求类型必须是无抛出移动构造和移动赋值的
 */
template <typename T, ConcurrencyType Type>
concept StackElement = requires {
    requires std::is_nothrow_destructible_v<T>;
    requires !ConcurrencyTrait<Type>::is_lock_free || 
             (std::is_nothrow_move_constructible_v<T> && 
              std::is_nothrow_move_assignable_v<T>);
};

/**
 * @brief 并发栈的基类，提供公共接口和基本功能
 * 
 * @tparam T 栈元素类型
 * @tparam Type 并发策略类型
 * @tparam Traits 并发特性类型
 */
template <typename T, ConcurrencyType Type, typename Traits = ConcurrencyTrait<Type>>
class ConcurrentStackBase {
 public:
    // 类型定义
    using value_type = T;
    using size_type = std::size_t;
    using reference = T&;
    using const_reference = const T&;
    using traits_type = Traits;

    // 禁止拷贝和移动
    ConcurrentStackBase(const ConcurrentStackBase&) = delete;
    ConcurrentStackBase& operator=(const ConcurrentStackBase&) = delete;
    ConcurrentStackBase(ConcurrentStackBase&&) = delete;
    ConcurrentStackBase& operator=(ConcurrentStackBase&&) = delete;

    /**
     * @brief 获取栈容量
     * @return 栈的容量（元素数量）
     */
    size_type capacity() const noexcept {
        return capacity_;
    }

    /**
     * @brief 获取栈名称
     * @return 栈的名称（基于并发策略）
     */
    static constexpr const char* name() noexcept {
        return Traits::name;
    }

    /**
     * @brief 检查栈是否为无锁实现
     * @return 如果是无锁实现则返回true
     */
    static constexpr bool is_lock_free() noexcept {
        return Traits::is_lock_free;
    }

    /**
     * @brief 检查栈是否支持多生产者
     * @return 如果支持多生产者则返回true
     */
    static constexpr bool is_multi_producer() noexcept {
        return Traits::is_multi_producer;
    }

    /**
     * @brief 检查栈是否支持多消费者
     * @return 如果支持多消费者则返回true
     */
    static constexpr bool is_multi_consumer() noexcept {
        return Traits::is_multi_consumer;
    }

protected:
    /**
     * @brief 构造函数
     * @param capacity 栈容量
     */
    explicit ConcurrentStackBase(size_type capacity) noexcept
        : capacity_(capacity) {}

    /**
     * @brief 析构函数
     */
    virtual ~ConcurrentStackBase() = default;

    // 栈容量
    const size_type capacity_;
};

// 前向声明
template <typename T, ConcurrencyType Type = ConcurrencyType::MPMC_Locked>
class ConcurrentStack;

/**
 * @brief 风险指针管理器
 * 
 * 用于MPMC无锁栈的内存管理，防止ABA问题和内存泄漏
 */
class HazardPointerManager {
public:
    // 每个线程的风险指针数量
    static constexpr size_t MAX_HAZARD_POINTERS_PER_THREAD = 2;
    // 最大线程数
    static constexpr size_t MAX_THREADS = 128;
    // 回收阈值
    static constexpr size_t SCAN_THRESHOLD = 1000;

    /**
     * @brief 风险指针记录
     */
    struct HazardPointer {
        std::atomic<std::thread::id> thread_id;
        std::atomic<void*> pointer;

        HazardPointer() : thread_id(std::thread::id()), pointer(nullptr) {}
    };

    /**
     * @brief 获取风险指针管理器单例
     * @return 风险指针管理器引用
     */
    static HazardPointerManager& instance() {
        static HazardPointerManager instance;
        return instance;
    }

    /**
     * @brief 获取当前线程的风险指针
     * @param index 风险指针索引
     * @return 风险指针引用
     */
    std::atomic<void*>& get_hazard_pointer(size_t index) {
        auto thread_id = std::this_thread::get_id();
        for (size_t i = 0; i < MAX_THREADS * MAX_HAZARD_POINTERS_PER_THREAD; ++i) {
            std::thread::id expected;
            if (hp_list_[i].thread_id.load(std::memory_order_relaxed) == thread_id) {
                return hp_list_[i + index].pointer;
            }
            if (hp_list_[i].thread_id.compare_exchange_strong(
                expected, thread_id, std::memory_order_relaxed)) {
                return hp_list_[i + index].pointer;
            }
        }
        throw std::runtime_error("Too many threads using hazard pointers");
    }

    /**
     * @brief 添加待回收的指针
     * @param p 待回收的指针
     * @param deleter 删除器函数
     */
    void retire_pointer(void* p, std::function<void(void*)> deleter) {
        if (!p) return;

        retired_list_.emplace_back(p, deleter);

        if (retired_list_.size() >= SCAN_THRESHOLD) {
            scan();
        }
    }

    /**
     * @brief 扫描并回收不再被引用的指针
     */
    void scan() {
        std::vector<void*> hazard_pointers;
        for (size_t i = 0; i < MAX_THREADS * MAX_HAZARD_POINTERS_PER_THREAD; ++i) {
            void* p = hp_list_[i].pointer.load(std::memory_order_relaxed);
            if (p) {
                hazard_pointers.push_back(p);
            }
        }

        std::sort(hazard_pointers.begin(), hazard_pointers.end());
        auto unique_end = std::unique(hazard_pointers.begin(), hazard_pointers.end());
        hazard_pointers.erase(unique_end, hazard_pointers.end());

        std::vector<RetiredPointer> still_hazardous;
        for (auto& retired : retired_list_) {
            if (std::binary_search(hazard_pointers.begin(), hazard_pointers.end(), retired.pointer)) {
                still_hazardous.push_back(retired);
            } else {
                retired.deleter(retired.pointer);
            }
        }

        retired_list_.swap(still_hazardous);
    }

private:
    /**
     * @brief 待回收的指针记录
     */
    struct RetiredPointer {
        void* pointer;
        std::function<void(void*)> deleter;

        RetiredPointer(void* p, std::function<void(void*)> d)
            : pointer(p), deleter(d) {}
    };

    // 构造函数
    HazardPointerManager() {
        for (auto& hp : hp_list_) {
            hp.thread_id.store(std::thread::id(), std::memory_order_relaxed);
            hp.pointer.store(nullptr, std::memory_order_relaxed);
        }
    }

    // 析构函数
    ~HazardPointerManager() {
        scan(); // 最终扫描，回收所有待回收的指针
    }

    // 风险指针列表
    std::array<HazardPointer, MAX_THREADS * MAX_HAZARD_POINTERS_PER_THREAD> hp_list_;
    // 待回收指针列表
    std::vector<RetiredPointer> retired_list_;
};

/**
 * @brief 风险指针包装器
 * 
 * 用于自动管理风险指针的生命周期
 */
class HazardPointer {
public:
    /**
     * @brief 构造函数
     * @param index 风险指针索引
     */
    explicit HazardPointer(size_t index)
        : hp_(HazardPointerManager::instance().get_hazard_pointer(index)) {
        hp_.store(nullptr, std::memory_order_relaxed);
    }

    /**
     * @brief 析构函数
     */
    ~HazardPointer() {
        hp_.store(nullptr, std::memory_order_release);
    }

    /**
     * @brief 保护指针
     * @param p 要保护的指针
     */
    void protect(void* p) {
        hp_.store(p, std::memory_order_release);
    }

    /**
     * @brief 重置指针
     */
    void reset() {
        hp_.store(nullptr, std::memory_order_release);
    }

private:
    std::atomic<void*>& hp_;
};

/**
 * @brief 单生产者单消费者无锁栈实现
 * 
 * 这是一个高性能的无锁栈实现，专为SPSC场景优化
 * 
 * @tparam T 栈元素类型
 */
template <typename T>
requires StackElement<T, ConcurrencyType::SPSC_LockFree>
class ConcurrentStack<T, ConcurrencyType::SPSC_LockFree>
    : public ConcurrentStackBase<T, ConcurrencyType::SPSC_LockFree> {
private:
    using Base = ConcurrentStackBase<T, ConcurrencyType::SPSC_LockFree>;
    using size_type = typename Base::size_type;

    // 内部节点结构
    struct Node {
        T data;
        Node* next;

        template <typename... Args>
        explicit Node(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
            : data(std::forward<Args>(args)...), next(nullptr) {}
    };

public:
    /**
     * @brief 构造函数
     * @param capacity 栈容量
     */
    explicit ConcurrentStack(size_type capacity)
        : Base(capacity), head_(nullptr), size_(0) {}

    /**
     * @brief 析构函数
     */
    ~ConcurrentStack() override {
        clear();
    }

    /**
     * @brief 清空栈
     */
    void clear() noexcept {
        Node* current = head_.load(std::memory_order_relaxed);
        while (current) {
            Node* next = current->next;
            delete current;
            current = next;
        }
        head_.store(nullptr, std::memory_order_relaxed);
        size_.store(0, std::memory_order_relaxed);
    }

    /**
     * @brief 推入元素
     * @tparam U 元素类型
     * @param item 要推入的元素
     * @return 如果成功推入则返回true，栈满则返回false
     */
    template <typename U>
    bool push(U&& item) noexcept(std::is_nothrow_constructible_v<T, U&&>) {
        return emplace(std::forward<U>(item));
    }

    /**
     * @brief 原地构造并推入元素
     * @tparam Args 构造参数类型
     * @param args 构造参数
     * @return 如果成功推入则返回true，栈满则返回false
     */
    template <typename... Args>
    bool emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
        // 检查栈是否已满
        if (size_.load(std::memory_order_relaxed) >= this->capacity_) {
            return false;
        }

        // 创建新节点
        Node* new_node = new Node(std::forward<Args>(args)...);
        
        // 获取当前头节点
        Node* old_head = head_.load(std::memory_order_relaxed);
        
        // 设置新节点的next指针
        new_node->next = old_head;
        
        // 更新头节点
        head_.store(new_node, std::memory_order_release);
        
        // 更新大小
        size_.fetch_add(1, std::memory_order_relaxed);
        
        return true;
    }

    /**
     * @brief 弹出元素
     * @param item 存储弹出元素的引用
     * @return 如果成功弹出则返回true，栈空则返回false
     */
    bool pop(T& item) noexcept(std::is_nothrow_move_assignable_v<T>) {
        // 获取当前头节点
        Node* old_head = head_.load(std::memory_order_acquire);
        
        // 检查栈是否为空
        if (!old_head) {
            return false;
        }
        
        // 更新头节点
        head_.store(old_head->next, std::memory_order_relaxed);
        
        // 获取数据
        item = std::move(old_head->data);
        
        // 删除旧节点
        delete old_head;
        
        // 更新大小
        size_.fetch_sub(1, std::memory_order_relaxed);
        
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
     * @brief 检查栈是否为空
     * @return 如果栈为空则返回true
     */
    bool empty() const noexcept {
        return head_.load(std::memory_order_acquire) == nullptr;
    }

    /**
     * @brief 获取栈中的元素数量
     * @return 栈中的元素数量
     */
    size_type size() const noexcept {
        return size_.load(std::memory_order_acquire);
    }

private:
    std::atomic<Node*> head_; // 头节点
    std::atomic<size_type> size_; // 元素数量
};

/**
 * @brief 单生产者单消费者有锁栈实现
 * 
 * 使用互斥锁和条件变量实现的SPSC栈
 * 
 * @tparam T 栈元素类型
 */
template <typename T>
requires StackElement<T, ConcurrencyType::SPSC_Locked>
class ConcurrentStack<T, ConcurrencyType::SPSC_Locked>
    : public ConcurrentStackBase<T, ConcurrencyType::SPSC_Locked> {
private:
    using Base = ConcurrentStackBase<T, ConcurrencyType::SPSC_Locked>;
    using size_type = typename Base::size_type;

    // 内部节点结构
    struct Node {
        T data;
        Node* next;

        template <typename... Args>
        explicit Node(Args&&... args)
            : data(std::forward<Args>(args)...), next(nullptr) {}
    };

public:
    /**
     * @brief 构造函数
     * @param capacity 栈容量
     */
    explicit ConcurrentStack(size_type capacity)
        : Base(capacity), head_(nullptr), size_(0) {}

    /**
     * @brief 析构函数
     */
    ~ConcurrentStack() override {
        clear();
    }

    /**
     * @brief 清空栈
     */
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        Node* current = head_;
        while (current) {
            Node* next = current->next;
            delete current;
            current = next;
        }
        head_ = nullptr;
        size_ = 0;
    }

    /**
     * @brief 推入元素
     * @tparam U 元素类型
     * @param item 要推入的元素
     * @return 如果成功推入则返回true，栈满则返回false
     */
    template <typename U>
    bool push(U&& item) {
        return emplace(std::forward<U>(item));
    }

    /**
     * @brief 原地构造并推入元素
     * @tparam Args 构造参数类型
     * @param args 构造参数
     * @return 如果成功推入则返回true，栈满则返回false
     */
    template <typename... Args>
    bool emplace(Args&&... args) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // 检查栈是否已满
        if (size_ >= this->capacity_) {
            return false;
        }
        
        // 创建新节点
        Node* new_node = new Node(std::forward<Args>(args)...);
        
        // 设置新节点的next指针
        new_node->next = head_;
        
        // 更新头节点
        head_ = new_node;
        
        // 更新大小
        ++size_;
        
        // 通知可能等待的消费者
        not_empty_cv_.notify_one();
        return true;
    }

    /**
     * @brief 弹出元素
     * @param item 存储弹出元素的引用
     * @return 如果成功弹出则返回true，栈空则返回false
     */
    bool pop(T& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // 检查栈是否为空
        if (!head_) {
            return false;
        }
        
        // 获取当前头节点
        Node* old_head = head_;
        
        // 更新头节点
        head_ = old_head->next;
        
        // 获取数据
        item = std::move(old_head->data);
        
        // 删除旧节点
        delete old_head;
        
        // 更新大小
        --size_;
        
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
     * 此方法会阻塞直到栈中有元素可弹出
     */
    void wait_and_pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // 等待栈非空
        not_empty_cv_.wait(lock, [this] { return head_ != nullptr; });
        
        // 获取当前头节点
        Node* old_head = head_;
        
        // 更新头节点
        head_ = old_head->next;
        
        // 获取数据
        item = std::move(old_head->data);
        
        // 删除旧节点
        delete old_head;
        
        // 更新大小
        --size_;
        
        // 通知可能等待的生产者
        not_full_cv_.notify_one();
    }

    /**
     * @brief 等待并推入元素
     * @tparam U 元素类型
     * @param item 要推入的元素
     * 
     * 此方法会阻塞直到栈有空间可用
     */
    template <typename U>
    void wait_and_push(U&& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // 等待栈非满
        not_full_cv_.wait(lock, [this] { return size_ < this->capacity_; });
        
        // 创建新节点
        Node* new_node = new Node(std::forward<U>(item));
        
        // 设置新节点的next指针
        new_node->next = head_;
        
        // 更新头节点
        head_ = new_node;
        
        // 更新大小
        ++size_;
        
        // 通知可能等待的消费者
        not_empty_cv_.notify_one();
    }

    /**
     * @brief 检查栈是否为空
     * @return 如果栈为空则返回true
     */
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return head_ == nullptr;
    }

    /**
     * @brief 获取栈中的元素数量
     * @return 栈中的元素数量
     */
    size_type size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return size_;
    }

private:
    Node* head_; // 头节点
    size_type size_; // 元素数量
    
    mutable std::mutex mutex_; // 互斥锁
    std::condition_variable not_empty_cv_; // 非空条件变量
    std::condition_variable not_full_cv_; // 非满条件变量
};

/**
 * @brief 多生产者多消费者无锁栈实现
 * 
 * 使用风险指针实现的高性能MPMC栈
 * 
 * @tparam T 栈元素类型
 */
template <typename T>
requires StackElement<T, ConcurrencyType::MPMC_LockFree>
class ConcurrentStack<T, ConcurrencyType::MPMC_LockFree>
    : public ConcurrentStackBase<T, ConcurrencyType::MPMC_LockFree> {
private:
    using Base = ConcurrentStackBase<T, ConcurrencyType::MPMC_LockFree>;
    using size_type = typename Base::size_type;

    // 内部节点结构
    struct Node {
        T data;
        std::atomic<Node*> next;

        template <typename... Args>
        explicit Node(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
            : data(std::forward<Args>(args)...), next(nullptr) {}

        // 空节点构造函数（用于哨兵节点）
        Node() : next(nullptr) {}
    };

public:
    /**
     * @brief 构造函数
     * @param capacity 栈容量
     */
    explicit ConcurrentStack(size_type capacity)
        : Base(capacity), head_(new Node()), size_(0) {}

    /**
     * @brief 析构函数
     */
    ~ConcurrentStack() override {
        clear();
        delete head_.load(std::memory_order_relaxed);
    }

    /**
     * @brief 清空栈
     */
    void clear() noexcept {
        T dummy;
        while (pop(dummy)) {}
    }

    /**
     * @brief 推入元素
     * @tparam U 元素类型
     * @param item 要推入的元素
     * @return 如果成功推入则返回true，栈满则返回false
     */
    template <typename U>
    bool push(U&& item) noexcept(std::is_nothrow_constructible_v<T, U&&>) {
        return emplace(std::forward<U>(item));
    }

    /**
     * @brief 原地构造并推入元素
     * @tparam Args 构造参数类型
     * @param args 构造参数
     * @return 如果成功推入则返回true，栈满则返回false
     */
    template <typename... Args>
    bool emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
        // 检查栈是否已满
		// 原子地增加大小并检查，防止容量控制错误
		size_type old_size = size_.fetch_add(1, std::memory_order_acquire);
		if (old_size >= this->capacity_) {
			// 回滚大小增加
			size_.fetch_sub(1, std::memory_order_release);
			return false;
		}

        // 创建新节点
        Node* new_node = new Node(std::forward<Args>(args)...);
        
        // 获取当前头节点
        Node* old_head = head_.load(std::memory_order_relaxed);
        
        do {
            // 设置新节点的next指针
            new_node->next.store(old_head, std::memory_order_relaxed);
            
            // 尝试更新头节点
        } while (!head_.compare_exchange_weak(
            old_head, new_node, std::memory_order_release, std::memory_order_relaxed));
        
        return true;
    }

    /**
     * @brief 弹出元素
     * @param item 存储弹出元素的引用
     * @return 如果成功弹出则返回true，栈空则返回false
     */
    bool pop(T& item) noexcept(std::is_nothrow_move_assignable_v<T>) {
        // 使用风险指针保护当前头节点
        HazardPointer hp(0);
        
        Node* old_head;
        Node* new_head;
        
        do {
            // 检查栈是否为空
            old_head = head_.load(std::memory_order_acquire);
            if (!old_head || !old_head->next.load(std::memory_order_relaxed)) {
                return false;
            }
            
            // 保护当前头节点
            hp.protect(old_head);
            
            // 检查头节点是否已经被修改
            if (old_head != head_.load(std::memory_order_relaxed)) {
                continue;
            }
            
            // 获取新的头节点
            new_head = old_head->next.load(std::memory_order_relaxed);
            
            // 尝试更新头节点
        } while (!head_.compare_exchange_weak(
            old_head, new_head, std::memory_order_release, std::memory_order_relaxed));
        
        // 获取数据
        item = std::move(old_head->data);
        
        // 更新大小
        size_.fetch_sub(1, std::memory_order_relaxed);
        
        // 安全回收旧节点
        hp.reset();
        HazardPointerManager::instance().retire_pointer(
            old_head, [](void* p) { delete static_cast<Node*>(p); });
        
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
     * @brief 检查栈是否为空
     * @return 如果栈为空则返回true
     */
    bool empty() const noexcept {
        Node* h = head_.load(std::memory_order_acquire);
        return !h || !h->next.load(std::memory_order_relaxed);
    }

    /**
     * @brief 获取栈中的元素数量
     * @return 栈中的元素数量
     */
    size_type size() const noexcept {
        return size_.load(std::memory_order_acquire);
    }

private:
    std::atomic<Node*> head_; // 头节点
    std::atomic<size_type> size_; // 元素数量
};

/**
 * @brief 多生产者多消费者有锁栈实现
 * 
 * 使用读写锁实现的MPMC栈，适合读写不均衡的场景
 * 
 * @tparam T 栈元素类型
 */
template <typename T>
requires StackElement<T, ConcurrencyType::MPMC_Locked>
class ConcurrentStack<T, ConcurrencyType::MPMC_Locked>
    : public ConcurrentStackBase<T, ConcurrencyType::MPMC_Locked> {
private:
    using Base = ConcurrentStackBase<T, ConcurrencyType::MPMC_Locked>;
    using size_type = typename Base::size_type;

    // 内部节点结构
    struct Node {
        T data;
        Node* next;

        template <typename... Args>
        explicit Node(Args&&... args)
            : data(std::forward<Args>(args)...), next(nullptr) {}
    };

public:
    /**
     * @brief 构造函数
     * @param capacity 栈容量
     */
    explicit ConcurrentStack(size_type capacity)
        : Base(capacity), head_(nullptr), size_(0) {}

    /**
     * @brief 析构函数
     */
    ~ConcurrentStack() override {
        clear();
    }

    /**
     * @brief 清空栈
     */
    void clear() {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        Node* current = head_;
        while (current) {
            Node* next = current->next;
            delete current;
            current = next;
        }
        head_ = nullptr;
        size_ = 0;
    }

    /**
     * @brief 推入元素
     * @tparam U 元素类型
     * @param item 要推入的元素
     * @return 如果成功推入则返回true，栈满则返回false
     */
    template <typename U>
    bool push(U&& item) {
        return emplace(std::forward<U>(item));
    }

    /**
     * @brief 原地构造并推入元素
     * @tparam Args 构造参数类型
     * @param args 构造参数
     * @return 如果成功推入则返回true，栈满则返回false
     */
    template <typename... Args>
    bool emplace(Args&&... args) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        // 检查栈是否已满
        if (size_ >= this->capacity_) {
            return false;
        }
        
        // 创建新节点
        Node* new_node = new Node(std::forward<Args>(args)...);
        
        // 设置新节点的next指针
        new_node->next = head_;
        
        // 更新头节点
        head_ = new_node;
        
        // 更新大小
        ++size_;
        
        // 通知可能等待的消费者
        lock.unlock();
        cv_.notify_all();
        return true;
    }

    /**
     * @brief 弹出元素
     * @param item 存储弹出元素的引用
     * @return 如果成功弹出则返回true，栈空则返回false
     */
    bool pop(T& item) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        // 检查栈是否为空
        if (!head_) {
            return false;
        }
        
        // 获取当前头节点
        Node* old_head = head_;
        
        // 更新头节点
        head_ = old_head->next;
        
        // 获取数据
        item = std::move(old_head->data);
        
        // 删除旧节点
        delete old_head;
        
        // 更新大小
        --size_;
        
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
     * 此方法会阻塞直到栈中有元素可弹出
     */
    void wait_and_pop(T& item) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        // 等待栈非空
        cv_.wait(lock, [this] { return head_ != nullptr; });
        
        // 获取当前头节点
        Node* old_head = head_;
        
        // 更新头节点
        head_ = old_head->next;
        
        // 获取数据
        item = std::move(old_head->data);
        
        // 删除旧节点
        delete old_head;
        
        // 更新大小
        --size_;
    }

    /**
     * @brief 等待并推入元素
     * @tparam U 元素类型
     * @param item 要推入的元素
     * 
     * 此方法会阻塞直到栈有空间可用
     */
    template <typename U>
    void wait_and_push(U&& item) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        // 等待栈非满
        cv_.wait(lock, [this] { return size_ < this->capacity_; });
        
        // 创建新节点
        Node* new_node = new Node(std::forward<U>(item));
        
        // 设置新节点的next指针
        new_node->next = head_;
        
        // 更新头节点
        head_ = new_node;
        
        // 更新大小
        ++size_;
        
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
        
        size_type available = size_;
        size_type to_pop = std::min(available, max_items);
        
        if (to_pop == 0) {
            return 0;
        }
        
        // 升级为独占锁
        lock.unlock();
        std::unique_lock<std::shared_mutex> unique_lock(mutex_);
        
        // 再次检查大小（可能在锁升级期间发生变化）
        available = size_;
        to_pop = std::min(available, max_items);
        
        for (size_type i = 0; i < to_pop; ++i) {
            Node* old_head = head_;
            head_ = old_head->next;
            *output++ = std::move(old_head->data);
            delete old_head;
        }
        
        size_ -= to_pop;
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
        
        size_type available = this->capacity_ - size_;
        size_type count = 0;
        
        while (first != last && count < available) {
            Node* new_node = new Node(*first++);
            new_node->next = head_;
            head_ = new_node;
            ++count;
        }
        
        size_ += count;
        
        if (count > 0) {
            lock.unlock();
            cv_.notify_all();
        }
        
        return count;
    }

    /**
     * @brief 检查栈是否为空
     * @return 如果栈为空则返回true
     */
    bool empty() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return head_ == nullptr;
    }

    /**
     * @brief 获取栈中的元素数量
     * @return 栈中的元素数量
     */
    size_type size() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return size_;
    }

private:
    Node* head_; // 头节点
    size_type size_; // 元素数量
    
    mutable std::shared_mutex mutex_; // 读写锁
    std::condition_variable_any cv_; // 条件变量
};

// 使用示例
/*
// 创建一个SPSC无锁栈
ConcurrentStack<int, ConcurrencyType::SPSC_LockFree> spsc_stack(1024);

// 创建一个MPMC有锁栈
ConcurrentStack<std::string, ConcurrencyType::MPMC_Locked> mpmc_stack(128);

// 生产者线程
std::thread producer([&]() {
    for (int i = 0; i < 1000; ++i) {
        while (!spsc_stack.push(i)) {
            std::this_thread::yield();
        }
    }
});

// 消费者线程
std::thread consumer([&]() {
    int value;
    for (int i = 0; i < 1000; ++i) {
        while (!spsc_stack.pop(value)) {
            std::this_thread::yield();
        }
        std::cout << value << " ";
    }
});

producer.join();
consumer.join();
*/

#endif // !_CONCURRENT_STACK_HPP_