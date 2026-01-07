#pragma once

#include <functional>
#include <memory>
#include <type_traits>
#include <vector>

namespace stdex {

enum class CallbackResult
{
  Keep,
  Erase,
};

// One-time binding tag
constexpr struct OneshotTag
{
  explicit OneshotTag() = default;
} oneshot;

// N-time binding tag, strongly typed enum for type safety
enum class NshotTag : size_t
{
};

namespace detail {
template<class Self>
std::shared_ptr<Self> lock_if_weak(std::weak_ptr<Self> const& self)
{
  return self.lock();
}

template<class Self>
Self const& lock_if_weak(Self const& self)
{
  return self;
}

template<class Self, class MemFn>
auto bind(Self self, MemFn memfn, OneshotTag)
{
  return [self = std::move(self), memfn](auto... args) {
    auto const& ptr = detail::lock_if_weak(self);
    if (ptr == nullptr) { return CallbackResult::Erase; }
    ((*ptr).*memfn)(args...);
    return CallbackResult::Erase;
  };
}

template<class Self, class MemFn>
auto bind(Self self, MemFn memfn, NshotTag n)
{
  return [self = std::move(self), memfn,
          n    = static_cast<size_t>(n)](auto... args) mutable {
    if (n == 0) { return CallbackResult::Erase; }
    auto const& ptr = detail::lock_if_weak(self);
    if (ptr == nullptr) { return CallbackResult::Erase; }
    ((*ptr).*memfn)(args...);
    --n;
    if (n == 0) { return CallbackResult::Erase; }
    return CallbackResult::Keep;
  };
}

template<class Self, class MemFn>
auto bind(Self self, MemFn memfn)
{
  return [self = std::move(self), memfn](auto... args) {
    auto const& ptr = detail::lock_if_weak(self);
    if (ptr == nullptr) { return CallbackResult::Erase; }
    ((*ptr).*memfn)(args...);
    return CallbackResult::Keep;
  };
}
}  // namespace detail

// Parameter pack syntax
// ...Args: ellipsis on the left means define, defining a parameter pack
// Args...: ellipsis on the right means use, using a parameter pack

template<class... Args>  // define Args
struct Signal
{
private:

  // move_only_function C++23 support
#if __cpp_lib_move_only_function  // standard feature-test macro
  using Functor = std::move_only_function<void(Args...)>;  // use Args
#else
  using Functor = std::function<CallbackResult(Args...)>;  // use Args
#endif

  std::vector<Functor> m_Callbacks;

public:

#if __cpp_if_constexpr
  template<class Func>
  void connect(Func callback)
  {
    if constexpr (std::is_invocable_r_v<CallbackResult, Func, Args...>) {
      m_Callbacks.push_back(std::move(callback));
    } else {
      m_Callbacks.push_back(
          [callback = std::move(callback)](Args... args) mutable {
        callback(std::forward<Args>(args)...);
        return CallbackResult::Keep;
      });
    }
  }
#else
  template<
      class Func,
      typename std::enable_if<std::is_convertible<decltype(std::declval<Func>()(
                                                      std::declval<Args>()...)),
                                                  CallbackResult>::value,
                              int>::type = 0>
  void connect(Func callback)
  {
    m_Callbacks.push_back(std::move(callback));
  }

  template<class Func,
           typename std::enable_if<std::is_void<decltype(std::declval<Func>()(
                                       std::declval<Args>()...))>::value,
                                   int>::type = 0>
  void connect(Func callback)
  {
    m_Callbacks.push_back(
        [callback = std::move(callback)](Args... args) mutable {
      callback(std::forward<Args>(args)...);
      return CallbackResult::Keep;
    });
  }
#endif
  // Tag parameter pack is passed to bind to determine binding count, then
  // lock_if_weak is checked
  template<class Self, class MemFn, class... Tags>  // define Tags
  void connect(Self self, MemFn memfn, Tags... tags)
  {  // use Tags, define tags
    m_Callbacks.push_back(
        detail::bind(std::move(self), memfn, tags...));  // use tags
  }

  void emit(Args... args)
  {  // use Args, define args
    for (auto it = m_Callbacks.begin(); it != m_Callbacks.end();) {
      CallbackResult result = (*it)(args...);
      switch (result) {
      case CallbackResult::Keep : ++it; break;
      case CallbackResult::Erase :
        it = m_Callbacks.erase(
            it);  // Erase if empty, enum class is more intuitive
        break;
      };
    }
  }
};
#if __cplusplus >= 202002L &&                                                  \
    !(defined(_MSC_VER) && (!defined(_MSVC_TRADITIONAL) || _MSVC_TRADITIONAL))
#  define PENGSIG_FUN(_fun, ...)                                \
    [=](auto&&... __args) {                                     \
      return _fun(__VA_ARGS__ __VA_OPT__(, )                    \
                      std::forward<decltype(_args)>(_args)...); \
    }
#else
#  define PENGSIG_FUN(_fun)                                  \
    [=](auto&&... _args) {                                   \
      return __fun(std::forward<decltype(_args)>(_args)...); \
    }
#endif

}  // namespace stdex

/*
// Usage Examples:

#include <iostream>
#include <memory>
#include <string>

class Button {
public:
    stdex::Signal<void()> clicked;

    void simulate_click() {
        clicked.emit();
    }
};

class MainWindow {
private:
    int click_count = 0;
public:
    void on_click() {
        click_count++;
        std::cout << "MainWindow: Button clicked! Count: " << click_count <<
"\n";
    }
};

class Dialog {
public:
    void on_response() {
        std::cout << "Dialog: Got response!\n";
    }
};

int main() {
    using namespace stdex;

    // Example 1: Basic signal/slot with lambda
    std::cout << "=== Example 1: Lambda ===\n";
    Signal<void(int)> value_changed;

    value_changed.connect([](int value) {
        std::cout << "Value changed to: " << value << "\n";
    });

    value_changed.emit(42);


    // Example 2: Signal with multiple arguments
    std::cout << "\n=== Example 2: Multiple arguments ===\n";
    Signal<void(const std::string&, int)> data_received;

    data_received.connect([](const std::string& name, int value) {
        std::cout << "Received: " << name << " = " << value << "\n";
    });

    data_received.emit("temperature", 25);


    // Example 3: Connecting to member function
    std::cout << "\n=== Example 3: Member function ===\n";
    Button button;
    MainWindow main_window;
    Dialog dialog;

    // Permanent connection
    button.clicked.connect([&main_window]() {
        main_window.on_click();
    });

    button.simulate_click();
    button.simulate_click();


    // Example 4: One-shot connection (auto-remove after first call)
    std::cout << "\n=== Example 4: One-shot connection ===\n";
    Signal<void()> event;

    int one_shot_count = 0;
    event.connect(oneshot, [&one_shot_count]() {
        one_shot_count++;
        std::cout << "One-shot: This only runs once! Count: " << one_shot_count
<<
"\n";
    });

    event.emit();  // Will call and remove the one-shot
    event.emit();  // One-shot won't be called again


    // Example 5: N-shot connection (call N times then remove)
    std::cout << "\n=== Example 5: N-shot connection ===\n";
    Signal<void()> timer;

    int n_shot_count = 0;
    timer.connect(NshotTag(3), [&n_shot_count]() {
        n_shot_count++;
        std::cout << "N-shot: Call " << n_shot_count << " of 3\n";
    });

    timer.emit();  // Call 1
    timer.emit();  // Call 2
    timer.emit();  // Call 3
    timer.emit();  // Won't be called (already removed)


    // Example 6: Weak pointer connection (auto-removes when object destroyed)
    std::cout << "\n=== Example 6: Weak pointer ===\n";
    Signal<void()> notification;

    {
        auto dialog_ptr = std::make_shared<Dialog>();
        notification.connect(std::weak_ptr<Dialog>(dialog_ptr),
&Dialog::on_response); notification.emit();  // Will call Dialog::on_response
        std::cout << "Dialog going out of scope...\n";
    }  // Dialog destroyed

    std::cout << "After dialog destruction:\n";
    notification.emit();  // Won't call (Dialog was destroyed)


    // Example 7: Return CallbackResult to control connection lifetime
    std::cout << "\n=== Example 7: CallbackResult control ===\n";
    Signal<void(int)> counter;

    int call_count = 0;
    counter.connect([&call_count](int value) -> CallbackResult {
        call_count++;
        std::cout << "Counter: " << value << ", call count: " << call_count <<
"\n"; return (call_count >= 3) ? CallbackResult::Erase : CallbackResult::Keep;
    });

    counter.emit(1);
    counter.emit(2);
    counter.emit(3);  // Will remove itself after this
    counter.emit(4);  // Won't be called


    return 0;
}
*/
