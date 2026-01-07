#pragma once

#include <atomic>
#include <concepts>
#include <condition_variable>
#include <format>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <ranges>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace stdex {

// DAG (Directed Acyclic Graph) Task Scheduler Framework
// Features:
// - Complete DAG task dependency management
// - Concurrent execution using thread pool
// - Automatic cycle detection
// - Real-time task state tracking
// - Comprehensive exception handling and error reporting
namespace detail {
// Concept definitions
template<typename ExecutorType>
concept TaskExecutor =
    requires (ExecutorType executor, std::function<void()> task) {
      { executor.submit(std::move(task)) } -> std::same_as<void>;
      { executor.wait() } -> std::same_as<void>;
    };

template<typename FunctionType>
concept Task = requires (FunctionType task) {
  { task() } -> std::same_as<void>;
};
}  // namespace detail
// Task state enumeration
enum class TaskState
{
  Pending,
  Ready,
  Running,
  Completed,
  Failed
};

// Base task interface
class TaskInterface
{
public:

  virtual ~TaskInterface()                                  = default;
  virtual void                            execute()         = 0;
  virtual std::string                     get_id() const    = 0;
  virtual TaskState                       get_state() const = 0;
  virtual void                            set_state(TaskState new_state) = 0;
  virtual const std::vector<std::string>& get_dependencies() const       = 0;
  virtual void add_dependency(const std::string& dependency_id)          = 0;
};

// Concrete task implementation
template<detail::Task FunctionType>
class TaskImplementation : public TaskInterface
{
public:

  TaskImplementation(std::string task_id, FunctionType&& function,
                     std::vector<std::string> dependencies = {})
      : m_TaskId(std::move(task_id))
      , m_Function(std::forward<FunctionType>(function))
      , m_Dependencies(std::move(dependencies))
      , m_State(TaskState::Pending)
  {
  }

  void execute() override
  {
    set_state(TaskState::Running);
    try {
      m_Function();
      set_state(TaskState::Completed);
    } catch (...) {
      set_state(TaskState::Failed);
      throw;
    }
  }

  std::string get_id() const override { return m_TaskId; }

  TaskState get_state() const override { return m_State.load(); }

  void set_state(TaskState new_state) override { m_State.store(new_state); }

  const std::vector<std::string>& get_dependencies() const override
  {
    return m_Dependencies;
  }

  void add_dependency(const std::string& dependency_id) override
  {
    m_Dependencies.push_back(dependency_id);
  }

private:

  std::string              m_TaskId;
  FunctionType             m_Function;
  std::vector<std::string> m_Dependencies;
  std::atomic<TaskState>   m_State;
};

// Thread pool executor
class ThreadPoolExecutor
{
public:

  explicit ThreadPoolExecutor(
      std::size_t thread_count = std::thread::hardware_concurrency())
      : m_Stop(false)
  {
    for (std::size_t i = 0; i < thread_count; ++i) {
      m_Workers.emplace_back([this] { worker_loop(); });
    }
  }

  ~ThreadPoolExecutor()
  {
    {
      std::unique_lock lock(m_QueueMutex);
      m_Stop = true;
    }
    m_Condition.notify_all();
    for (auto& worker : m_Workers) {
      if (worker.joinable()) { worker.join(); }
    }
  }

  void submit(std::function<void()> task)
  {
    {
      std::unique_lock lock(m_QueueMutex);
      if (m_Stop) { throw std::runtime_error("Executor has been stopped"); }
      m_Tasks.emplace(std::move(task));
    }
    m_Condition.notify_one();
  }

  void wait()
  {
    std::unique_lock lock(m_QueueMutex);
    m_Completed.wait(lock,
                     [this] { return m_Tasks.empty() && m_ActiveTasks == 0; });
  }

private:

  void worker_loop()
  {
    while (true) {
      std::function<void()> task;
      {
        std::unique_lock lock(m_QueueMutex);
        m_Condition.wait(lock, [this] { return m_Stop || !m_Tasks.empty(); });

        if (m_Stop && m_Tasks.empty()) { return; }

        task = std::move(m_Tasks.front());
        m_Tasks.pop();
        ++m_ActiveTasks;
      }

      task();

      {
        std::unique_lock lock(m_QueueMutex);
        --m_ActiveTasks;
        if (m_Tasks.empty() && m_ActiveTasks == 0) { m_Completed.notify_all(); }
      }
    }
  }

  std::vector<std::thread>          m_Workers;
  std::queue<std::function<void()>> m_Tasks;
  std::mutex                        m_QueueMutex;
  std::condition_variable           m_Condition;
  std::condition_variable           m_Completed;
  std::atomic<bool>                 m_Stop;
  std::atomic<int>                  m_ActiveTasks { 0 };
};

// DAG scheduler
class DagScheduler
{
public:

  explicit DagScheduler(std::unique_ptr<ThreadPoolExecutor> executor = nullptr)
      : m_Executor(executor ? std::move(executor)
                            : std::make_unique<ThreadPoolExecutor>())
  {
  }

  // Add task to the scheduler
  template<detail::Task FunctionType>
  void add_task(std::string task_id, FunctionType&& function,
                std::vector<std::string> dependencies = {})
  {
    auto task = std::make_shared<TaskImplementation<FunctionType>>(
        std::move(task_id), std::forward<FunctionType>(function),
        std::move(dependencies));

    std::lock_guard lock(m_Mutex);
    if (!m_Tasks.try_emplace(task->get_id(), task).second) {
      throw std::runtime_error(
          std::format("Task with id '{}' already exists", task->get_id()));
    }
  }

  // Add dependency relationship between tasks
  void add_dependency(const std::string& task_id,
                      const std::string& dependency_id)
  {
    std::lock_guard lock(m_Mutex);

    auto task_iterator       = m_Tasks.find(task_id);
    auto dependency_iterator = m_Tasks.find(dependency_id);

    if (task_iterator == m_Tasks.end() ||
        dependency_iterator == m_Tasks.end()) {
      throw std::runtime_error("Task or dependency not found");
    }

    task_iterator->second->add_dependency(dependency_id);
  }

  // Execute the DAG scheduling
  void execute()
  {
    std::lock_guard lock(m_Mutex);

    validate_dag();
    build_execution_graph();

    std::vector<std::string> ready_tasks = get_ready_tasks();

    while (!ready_tasks.empty() || !m_RunningTasks.empty()) {
      // Submit ready tasks
      for (const auto& task_id : ready_tasks) {
        auto task = m_Tasks.at(task_id);
        task->set_state(TaskState::Ready);

        auto future = std::async(std::launch::async,
                                 [this, task] { return execute_task(task); });

        m_RunningTasks.emplace(task_id, std::move(future));
      }

      // Wait for any task to complete
      if (!m_RunningTasks.empty()) {
        wait_for_completion();
        ready_tasks = get_ready_tasks();
      }
    }
  }

  // Get task state by task ID
  TaskState get_task_state(const std::string& task_id) const
  {
    std::lock_guard lock(m_Mutex);
    if (auto iterator = m_Tasks.find(task_id); iterator != m_Tasks.end()) {
      return iterator->second->get_state();
    }
    throw std::runtime_error("Task not found");
  }

  // Reset scheduler state
  void reset()
  {
    std::lock_guard lock(m_Mutex);
    for (auto& [id, task] : m_Tasks) {
      task->set_state(TaskState::Pending);
    }
    m_RunningTasks.clear();
    m_CompletedTasks.clear();
  }

private:

  void validate_dag()
  {
    std::unordered_set<std::string> visited_nodes, recursion_stack;

    for (const auto& [id, task] : m_Tasks) {
      if (!is_visited(visited_nodes, id)) {
        if (has_cycle(id, visited_nodes, recursion_stack)) {
          throw std::runtime_error("DAG contains cycles");
        }
      }
    }
  }

  bool has_cycle(const std::string&               task_id,
                 std::unordered_set<std::string>& visited_nodes,
                 std::unordered_set<std::string>& recursion_stack)
  {
    if (!recursion_stack.insert(task_id).second) { return true; }

    auto task = m_Tasks.at(task_id);
    for (const auto& dependency_id : task->get_dependencies()) {
      if (!is_visited(visited_nodes, dependency_id)) {
        if (has_cycle(dependency_id, visited_nodes, recursion_stack)) {
          return true;
        }
      } else if (recursion_stack.contains(dependency_id)) {
        return true;
      }
    }

    recursion_stack.erase(task_id);
    visited_nodes.insert(task_id);
    return false;
  }

  bool is_visited(const std::unordered_set<std::string>& visited_nodes,
                  const std::string&                     node_id)
  {
    return visited_nodes.find(node_id) != visited_nodes.end();
  }

  void build_execution_graph()
  {
    m_InDegree.clear();
    m_Dependents.clear();

    for (const auto& [id, task] : m_Tasks) {
      m_InDegree [id] = task->get_dependencies().size();
      for (const auto& dependency_id : task->get_dependencies()) {
        m_Dependents [dependency_id].push_back(id);
      }
    }
  }

  std::vector<std::string> get_ready_tasks()
  {
    std::vector<std::string> ready_tasks;

    for (const auto& [id, task] : m_Tasks) {
      if (task->get_state() == TaskState::Pending && m_InDegree.at(id) == 0) {
        ready_tasks.push_back(id);
      }
    }

    return ready_tasks;
  }

  void execute_task(std::shared_ptr<TaskInterface> task)
  {
    try {
      task->execute();

      std::lock_guard lock(m_Mutex);
      m_CompletedTasks.insert(task->get_id());
      m_RunningTasks.erase(task->get_id());

      // Update dependent task in-degrees
      if (auto iterator = m_Dependents.find(task->get_id());
          iterator != m_Dependents.end()) {
        for (const auto& dependent_id : iterator->second) {
          --m_InDegree [dependent_id];
        }
      }

    } catch (const std::exception& exception) {
      std::lock_guard lock(m_Mutex);
      std::cerr << std::format("Task '{}' failed: {}", task->get_id(),
                               exception.what())
                << std::endl;
      m_RunningTasks.erase(task->get_id());
    }
  }

  void wait_for_completion()
  {
    for (auto iterator = m_RunningTasks.begin();
         iterator != m_RunningTasks.end();) {
      auto& [id, future] = *iterator;

      if (future.wait_for(std::chrono::milliseconds(10)) ==
          std::future_status::ready) {
        try {
          future.get();
        } catch (...) {
          // Exception already handled in execute_task
        }
        iterator = m_RunningTasks.erase(iterator);
      } else {
        ++iterator;
      }
    }
  }

  std::unique_ptr<ThreadPoolExecutor>                             m_Executor;
  std::unordered_map<std::string, std::shared_ptr<TaskInterface>> m_Tasks;
  std::unordered_map<std::string, int>                            m_InDegree;
  std::unordered_map<std::string, std::vector<std::string>>       m_Dependents;
  std::unordered_map<std::string, std::future<void>> m_RunningTasks;
  std::unordered_set<std::string>                    m_CompletedTasks;
  mutable std::mutex                                 m_Mutex;
};
}  // namespace stdex

/*
// Usage Example:

#include <iostream>
#include <thread>
#include <chrono>

int main() {
    using namespace stdex;
    DagScheduler scheduler;

    // Add tasks
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
    }, {"task1", "task2"});  // task4 depends on task1 and task2

    scheduler.add_task("task5", [] {
        std::cout << "Task 5 executing..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(75));
    }, {"task3", "task4"});  // task5 depends on task3 and task4

    try {
        std::cout << "Starting DAG execution..." << std::endl;
        scheduler.execute();
        std::cout << "All tasks completed successfully!" << std::endl;
    } catch (const std::exception& exception) {
        std::cerr << "Scheduler error: " << exception.what() << std::endl;
    }

    return 0;
}
*/
