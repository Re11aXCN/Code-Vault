#pragma once

template <typename _Ty>
struct NoInitializedPod {
private:
    _Ty _M_t;
public:
    NoInitializedPod() {}; // 不是 = default，也不写 : _M_t()，这样一来只要 T 是 POD 类型，value 就不会0初始化
    NoInitializedPod(const NoInitializedPod& p) : _M_t(p._M_t) {}
    NoInitializedPod(NoInitializedPod&& p) noexcept : _M_t(std::move(p._M_t)) {}
    NoInitializedPod& operator=(const NoInitializedPod& p) { _M_t = p._M_t; return *this; }
    NoInitializedPod& operator=(NoInitializedPod&& p) noexcept { _M_t = std::move(p._M_t); return *this; }

    NoInitializedPod(const _Ty& t) : _M_t(t) {}
    NoInitializedPod(_Ty&& t) noexcept : _M_t(std::move(t)) {}
    NoInitializedPod& operator=(const _Ty& t) { _M_t = t; return *this; }
    NoInitializedPod& operator=(_Ty&& t) noexcept { _M_t = std::move(t); return *this; }

    operator _Ty& () { return _M_t; }
    operator const _Ty& () const { return _M_t; }

    _Ty& get() { return _M_t; }
    const _Ty& get() const { return _M_t; }

    void destory() { _M_t.~_Ty(); }

    template <typename... _Ts>
    NoInitializedPod& emplace(_Ts&&... ts) {
        ::new (&_M_t) _Ty(std::forward<_Ts>(ts)...);
        return *this;
    }
};