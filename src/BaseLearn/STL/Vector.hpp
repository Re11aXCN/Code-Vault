#ifndef _VECTOR_HPP_
#define _VECTOR_HPP_
#include <cstddef>
#include <format>
#include <iostream>
#include <string>
#include <vector>
namespace istl {

	template<typename T>
	class vector {
	public:
		using value_type = T;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using reference = value_type&;
		using const_reference = const value_type&;
		using iterator = value_type*;
		using const_iterator = const value_type*;

		// 容量相关方法
		size_type size() const noexcept { return _finish - _start; }
		size_type capacity() const noexcept { return _end_of_storage - _start; }
		bool empty() const noexcept { return _start == _finish; }

		// 迭代器方法
		iterator begin() noexcept { return _start; }
		const_iterator begin() const noexcept { return _start; }
		const_iterator cbegin() const noexcept { return _start; }
		iterator end() noexcept { return _finish; }
		const_iterator end() const noexcept { return _finish; }
		const_iterator cend() const noexcept { return _finish; }

		// 元素访问方法
		reference operator[](size_type n) { return *(_start + n); }
		const_reference operator[](size_type n) const { return *(_start + n); }
		reference at(size_type n) {
			if (n >= size()) {
				throw std::out_of_range("vector::at");
			}
			return (*this)[n];
		}
		const_reference at(size_type n) const {
			if (n >= size()) {
				throw std::out_of_range("vector::at");
			}
			return (*this)[n];
		}
		reference front() noexcept { return *_start; }
		const_reference front() const noexcept { return *_start; }
		reference back() noexcept { return *(_finish - 1); }
		const_reference back() const noexcept { return *(_finish - 1); }
	public:
		void clear() {
			/*if (_start) {
				for (auto p = _start; p != _finish; ++p) {
					p->~T();
				}
				_finish = _start;
			}*/
			std::destroy(_start, _finish);
			_finish = _start;
		}

		void push_back(const T& value) {
			if (_finish == _end_of_storage) {
				reserve(capacity() == 0 ? 1 : capacity() * 2);
			}
			new (static_cast<void*>(_finish)) T(value);
			++_finish;
		}

		void push_back(T&& value) {
			if (_finish == _end_of_storage) {
				reserve(capacity() == 0 ? 1 : capacity() * 2);
			}
			new (static_cast<void*>(_finish)) T(std::move(value));
			++_finish;
		}

		void pop_back() noexcept {
			if (_finish > _start) {
				--_finish;
				_finish->~T();
			}
		}

		template<typename... Args>
		void emplace_back(Args&&... args) {
			if (_finish == _end_of_storage) {
				reserve(capacity() == 0 ? 1 : capacity() * 2);
			}
			new (static_cast<void*>(_finish)) T(std::forward<Args>(args)...);
			++_finish;
		}

		void reserve(size_type n) {
			if (n <= capacity()) return;

			const size_type old_size = size();
			T* new_start = allocate(n);
			if (_start) {
				std::uninitialized_move(_start, _finish, new_start);
				std::destroy(_start, _finish);
				deallocate(_start);
			}
			_start = new_start;
			_finish = _start + old_size;
			_end_of_storage = _start + n;
		}

		void resize(size_type n) {
			if (n > capacity()) {
				reserve(n);
			}
			if (n > size()) {
				std::uninitialized_default_construct(_finish, _start + n);
			}
			else {
				for (auto p = _finish; p != _start + n; ++p) {
					p->~T();
				}
				_finish = _start + n;
			}
		}

		void resize(size_type n, const value_type& value) {
			if (n > capacity()) {
				reserve(n);
			}
			if (n > size()) {
				std::uninitialized_fill_n(_finish, n - size(), value);
			}
			else {
				for (auto p = _finish; p != _start + n; ++p) {
					p->~T();
				}
				_finish = _start + n;
			}
		}

		void swap(vector& other) noexcept {
			std::swap(_start, other._start);
			std::swap(_finish, other._finish);
			std::swap(_end_of_storage, other._end_of_storage);
		}

		// 实现insert方法
		iterator insert(const_iterator pos, const T& value) {
			// 计算插入位置的索引
			difference_type index = pos - _start;

			// 确保索引有效
			if (index < 0 || index > size()) {
				throw std::out_of_range("vector::insert");
			}

			// 如果容量不足，需要重新分配内存
			if (_finish == _end_of_storage) {
				size_type new_capacity = capacity() == 0 ? 1 : capacity() * 2;
				reserve(new_capacity);
				// 重新计算pos，因为reserve可能会改变_start
				pos = _start + index;
			}

			// 在pos位置插入元素
			iterator insert_pos = const_cast<iterator>(pos);

			// 如果插入位置不是末尾，需要移动元素
			if (insert_pos != _finish) {
				// 在末尾创建一个新元素的空间
				new (static_cast<void*>(_finish)) T(std::move(*(_finish - 1)));

				// 将插入位置之后的元素向后移动一个位置
				std::move_backward(insert_pos, _finish - 1, _finish);

				// 在插入位置构造新元素
				*insert_pos = value;
			}
			else {
				// 如果插入位置是末尾，直接在末尾构造新元素
				new (static_cast<void*>(_finish)) T(value);
			}

			// 更新finish指针
			++_finish;

			// 返回指向插入元素的迭代器
			return insert_pos;
		}

		// 在指定位置插入右值
		iterator insert(const_iterator pos, T&& value) {
			difference_type index = pos - _start;

			if (index < 0 || index > size()) {
				throw std::out_of_range("vector::insert");
			}

			if (_finish == _end_of_storage) {
				size_type new_capacity = capacity() == 0 ? 1 : capacity() * 2;
				reserve(new_capacity);
				pos = _start + index;
			}

			iterator insert_pos = const_cast<iterator>(pos);

			if (insert_pos != _finish) {
				new (static_cast<void*>(_finish)) T(std::move(*(_finish - 1)));
				std::move_backward(insert_pos, _finish - 1, _finish);
				*insert_pos = std::move(value);
			}
			else {
				new (static_cast<void*>(_finish)) T(std::move(value));
			}

			++_finish;
			return insert_pos;
		}

		// 在指定位置插入n个相同的元素
		iterator insert(const_iterator pos, size_type n, const T& value) {
			if (n == 0) return const_cast<iterator>(pos);

			difference_type index = pos - _start;

			if (index < 0 || index > size()) {
				throw std::out_of_range("vector::insert");
			}

			// 确保有足够的容量
			if (size() + n > capacity()) {
				size_type new_capacity = std::max(capacity() * 2, size() + n);
				reserve(new_capacity);
				pos = _start + index;
			}

			iterator insert_pos = const_cast<iterator>(pos);

			// 如果插入位置不是末尾
			if (insert_pos != _finish) {
				// 将插入位置之后的元素向后移动n个位置
				size_type elems_after = _finish - insert_pos;

				// 先移动末尾的元素
				for (size_type i = 0; i < std::min(elems_after, n); ++i) {
					new (static_cast<void*>(_finish + i)) T(std::move(*(_finish - 1 - i)));
				}

				// 如果要移动的元素数量大于要插入的元素数量
				if (elems_after > n) {
					std::move_backward(insert_pos, _finish - n, _finish);
					// 在插入位置填充n个元素
					std::fill_n(insert_pos, n, value);
				}
				else {
					// 在插入位置填充前elems_after个元素
					std::fill_n(insert_pos, elems_after, value);
					// 在末尾构造剩余的元素
					std::uninitialized_fill_n(_finish, n - elems_after, value);
				}
			}
			else {
				// 如果插入位置是末尾，直接在末尾构造n个新元素
				std::uninitialized_fill_n(_finish, n, value);
			}

			_finish += n;
			return insert_pos;
		}

		// 在指定位置插入范围[first, last)的元素
		template<typename InputIt>
		iterator insert(const_iterator pos, InputIt first, InputIt last) {
			difference_type index = pos - _start;

			if (index < 0 || index > size()) {
				throw std::out_of_range("vector::insert");
			}

			difference_type n = std::distance(first, last);
			if (n <= 0) return const_cast<iterator>(pos);

			// 确保有足够的容量
			if (size() + n > capacity()) {
				size_type new_capacity = std::max(capacity() * 2, size() + n);
				reserve(new_capacity);
				pos = _start + index;
			}

			iterator insert_pos = const_cast<iterator>(pos);

			// 如果插入位置不是末尾
			if (insert_pos != _finish) {
				// 将插入位置之后的元素向后移动n个位置
				size_type elems_after = _finish - insert_pos;

				// 先移动末尾的元素
				for (size_type i = 0; i < std::min(elems_after, static_cast<size_type>(n)); ++i) {
					new (static_cast<void*>(_finish + i)) T(std::move(*(_finish - 1 - i)));
				}

				// 如果要移动的元素数量大于要插入的元素数量
				if (elems_after > static_cast<size_type>(n)) {
					std::move_backward(insert_pos, _finish - n, _finish);
					// 在插入位置复制范围[first, last)
					std::copy(first, last, insert_pos);
				}
				else {
					// 计算需要直接复制和需要构造的元素数量
					InputIt mid = first;
					std::advance(mid, elems_after);
					// 在插入位置复制前elems_after个元素
					std::copy(first, mid, insert_pos);
					// 在末尾构造剩余的元素
					std::uninitialized_copy(mid, last, _finish);
				}
			}
			else {
				// 如果插入位置是末尾，直接在末尾构造范围[first, last)的元素
				std::uninitialized_copy(first, last, _finish);
			}

			_finish += n;
			return insert_pos;
		}

		// 实现erase方法 - 删除指定位置的元素
		iterator erase(const_iterator pos) {
			if (empty()) return end();

			difference_type index = pos - _start;

			if (index < 0 || index >= size()) {
				throw std::out_of_range("vector::erase");
			}

			iterator erase_pos = const_cast<iterator>(pos);

			// 如果不是最后一个元素，需要移动后面的元素
			if (erase_pos + 1 != _finish) {
				std::move(erase_pos + 1, _finish, erase_pos);
			}

			// 销毁最后一个元素
			--_finish;
			_finish->~T();

			return erase_pos;
		}

		// 删除范围[first, last)的元素
		iterator erase(const_iterator first, const_iterator last) {
			if (empty() || first == last) return const_cast<iterator>(last);

			difference_type start_index = first - _start;
			difference_type end_index = last - _start;

			if (start_index < 0 || start_index >= size() ||
				end_index < 0 || end_index > size() ||
				start_index > end_index) {
				throw std::out_of_range("vector::erase");
			}

			iterator erase_start = const_cast<iterator>(first);
			iterator erase_end = const_cast<iterator>(last);

			// 计算要删除的元素数量
			difference_type n = erase_end - erase_start;

			// 如果不是删除末尾的元素，需要移动后面的元素
			if (erase_end != _finish) {
				std::move(erase_end, _finish, erase_start);
			}

			// 销毁末尾的n个元素
			for (iterator it = _finish - n; it != _finish; ++it) {
				it->~T();
			}

			_finish -= n;
			return erase_start;
		}
	public:
		// 构造函数和析构函数
		vector() noexcept : _start(nullptr), _finish(nullptr), _end_of_storage(nullptr) {}

		vector(size_type n, const value_type& value = value_type())
			: _start(allocate(n)), _finish(_start + n), _end_of_storage(_start + n)
		{
			std::uninitialized_fill_n(_start, n, value);
		}

		template<typename InputIt>
		vector(InputIt first, InputIt last)
			: _start(allocate(std::distance(first, last))),
			_finish(_start + std::distance(first, last)),
			_end_of_storage(_start + std::distance(first, last))
		{
			std::uninitialized_copy(first, last, _start);
		}

		vector(const vector& other)
			: _start(allocate(other.size())),
			_finish(_start + other.size()),
			_end_of_storage(_start + other.size())
		{
			std::uninitialized_copy(other.begin(), other.end(), _start);
		}

		vector(vector&& other) noexcept
			: _start(other._start), _finish(other._finish), _end_of_storage(other._end_of_storage)
		{
			other._start = nullptr;
			other._finish = nullptr;
			other._end_of_storage = nullptr;
		}

		vector& operator=(const vector& other)
		{
			if (this != &other) {
				clear();
				if (capacity() < other.size()) {
					deallocate(_start);
					_start = allocate(other.size());
					_end_of_storage = _start + other.size();
				}
				_finish = std::uninitialized_copy(other.begin(), other.end(), _start);
			}
			return *this;
		}

		vector& operator=(vector&& other) noexcept {
			if (this != &other) {
				deallocate(_start);
				_start = other._start;
				_finish = other._finish;
				_end_of_storage = other._end_of_storage;
				other._start = nullptr;
				other._finish = nullptr;
				other._end_of_storage = nullptr;
			}
			return *this;
		}

		~vector() noexcept {
			clear();
			deallocate(_start);
			_start = _finish = _end_of_storage = nullptr;
		}
	private:
		T* allocate(size_type n) {
			return static_cast<T*>(::operator new(n * sizeof(value_type)));
		}

		void deallocate(T* p) noexcept {
			if (p) {
				::operator delete(p);
			}
		}

		T* _start;
		T* _finish;
		T* _end_of_storage;
	};
	namespace utils {

		// 打印vector的宏，使用std::format格式化输出
#define PRINT_VECTOR(vec) do { \
    std::cout << std::format("Vector内容: "); \
    for (const auto& item : vec) { \
        std::cout << std::format("{} ", item); \
    } \
    std::cout << std::format("\nSize: {}, Capacity: {}\n", vec.size(), vec.capacity()); \
} while(0)

// 简化版本的打印宏
#define PRINT_VEC(vec) PRINT_VECTOR(vec)

// 单例测试宏
#define TEST_VECTOR_SINGLETON(T) do { \
    std::cout << std::format("\n===== 开始测试 {} 类型的vector单例 =====\n", #T); \
    static istl::vector<T>& getInstance() { \
        static istl::vector<T> instance; \
        return instance; \
    } \
    auto& vec1 = getInstance(); \
    auto& vec2 = getInstance(); \
    std::cout << std::format("单例测试: vec1地址 = {}, vec2地址 = {}\n", \
                           static_cast<void*>(&vec1), static_cast<void*>(&vec2)); \
    std::cout << std::format("地址是否相同: {}\n", &vec1 == &vec2 ? "是" : "否"); \
    std::cout << std::format("===== 结束测试 {} 类型的vector单例 =====\n", #T); \
} while(0)
	}
}
#endif // VECTOR_HPP_

int main()
{
	// 使用string类型测试vector
	using namespace istl;
	using namespace istl::utils;

	// 创建一个string类型的vector
	vector<std::string> strVec;

	// 添加一些测试数据
	strVec.push_back("Hello");
	strVec.push_back("World");
	strVec.push_back("C++");

	// 打印vector内容
	PRINT_VECTOR(strVec);

	// 测试insert方法
	strVec.insert(strVec.begin() + 1, "Inserted");
	PRINT_VECTOR(strVec);

	// 测试erase方法
	strVec.erase(strVec.begin() + 2);
	PRINT_VECTOR(strVec);

	// 测试范围insert
	std::vector<std::string> stdVec{ "One", "Two", "Three" };
	strVec.insert(strVec.end(), stdVec.begin(), stdVec.end());
	PRINT_VECTOR(strVec);

	// 测试范围erase
	strVec.erase(strVec.begin(), strVec.begin() + 2);
	PRINT_VECTOR(strVec);

	return 0;
}