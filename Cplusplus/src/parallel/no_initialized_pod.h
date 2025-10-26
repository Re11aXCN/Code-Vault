#pragma once

template <typename _Ty>
struct no_initialized_pod {
private:
    _Ty _M_t;
public:
    no_initialized_pod() {}; // 不是 = default，也不写 : _M_t()，这样一来只要 T 是 POD 类型，value 就不会0初始化
    no_initialized_pod(const no_initialized_pod& p) : _M_t(p._M_t) {}
    no_initialized_pod(no_initialized_pod&& p) noexcept : _M_t(std::move(p._M_t)) {}
    no_initialized_pod& operator=(const no_initialized_pod& p) { _M_t = p._M_t; return *this; }
    no_initialized_pod& operator=(no_initialized_pod&& p) noexcept { _M_t = std::move(p._M_t); return *this; }

    no_initialized_pod(const _Ty& t) : _M_t(t) {}
    no_initialized_pod(_Ty&& t) noexcept : _M_t(std::move(t)) {}
    no_initialized_pod& operator=(const _Ty& t) { _M_t = t; return *this; }
    no_initialized_pod& operator=(_Ty&& t) noexcept { _M_t = std::move(t); return *this; }

    operator _Ty& () { return _M_t; }
    operator const _Ty& () const { return _M_t; }

    _Ty& get() { return _M_t; }
    const _Ty& get() const { return _M_t; }

    void destory() { _M_t.~_Ty(); }

    template <typename... _Ts>
    no_initialized_pod& emplace(_Ts&&... ts) {
        ::new (&_M_t) _Ty(std::forward<_Ts>(ts)...);
        return *this;
    }
};