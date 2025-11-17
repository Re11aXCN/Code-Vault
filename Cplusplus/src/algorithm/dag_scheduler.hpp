#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <atomic>
#include <concepts>
#include <type_traits>
#include <ranges>
#include <format>
//DAG最简任务工程调度框架，支持并发
/*
    DAG支持：完整的有向无环图任务依赖管理
    并发执行：使用线程池并发执行独立任务
    依赖检测：自动检测循环依赖
    任务状态跟踪：实时跟踪每个任务的状态
    异常处理：完善的异常处理和错误报告
*/
namespace dag_scheduler {

    // 概念定义
    template<typename T>
    concept TaskExecutor = requires(T executor, std::function<void()> task) {
        { executor.submit(std::move(task)) } -> std::same_as<void>;
        { executor.wait() } -> std::same_as<void>;
    };

    template<typename T>
    concept Task = requires(T task) {
        { task() } -> std::same_as<void>;
    };

    // 任务状态
    enum class TaskState {
        Pending,
        Ready,
        Running,
        Completed,
        Failed
    };

    // 基础任务接口
    class ITask {
    public:
        virtual ~ITask() = default;
        virtual void execute() = 0;
        virtual std::string id() const = 0;
        virtual TaskState state() const = 0;
        virtual void set_state(TaskState state) = 0;
        virtual const std::vector<std::string>& dependencies() const = 0;
    };

    // 具体任务实现
    template<Task F>
    class TaskImpl : public ITask {
    public:
        TaskImpl(std::string id, F&& func, std::vector<std::string> deps = {})
            : id_(std::move(id))
            , func_(std::forward<F>(func))
            , dependencies_(std::move(deps))
            , state_(TaskState::Pending) {
        }

        void execute() override {
            set_state(TaskState::Running);
            try {
                func_();
                set_state(TaskState::Completed);
            }
            catch (...) {
                set_state(TaskState::Failed);
                throw;
            }
        }

        std::string id() const override { return id_; }
        TaskState state() const override { return state_.load(); }
        void set_state(TaskState state) override { state_.store(state); }
        const std::vector<std::string>& dependencies() const override { return dependencies_; }

    private:
        std::string id_;
        F func_;
        std::vector<std::string> dependencies_;
        std::atomic<TaskState> state_;
    };

    // 线程池执行器
    class ThreadPoolExecutor {
    public:
        explicit ThreadPoolExecutor(size_t num_threads = std::thread::hardware_concurrency())
            : stop_(false) {
            for (size_t i = 0; i < num_threads; ++i) {
                workers_.emplace_back([this] { worker_loop(); });
            }
        }

        ~ThreadPoolExecutor() {
            {
                std::unique_lock lock(queue_mutex_);
                stop_ = true;
            }
            condition_.notify_all();
            for (auto& worker : workers_) {
                if (worker.joinable()) {
                    worker.join();
                }
            }
        }

        void submit(std::function<void()> task) {
            {
                std::unique_lock lock(queue_mutex_);
                if (stop_) {
                    throw std::runtime_error("Executor has been stopped");
                }
                tasks_.emplace(std::move(task));
            }
            condition_.notify_one();
        }

        void wait() {
            std::unique_lock lock(queue_mutex_);
            completed_.wait(lock, [this] {
                return tasks_.empty() && active_tasks_ == 0;
                });
        }

    private:
        void worker_loop() {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock lock(queue_mutex_);
                    condition_.wait(lock, [this] {
                        return stop_ || !tasks_.empty();
                        });

                    if (stop_ && tasks_.empty()) {
                        return;
                    }

                    task = std::move(tasks_.front());
                    tasks_.pop();
                    ++active_tasks_;
                }

                task();

                {
                    std::unique_lock lock(queue_mutex_);
                    --active_tasks_;
                    if (tasks_.empty() && active_tasks_ == 0) {
                        completed_.notify_all();
                    }
                }
            }
        }

        std::vector<std::thread> workers_;
        std::queue<std::function<void()>> tasks_;
        std::mutex queue_mutex_;
        std::condition_variable condition_;
        std::condition_variable completed_;
        std::atomic<bool> stop_;
        std::atomic<int> active_tasks_{ 0 };
    };

    // DAG调度器
    class DAGScheduler {
    public:
        explicit DAGScheduler(std::unique_ptr<TaskExecutor> executor = nullptr)
            : executor_(executor ? std::move(executor)
                : std::make_unique<ThreadPoolExecutor>()) {
        }

        // 添加任务
        template<Task F>
        void add_task(std::string id, F&& func, std::vector<std::string> dependencies = {}) {
            auto task = std::make_shared<TaskImpl<F>>(
                std::move(id), std::forward<F>(func), std::move(dependencies));

            std::lock_guard lock(mutex_);
            if (!tasks_.try_emplace(task->id(), task).second) {
                throw std::runtime_error(std::format("Task with id '{}' already exists", task->id()));
            }
        }

        // 添加依赖关系
        void add_dependency(const std::string& task_id, const std::string& dependency_id) {
            std::lock_guard lock(mutex_);

            auto task_it = tasks_.find(task_id);
            auto dep_it = tasks_.find(dependency_id);

            if (task_it == tasks_.end() || dep_it == tasks_.end()) {
                throw std::runtime_error("Task or dependency not found");
            }

            task_it->second->dependencies().push_back(dependency_id);
        }

        // 执行调度
        void execute() {
            std::lock_guard lock(mutex_);

            validate_dag();
            build_execution_graph();

            std::vector<std::string> ready_tasks = get_ready_tasks();

            while (!ready_tasks.empty() || !running_tasks_.empty()) {
                // 提交就绪任务
                for (const auto& task_id : ready_tasks) {
                    auto task = tasks_.at(task_id);
                    task->set_state(TaskState::Ready);

                    auto future = std::async(std::launch::async, [this, task] {
                        return execute_task(task);
                        });

                    running_tasks_.emplace(task_id, std::move(future));
                }

                // 等待任意任务完成
                if (!running_tasks_.empty()) {
                    wait_for_completion();
                    ready_tasks = get_ready_tasks();
                }
            }
        }

        // 获取任务状态
        TaskState get_task_state(const std::string& task_id) const {
            std::lock_guard lock(mutex_);
            if (auto it = tasks_.find(task_id); it != tasks_.end()) {
                return it->second->state();
            }
            throw std::runtime_error("Task not found");
        }

        // 重置调度器状态
        void reset() {
            std::lock_guard lock(mutex_);
            for (auto& [id, task] : tasks_) {
                task->set_state(TaskState::Pending);
            }
            running_tasks_.clear();
            completed_tasks_.clear();
        }

    private:
        void validate_dag() {
            std::unordered_set<std::string> visited, recursion_stack;

            for (const auto& [id, task] : tasks_) {
                if (!is_visited(visited, id)) {
                    if (has_cycle(id, visited, recursion_stack)) {
                        throw std::runtime_error("DAG contains cycles");
                    }
                }
            }
        }

        bool has_cycle(const std::string& task_id,
            std::unordered_set<std::string>& visited,
            std::unordered_set<std::string>& recursion_stack) {
            if (!recursion_stack.insert(task_id).second) {
                return true;
            }

            auto task = tasks_.at(task_id);
            for (const auto& dep_id : task->dependencies()) {
                if (!is_visited(visited, dep_id)) {
                    if (has_cycle(dep_id, visited, recursion_stack)) {
                        return true;
                    }
                }
                else if (recursion_stack.contains(dep_id)) {
                    return true;
                }
            }

            recursion_stack.erase(task_id);
            visited.insert(task_id);
            return false;
        }

        bool is_visited(const std::unordered_set<std::string>& visited, const std::string& id) {
            return visited.find(id) != visited.end();
        }

        void build_execution_graph() {
            in_degree_.clear();
            dependents_.clear();

            for (const auto& [id, task] : tasks_) {
                in_degree_[id] = task->dependencies().size();
                for (const auto& dep_id : task->dependencies()) {
                    dependents_[dep_id].push_back(id);
                }
            }
        }

        std::vector<std::string> get_ready_tasks() {
            std::vector<std::string> ready;

            for (const auto& [id, task] : tasks_) {
                if (task->state() == TaskState::Pending && in_degree_.at(id) == 0) {
                    ready.push_back(id);
                }
            }

            return ready;
        }

        void execute_task(std::shared_ptr<ITask> task) {
            try {
                task->execute();

                std::lock_guard lock(mutex_);
                completed_tasks_.insert(task->id());
                running_tasks_.erase(task->id());

                // 更新依赖任务入度
                if (auto it = dependents_.find(task->id()); it != dependents_.end()) {
                    for (const auto& dependent_id : it->second) {
                        --in_degree_[dependent_id];
                    }
                }

            }
            catch (const std::exception& e) {
                std::lock_guard lock(mutex_);
                std::cerr << std::format("Task '{}' failed: {}", task->id(), e.what()) << std::endl;
                running_tasks_.erase(task->id());
            }
        }

        void wait_for_completion() {
            // 等待任意任务完成
            for (auto it = running_tasks_.begin(); it != running_tasks_.end(); ) {
                auto& [id, future] = *it;

                if (future.wait_for(std::chrono::milliseconds(10)) == std::future_status::ready) {
                    try {
                        future.get(); // 获取结果，可能重新抛出异常
                    }
                    catch (...) {
                        // 异常已经在execute_task中处理
                    }
                    it = running_tasks_.erase(it);
                }
                else {
                    ++it;
                }
            }
        }

        std::unique_ptr<TaskExecutor> executor_;
        std::unordered_map<std::string, std::shared_ptr<ITask>> tasks_;
        std::unordered_map<std::string, int> in_degree_;
        std::unordered_map<std::string, std::vector<std::string>> dependents_;
        std::unordered_map<std::string, std::future<void>> running_tasks_;
        std::unordered_set<std::string> completed_tasks_;
        mutable std::mutex mutex_;
    };

} // namespace dag_scheduler
/*
// 使用示例
int main() {
    using namespace dag_scheduler;

    DAGScheduler scheduler;

    // 添加任务
    scheduler.add_task("task1", [] {
        std::cout << "Task 1 executing..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });

    scheduler.add_task("task2", [] {
        std::cout << "Task 2 executing..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        });

    scheduler.add_task("task3", [] {
        std::cout << "Task 3 executing..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        });

    scheduler.add_task("task4", [] {
        std::cout << "Task 4 executing..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }, { "task1", "task2" }); // task4 依赖 task1 和 task2

    scheduler.add_task("task5", [] {
        std::cout << "Task 5 executing..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(75));
        }, { "task3", "task4" }); // task5 依赖 task3 和 task4

    try {
        std::cout << "Starting DAG execution..." << std::endl;
        scheduler.execute();
        std::cout << "All tasks completed successfully!" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Scheduler error: " << e.what() << std::endl;
    }

    return 0;
}
*/