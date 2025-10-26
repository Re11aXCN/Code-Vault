#pragma once

#include <exception>
#include <initializer_list>
#include <type_traits>

struct BadOptionalAccess : std::exception {
	BadOptionalAccess() = default;
	virtual ~BadOptionalAccess() = default;

	const char* what() const noexcept override {
		return "BadOptionalAccess";
	}
};

struct Nullopt {
	explicit Nullopt() = default;
};

inline constexpr Nullopt nullopt;

struct InPlace {
	explicit InPlace() = default;
};

inline constexpr InPlace inPlace;

template <class T>
struct Optional {
private:
	bool m_has_value;
	union {
		T m_value;
	};

public:
	Optional(T&& value) noexcept : m_has_value(true), m_value(std::move(value)) {}

	Optional(T const& value) noexcept : m_has_value(true), m_value(std::move(value)) {}

	Optional() noexcept : m_has_value(false) {}

	Optional(Nullopt) noexcept : m_has_value(false) {}

	template <class ...Ts>
	explicit Optional(InPlace, Ts &&...value_args) : m_has_value(true), m_value(std::forward<Ts>(value_args)...) {}

	template <class U, class ...Ts>
	explicit Optional(InPlace, std::initializer_list<U> ilist, Ts &&...value_args) : m_has_value(true),
		m_value(ilist, std::forward<Ts>(value_args)...) {}

	Optional(Optional const& that) : m_has_value(that.m_has_value) {
		if (m_has_value) {
			new (&m_value) T(that.m_value); // placement-new（不分配内存，只是构造）
			// m_value = T(that.m_value); // m_value.operator=(T const &);
		}
	}

	Optional(Optional&& that) noexcept : m_has_value(that.m_has_value) {
		if (m_has_value) {
			new (&m_value) T(std::move(that.m_value));
		}
	}

	Optional& operator=(Nullopt) noexcept {
		if (m_has_value) {
			m_value.~T();
			m_has_value = false;
		}
		return *this;
	}

	Optional& operator=(T&& value) noexcept {
		if (m_has_value) {
			m_value.~T();
			m_has_value = false;
		}
		new (&m_value) T(std::move(value));
		m_has_value = true;
		return *this;
	}

	Optional& operator=(T const& value) noexcept {
		if (m_has_value) {
			m_value.~T();
			m_has_value = false;
		}
		new (&m_value) T(value);
		m_has_value = true;
		return *this;
	}

	Optional& operator=(Optional const& that) {
		if (this == &that) return *this;

		if (m_has_value) {
			m_value.~T();
			m_has_value = false;
		}
		if (that.m_has_value) {
			new (&m_value) T(that.m_value);
		}
		m_has_value = that.m_has_value;
		return *this;
	}

	Optional& operator=(Optional&& that) noexcept {
		if (this == &that) return *this;

		if (m_has_value) {
			m_value.~T();
			m_has_value = false;
		}
		if (that.m_has_value) {
			new (&m_value) T(std::move(that.m_value));
			that.m_value.~T();
		}
		m_has_value = that.m_has_value;
		that.m_has_value = false;
		return *this;
	}

	template <class ...Ts>
	void emplace(Ts &&...value_args) {
		if (m_has_value) {
			m_value.~T();
			m_has_value = false;
		}
		new (&m_value) T(std::forward<Ts>(value_args)...);
		m_has_value = true;
	}

	template <class U, class ...Ts>
	void emplace(std::initializer_list<U> ilist, Ts &&...value_args) {
		if (m_has_value) {
			m_value.~T();
			m_has_value = false;
		}
		new (&m_value) T(ilist, std::forward<Ts>(value_args)...);
		m_has_value = true;
	}

	void reset() noexcept { // 等价于 *this = nullopt;
		if (m_has_value) {
			m_value.~T();
			m_has_value = false;
		}
	}

	~Optional() noexcept {
		if (m_has_value) {
			m_value.~T(); // placement-delete（不释放内存，只是析构）
		}
	}

	bool has_value() const noexcept {
		return m_has_value;
	}

	explicit operator bool() const noexcept {
		return m_has_value;
	}

	bool operator==(Nullopt) const noexcept {
		return !m_has_value;
	}

	friend bool operator==(Nullopt, Optional const& self) noexcept {
		return !self.m_has_value;
	}

	bool operator!=(Nullopt) const noexcept {
		return m_has_value;
	}

	friend bool operator!=(Nullopt, Optional const& self) noexcept {
		return self.m_has_value;
	}

	T const& value() const& {
		if (!m_has_value)
			throw BadOptionalAccess();
		return m_value;
	}

	T& value()& {
		if (!m_has_value)
			throw BadOptionalAccess();
		return m_value;
	}

	T const&& value() const&& {
		if (!m_has_value)
			throw BadOptionalAccess();
		return std::move(m_value);
	}

	T&& value()&& {
		if (!m_has_value)
			throw BadOptionalAccess();
		return std::move(m_value);
	}

	T const& operator*() const& noexcept {
		return m_value;
	}

	T& operator*() & noexcept {
		return m_value;
	}

	T const&& operator*() const&& noexcept {
		return std::move(m_value);
	}

	T&& operator*() && noexcept {
		return std::move(m_value);
	}

	T const* operator->() const noexcept {
		return &m_value;
	}

	T* operator->() noexcept {
		return &m_value;
	}

	T value_or(T default_value) const& {
		if (!m_has_value)
			return default_value;
		return m_value;
	}

	T value_or(T default_value) && noexcept {
		if (!m_has_value)
			return default_value;
		return std::move(m_value);
	}

	bool operator==(Optional<T> const& that) const noexcept {
		if (m_has_value != that.m_has_value)
			return false;
		if (m_has_value) {
			return m_value == that.m_value;
		}
		return true;
	}

	bool operator!=(Optional const& that) const noexcept {
		if (m_has_value != that.m_has_value)
			return true;
		if (m_has_value) {
			return m_value != that.m_value;
		}
		return false;
	}

	bool operator>(Optional const& that) const noexcept {
		if (!m_has_value || !that.m_has_value)
			return false;
		return m_value > that.m_value;
	}

	bool operator<(Optional const& that) const noexcept {
		if (!m_has_value || !that.m_has_value)
			return false;
		return m_value < that.m_value;
	}

	bool operator>=(Optional const& that) const noexcept {
		if (!m_has_value || !that.m_has_value)
			return true;
		return m_value >= that.m_value;
	}

	bool operator<=(Optional const& that) const noexcept {
		if (!m_has_value || !that.m_has_value)
			return true;
		return m_value <= that.m_value;
	}

	template <class F>
	auto and_then(F&& f) const& {
		using RetType = std::remove_cv_t<std::remove_reference_t<decltype(f(m_value))>>;
		if (m_has_value) {
			return std::forward<F>(f)(m_value);
		}
		else {
			return RetType{};
		}
	}

	template <class F>
	auto and_then(F&& f)& {
		using RetType = std::remove_cv_t<std::remove_reference_t<decltype(f(m_value))>>;
		if (m_has_value) {
			return std::forward<F>(f)(m_value);
		}
		else {
			return RetType{};
		}
	}

	// 为什么设置C++ ISO 标准为 latest却还是 C++03标准的 __cplusplus = 199711L
	// 解决
	// https://yapingxin.blog.csdn.net/article/details/139694408?spm=1001.2101.3001.6650.3&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7EYuanLiJiHua%7EPosition-3-139694408-blog-50915378.235%5Ev43%5Econtrol&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7EYuanLiJiHua%7EPosition-3-139694408-blog-50915378.235%5Ev43%5Econtrol&utm_relevant_index=6
	// https://blog.csdn.net/qq_45254369/article/details/129824327
#if __cplusplus > 201103L // C++14 才有 _t 和 _v
	template <class F>
	auto and_then(F&& f) const&& {
		using RetType = std::remove_cv_t<std::remove_reference_t<decltype(f(std::move(m_value)))>>;
		if (m_has_value) {
			return std::forward<F>(f)(std::move(m_value));
		}
		else {
			return RetType{};
		}
	}

	template <class F>
	auto and_then(F&& f)&& {
		using RetType = std::remove_cv_t<std::remove_reference_t<decltype(f(std::move(m_value)))>>;
		if (m_has_value) {
			return std::forward<F>(f)(std::move(m_value));
		}
		else {
			return RetType{};
		}
	}

	template <class F>
	auto transform(F&& f) const& -> Optional<std::remove_cv_t<std::remove_reference_t<decltype(f(m_value))>>> {
		if (m_has_value) {
			return std::forward<F>(f)(m_value);
		}
		else {
			return nullopt;
		}
	}

	template <class F>
	auto transform(F&& f) & -> Optional<std::remove_cv_t<std::remove_reference_t<decltype(f(m_value))>>> {
		if (m_has_value) {
			return std::forward<F>(f)(m_value);
		}
		else {
			return nullopt;
		}
	}

	template <class F>
	auto transform(F&& f) const&& -> Optional<std::remove_cv_t<std::remove_reference_t<decltype(f(std::move(m_value)))>>> {
		if (m_has_value) {
			return std::forward<F>(f)(std::move(m_value));
		}
		else {
			return nullopt;
		}
	}

	template <class F>
	auto transform(F&& f) && -> Optional<std::remove_cv_t<std::remove_reference_t<decltype(f(std::move(m_value)))>>> {
		if (m_has_value) {
			return std::forward<F>(f)(std::move(m_value));
		}
		else {
			return nullopt;
		}
	}

	template <class F, std::enable_if_t<std::is_copy_constructible_v<T>, int> = 0>
	Optional or_else(F&& f) const& {
		if (m_has_value) {
			return *this;
		}
		else {
			return std::forward<F>(f)();
		}
	}

	template <class F, std::enable_if_t<std::is_move_constructible_v<T>, int> = 0>
	Optional or_else(F&& f)&& {
		if (m_has_value) {
			return std::move(*this);
		}
		else {
			return std::forward<F>(f)();
		}
	}
#endif

	void swap(Optional& that) noexcept {
		if (m_has_value && that.m_has_value) {
			using std::swap; // ADL
			swap(m_value, that.m_value);
		}
		else if (!m_has_value && !that.m_has_value) {
			// do nothing
		}
		else if (m_has_value) {
			that.emplace(std::move(m_value));
			reset();
		}
		else {
			emplace(std::move(that.m_value));
			that.reset();
		}
	}
};

#if __cpp_deduction_guides
template <class T> // CTAD
Optional(T) -> Optional<T>;
#endif

template <class T>
Optional<T> makeOptional(T value) {
	return Optional<T>(std::move(value));
}
