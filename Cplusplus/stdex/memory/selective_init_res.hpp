#pragma once

#include <algorithm>
#include <cstddef>  // offsetof
#include <new>      // placement new
#include <type_traits>
#include <utility>

namespace stdex {

// ============================================================================
// Public concepts and types (outside details namespace)
// ============================================================================

// POD type detection (is_trivially_copyable AND is_standard_layout)
template<typename T>
struct IsPod
    : std::integral_constant<
          bool, std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>>
{ };

template<typename T>
inline constexpr bool is_pod_v = IsPod<T>::value;

// Initialization policy tags
struct InitNonPodMembersTag
{ };

struct NoInitTag
{ };

// User specialization interface - users specialize this for their types
template<typename Type, typename InitPolicy>
void init_non_pod_members(unsigned char* /*storage*/) noexcept
{
  // Default empty implementation
}

// ============================================================================
// Implementation details namespace
// ============================================================================

namespace details {

// Member traits - helper metafunction to get member type and check if it's POD
template<typename ClassType, typename MemberType>
struct MemberTraits
{
  using Type                   = MemberType;
  static constexpr bool is_pod = stdex::is_pod_v<MemberType>;
};
};  // namespace details

// Member initializer - initializes non-POD members selectively
template<typename ClassType, size_t Offset, typename MemberType>
struct MemberInitializer
{
  static constexpr void init(unsigned char* storage) noexcept
  {
    if constexpr (!details::MemberTraits<ClassType, MemberType>::is_pod) {
      MemberType* member_ptr = reinterpret_cast<MemberType*>(storage + Offset);
      ::new (member_ptr) MemberType();  // Non-POD member: default construct
    }
    // POD member: empty implementation, keep garbage value
  }
};

namespace details {
// User-provided initialization policy type traits
template<typename Policy>
struct IsInitPolicy : std::false_type
{ };

template<>
struct IsInitPolicy<stdex::InitNonPodMembersTag> : std::true_type
{ };

template<>
struct IsInitPolicy<stdex::NoInitTag> : std::true_type
{ };

// Bridge to connect InitNonPodMembersImpl with init_non_pod_members
template<typename Type>
struct InitNonPodMembersImpl
{
  static void init(unsigned char* storage) noexcept
  {
    stdex::init_non_pod_members<Type, stdex::InitNonPodMembersTag>(storage);
  }
};

// Base class for InitNonPodMembersTag policy
template<typename Type>
struct SelectiveInitResBaseInitNonPod
{
  static constexpr bool needs_destroy = true;

  static void construct(unsigned char* storage) noexcept
  {
    // Call user-provided or default init_non_pod_members
    InitNonPodMembersImpl<Type>::init(storage);
  }

  static void destroy(unsigned char* storage) noexcept
  {
    reinterpret_cast<Type*>(storage)->~Type();
  }
};

// Base class for NoInitTag policy
template<typename Type>
struct SelectiveInitResBaseNoInit
{
  static constexpr bool needs_destroy = false;

  static void construct(unsigned char* /*storage*/) noexcept
  {
    // Do not initialize
  }

  static void destroy(unsigned char* /*storage*/) noexcept
  {
    // Do not destroy (object was never constructed)
  }
};

// Dispatch to appropriate base class based on policy
template<typename Type, typename InitPolicy>
struct SelectiveInitResPolicyBase;

template<typename Type>
struct SelectiveInitResPolicyBase<Type, stdex::InitNonPodMembersTag>
    : SelectiveInitResBaseInitNonPod<Type>
{ };

template<typename Type>
struct SelectiveInitResPolicyBase<Type, stdex::NoInitTag>
    : SelectiveInitResBaseNoInit<Type>
{ };

// Traits to determine if we need destroy based on policy and type
template<typename Type, typename InitPolicy>
struct SelectiveInitResTraits
{
  static constexpr bool is_pod = stdex::is_pod_v<Type>;
  static constexpr bool policy_destroys =
      SelectiveInitResPolicyBase<Type, InitPolicy>::needs_destroy;
  static constexpr bool needs_destroy = !is_pod && policy_destroys;
  static constexpr bool policy_initializes =
      std::is_same_v<InitPolicy, stdex::InitNonPodMembersTag>;
};

}  // namespace details

// ============================================================================
// SelectiveInitRes main template class
// ============================================================================
// Type: The type to wrap
// InitPolicy: Initialization policy (optional, defaults to
// InitNonPodMembersTag)
//   - InitNonPodMembersTag: Initialize only non-POD members
//   - NoInitTag: Do not initialize any members

template<typename Type, typename InitPolicy = stdex::InitNonPodMembersTag>
class SelectiveInitRes
    : private details::SelectiveInitResPolicyBase<Type, InitPolicy>
{
private:

  using PolicyBase = details::SelectiveInitResPolicyBase<Type, InitPolicy>;
  using Traits     = details::SelectiveInitResTraits<Type, InitPolicy>;

  alignas(Type) unsigned char m_storage [sizeof(Type)];

  constexpr Type* get_ptr() noexcept
  {
    return reinterpret_cast<Type*>(m_storage);
  }

  constexpr const Type* get_ptr() const noexcept
  {
    return reinterpret_cast<const Type*>(m_storage);
  }

  // POD detection: if Type is POD, no initialization policy is needed
  static_assert(Traits::is_pod || details::IsInitPolicy<InitPolicy>::value,
                "For non-POD types, an initialization policy must be provided");

public:

  // Default constructor: initialize according to policy
  SelectiveInitRes() noexcept { PolicyBase::construct(m_storage); }

  // Copy constructor
  SelectiveInitRes(const SelectiveInitRes& other) noexcept
  {
    if constexpr (Traits::is_pod) {
      *get_ptr() = *other.get_ptr();
    } else {
      ::new (get_ptr()) Type(*other.get_ptr());
    }
  }

  // Move constructor
  SelectiveInitRes(SelectiveInitRes&& other) noexcept
  {
    if constexpr (Traits::is_pod) {
      *get_ptr() = *other.get_ptr();
    } else {
      ::new (get_ptr()) Type(std::move(*other.get_ptr()));
    }
  }

  // Constructor from const Type& (copy from original type)
  SelectiveInitRes(const Type& other) noexcept
  {
    if constexpr (Traits::is_pod) {
      *get_ptr() = other;
    } else {
      ::new (get_ptr()) Type(other);
    }
  }

  // Constructor from Type&& (move from original type)
  SelectiveInitRes(Type&& other) noexcept
  {
    if constexpr (Traits::is_pod) {
      *get_ptr() = other;
    } else {
      ::new (get_ptr()) Type(std::move(other));
    }
  }

  // Copy assignment operator
  SelectiveInitRes& operator= (const SelectiveInitRes& other) noexcept
  {
    if (this != &other) [[likely]] {
      // Destroy old value first if needed
      if constexpr (Traits::needs_destroy) { PolicyBase::destroy(m_storage); }
      // Then assign
      if constexpr (Traits::is_pod) {
        *get_ptr() = *other.get_ptr();
      } else {
        ::new (get_ptr()) Type(*other.get_ptr());
      }
    }
    return *this;
  }

  // Move assignment operator
  SelectiveInitRes& operator= (SelectiveInitRes&& other) noexcept
  {
    if (this != &other) [[likely]] {
      if constexpr (Traits::needs_destroy) { PolicyBase::destroy(m_storage); }
      if constexpr (Traits::is_pod) {
        *get_ptr() = *other.get_ptr();
      } else {
        ::new (get_ptr()) Type(std::move(*other.get_ptr()));
      }
    }
    return *this;
  }

  // Copy assignment from const Type&
  SelectiveInitRes& operator= (const Type& other) noexcept
  {
    // Destroy old value first if needed
    if constexpr (Traits::needs_destroy) { PolicyBase::destroy(m_storage); }
    if constexpr (Traits::is_pod) {
      *get_ptr() = other;
    } else {
      ::new (get_ptr()) Type(other);
    }
    return *this;
  }

  // Move assignment from Type&&
  SelectiveInitRes& operator= (Type&& other) noexcept
  {
    if constexpr (Traits::needs_destroy) { PolicyBase::destroy(m_storage); }
    if constexpr (Traits::is_pod) {
      *get_ptr() = other;
    } else {
      ::new (get_ptr()) Type(std::move(other));
    }
    return *this;
  }

  // swap function - swap with another SelectiveInitRes
  void swap(SelectiveInitRes& other) noexcept
  {
    if constexpr (Traits::is_pod) {
      std::swap_ranges(m_storage, m_storage + sizeof(Type), other.m_storage);
    } else {
      std::swap(*get_ptr(), *other.get_ptr());
    }
  }

  // swap function - swap with Type
  void swap(Type& other) noexcept
  {
    if constexpr (Traits::is_pod) {
      std::swap_ranges(m_storage, m_storage + sizeof(Type),
                       reinterpret_cast<unsigned char*>(&other));
    } else {
      Type temp(std::move(other));
      other      = std::move(*get_ptr());
      *get_ptr() = std::move(temp);
    }
  }

  // Destructor
  ~SelectiveInitRes() noexcept
  {
    if constexpr (Traits::needs_destroy) { PolicyBase::destroy(m_storage); }
  }

  // Implicit conversion operator
  [[nodiscard]] constexpr operator Type&() noexcept { return *get_ptr(); }

  [[nodiscard]] constexpr operator const Type&() const noexcept
  {
    return *get_ptr();
  }

  // get() method
  [[nodiscard]] constexpr Type& get() noexcept { return *get_ptr(); }

  [[nodiscard]] constexpr const Type& get() const noexcept
  {
    return *get_ptr();
  }

  // operator-> for direct member access
  [[nodiscard]] constexpr Type* operator->() noexcept { return get_ptr(); }

  [[nodiscard]] constexpr const Type* operator->() const noexcept
  {
    return get_ptr();
  }

  // emplace method (void return type)
  template<typename... Args>
  void emplace(Args&&... args) noexcept
  {
    // Destroy old value if needed
    if constexpr (Traits::needs_destroy) { PolicyBase::destroy(m_storage); }
    // Construct new value
    ::new (get_ptr()) Type(std::forward<Args>(args)...);
  }
};

// Non-member swap for ADL (Argument Dependent Lookup) support
template<typename Type, typename InitPolicy>
void swap(SelectiveInitRes<Type, InitPolicy>& lhs,
          SelectiveInitRes<Type, InitPolicy>& rhs) noexcept
{
  lhs.swap(rhs);
}

template<typename Type, typename InitPolicy>
void swap(SelectiveInitRes<Type, InitPolicy>& lhs, Type& rhs) noexcept
{
  lhs.swap(rhs);
}

template<typename Type, typename InitPolicy>
void swap(Type& lhs, SelectiveInitRes<Type, InitPolicy>& rhs) noexcept
{
  rhs.swap(lhs);
}

// Pointer swap - exchange pointers to SelectiveInitRes and Type
template<typename Type, typename InitPolicy>
void swap(SelectiveInitRes<Type, InitPolicy>*& lhs, Type*& rhs) noexcept
{
  auto temp = lhs;
  lhs       = reinterpret_cast<SelectiveInitRes<Type, InitPolicy>*>(rhs);
  rhs       = reinterpret_cast<Type*>(temp);
}

template<typename Type, typename InitPolicy>
void swap(Type*& lhs, SelectiveInitRes<Type, InitPolicy>*& rhs) noexcept
{
  auto temp = rhs;
  rhs       = reinterpret_cast<SelectiveInitRes<Type, InitPolicy>*>(lhs);
  lhs       = reinterpret_cast<Type*>(temp);
}

// Pointer swap for rvalue pointers (from vector::data() etc.)
template<typename Type, typename InitPolicy>
void swap(SelectiveInitRes<Type, InitPolicy>*& lhs, Type*&& rhs) noexcept
{
  auto temp = lhs;
  lhs       = reinterpret_cast<SelectiveInitRes<Type, InitPolicy>*>(rhs);
  rhs       = reinterpret_cast<Type*>(temp);
}

template<typename Type, typename InitPolicy>
void swap(Type*&& lhs, SelectiveInitRes<Type, InitPolicy>*& rhs) noexcept
{
  auto temp = rhs;
  rhs       = reinterpret_cast<SelectiveInitRes<Type, InitPolicy>*>(lhs);
  lhs       = reinterpret_cast<Type*>(temp);
}

// Note: Cannot support std::swap(svec.data(), vec.data()) because:
// 1. svec.data() returns T* (rvalue)
// 2. std::swap takes references, which cannot bind to rvalues
// 3. Use stdex::swap(ptr_var1, ptr_var2) where ptr_var1 and ptr_var2 are lvalue
// pointer variables

}  // namespace stdex

/*
#include <iostream>
#include <print>
#include <string>
#include <vector>
#include "memory/selective_init_res.hpp"
#include "algorithm/radix_sort.hpp"
// ========== 示例 1: Student 类型（包含 POD 和非 POD 成员） ==========

struct Student
{
    // POD 类型（int）：不初始化，保留垃圾值
    int age;
    // 非 POD 类型（std::string）：默认初始化（空字符串）
    std::string name;
    // 非 POD 类型（std::vector）：默认初始化（空向量）
    std::vector<int> scores;

    // 用于 emplace 测试的构造函数
    Student() = default;
    Student(const std::string& n, int a) : age(a), name(n), scores() {}
    Student(int a, const std::string& n, const std::vector<int>& v) : age(a),
name(n), scores(v) {}
};

// 为 Student 类型特化 init_non_pod_members
template<>
void stdex::init_non_pod_members<Student, stdex::InitNonPodMembersTag>(
    unsigned char* storage) noexcept
{
    // 只初始化非 POD 成员：name 和 scores
    // age 是 POD，保留垃圾值
    using NameType = decltype(Student::name);
    using ScoresType = decltype(Student::scores);

    stdex::MemberInitializer<Student, offsetof(Student, name),
NameType>::init(storage); stdex::MemberInitializer<Student, offsetof(Student,
scores), ScoresType>::init(storage);
}

// ========== 示例 2: 纯 POD 类型 ==========

struct Point
{
    double x;
    double y;
    double z;
};

static_assert(stdex::is_pod_v<Point>, "Point should be a POD type");

// ========== 示例 3: 纯非 POD 类型 ==========

struct ComplexObject
{
    std::string id;
    std::vector<double> data;
    int value;  // POD 成员
};

// 为 ComplexObject 类型特化 init_non_pod_members
template<>
void stdex::init_non_pod_members<ComplexObject, stdex::InitNonPodMembersTag>(
    unsigned char* storage) noexcept
{
    // 只初始化非 POD 成员：id 和 data
    // value 是 POD，保留垃圾值
    using IdType = decltype(ComplexObject::id);
    using DataType = decltype(ComplexObject::data);

    stdex::MemberInitializer<ComplexObject, offsetof(ComplexObject, id),
IdType>::init(storage); stdex::MemberInitializer<ComplexObject,
offsetof(ComplexObject, data), DataType>::init(storage);
}

// ========== 测试函数 ==========

void test_student() {
    std::cout << "========== Test 1: Student (mixed POD and non-POD) =========="
<< std::endl;

    // 使用 InitNonPodMembersTag 策略（默认）
    stdex::SelectiveInitRes<Student> student;

    // 直接通过 -> 访问成员（不再需要 .get()）
    std::cout << "age (垃圾值): " << student->age << std::endl;
    std::cout << "name (空字符串): \"" << student->name << "\" (长度: "
              << student->name.size() << ")" << std::endl;
    std::cout << "scores (空向量): size = " << student->scores.size()
              << std::endl;

    // 赋值操作
    student->age = 18;
    student->name = "Tom";
    student->scores.push_back(90);
    student->scores.push_back(95);

    std::cout << "\n赋值后:" << std::endl;
    std::cout << "age: " << student->age << std::endl;
    std::cout << "name: " << student->name << std::endl;
    std::cout << "scores: [";
    for (size_t i = 0; i < student->scores.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << student->scores[i];
    }
    std::cout << "]" << std::endl;
    std::cout << std::endl;
}

void test_student_no_init() {
    std::cout << "========== Test 2: Student with NoInitTag (no initialization)
==========" << std::endl;

    // 使用 NoInitTag 策略：不初始化任何成员
    // 注意：由于非 POD 成员未初始化，访问它们会导致未定义行为
    // 这个测试仅用于演示 NoInitTag 的行为，实际使用时应谨慎
    stdex::SelectiveInitRes<Student, stdex::NoInitTag> student;

    // 先通过 emplace 初始化，然后才能安全访问
    student.emplace("Initialized via emplace", 99);

    std::cout << "name length: " << student->name.size() << std::endl;
    std::cout << "scores size: " << student->scores.size() << std::endl;
    std::cout << "age: " << student->age << std::endl;
    std::cout << std::endl;
}

void test_pod_type() {
    std::cout << "========== Test 3: Point (POD type, no InitPolicy needed)
==========" << std::endl;

    // POD 类型不需要第二个模板参数
    stdex::SelectiveInitRes<Point> point;

    std::cout << "x (垃圾值): " << point->x << std::endl;
    std::cout << "y (垃圾值): " << point->y << std::endl;
    std::cout << "z (垃圾值): " << point->z << std::endl;

    point->x = 1.0;
    point->y = 2.0;
    point->z = 3.0;

    std::cout << "\n赋值后:" << std::endl;
    std::cout << "x: " << point->x << std::endl;
    std::cout << "y: " << point->y << std::endl;
    std::cout << "z: " << point->z << std::endl;
    std::cout << std::endl;
}

void test_complex_object() {
    std::cout << "========== Test 4: ComplexObject (non-POD type with custom
policy) ==========" << std::endl;

    stdex::SelectiveInitRes<ComplexObject> obj;

    std::cout << "id (空字符串): \"" << obj->id << "\" (长度: "
              << obj->id.size() << ")" << std::endl;
    std::cout << "data (空向量): size = " << obj->data.size() << std::endl;
    std::cout << "value (垃圾值): " << obj->value << std::endl;

    obj->id = "OBJ001";
    obj->data.push_back(1.5);
    obj->data.push_back(2.5);
    obj->value = 42;

    std::cout << "\n赋值后:" << std::endl;
    std::cout << "id: " << obj->id << std::endl;
    std::cout << "data: [";
    for (size_t i = 0; i < obj->data.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << obj->data[i];
    }
    std::cout << "]" << std::endl;
    std::cout << "value: " << obj->value << std::endl;
    std::cout << std::endl;
}

void test_copy_and_move() {
    std::cout << "========== Test 5: Copy and Move Semantics ==========" <<
std::endl;

    stdex::SelectiveInitRes<Student> student1;
    student1->age = 20;
    student1->name = "Alice";
    student1->scores.push_back(85);

    // 拷贝构造
    stdex::SelectiveInitRes<Student> student2 = student1;
    std::cout << "Copied student2:" << std::endl;
    std::cout << "  age: " << student2->age << std::endl;
    std::cout << "  name: " << student2->name << std::endl;

    // 移动构造
    stdex::SelectiveInitRes<Student> student3 = std::move(student2);
    std::cout << "Moved student3:" << std::endl;
    std::cout << "  age: " << student3->age << std::endl;
    std::cout << "  name: " << student3->name << std::endl;

    // 赋值
    stdex::SelectiveInitRes<Student> student4;
    student4 = student1;
    std::cout << "Assigned student4:" << std::endl;
    std::cout << "  age: " << student4->age << std::endl;
    std::cout << "  name: " << student4->name << std::endl;
    std::cout << std::endl;
}

void test_emplace() {
    std::cout << "========== Test 6: emplace() Method ==========" << std::endl;

    stdex::SelectiveInitRes<Student> student;
    student->age = 25;
    student->name = "Original";
    student->scores.push_back(100);

    std::cout << "Before emplace:" << std::endl;
    std::cout << "  name: " << student->name << std::endl;

    // 使用 emplace 重新构造
    student.emplace("Emplaced", 30);

    std::cout << "After emplace:" << std::endl;
    std::cout << "  age: " << student->age << std::endl;
    std::cout << "  name: " << student->name << std::endl;
    std::cout << std::endl;
}

void test_get_and_conversion() {
    std::cout << "========== Test 7: get() and Implicit Conversion =========="
<< std::endl;

    stdex::SelectiveInitRes<Student> student;
    student->age = 22;
    student->name = "Bob";

    // 使用 get() 方法
    Student& ref1 = student.get();
    std::cout << "Via get(): " << ref1.name << std::endl;

    // 使用隐式转换
    Student& ref2 = student;
    std::cout << "Via implicit conversion: " << ref2.name << std::endl;
    std::cout << std::endl;
}

int main()
{
    std::cout << "================ SelectiveInitRes Test Suite ================"
<< std::endl; std::cout << std::endl;

    test_student();
    test_student_no_init();
    test_pod_type();
    test_complex_object();
    test_copy_and_move();
    test_emplace();
    test_get_and_conversion();

    std::cout << "================ All Tests Completed ================" <<
std::endl;

    std::vector<Student> vec{
        { 16,   "Bob",  { 70, 90, 90, 60 } },
        { 18, "Alice",  { 80, 80, 80, 70 } },
        { 17,   "Dog",  { 90, 70, 78, 80 } },
        { 15,   "Cat",  { 63, 80, 75, 75 } }
    };

    std::vector<stdex::SelectiveInitRes<Student>> svec(vec.size());
    std::cout << "Before assignment - svec age: " << svec[0]->age << ", name:
\"" << svec[0]->name << "\", scores size: " << svec[0]->scores.size() <<
std::endl; svec[0] = std::move(vec[0]); // should work now std::cout << "After
assignment - svec age: " << svec[0]->age << ", name: \"" << svec[0]->name <<
"\", scores size: " << svec[0]->scores.size() << std::endl;

    // Test swap with Type
    Student tempStudent(100, "Temp", {50, 60});
    std::cout << "Before swap with Type - tempStudent: " << tempStudent.name <<
std::endl; svec[1].swap(tempStudent); std::cout << "After swap with Type -
svec[1]: " << svec[1]->name << ", tempStudent: " << tempStudent.name <<
std::endl;

    // Note: std::swap(vec, svec) cannot work because vec and svec have
different element types
    // vec is std::vector<Student>
    // svec is std::vector<stdex::SelectiveInitRes<Student>>
    // These are different types and cannot be swapped

    // For pointer swap, store pointers in lvalue variables first
    stdex::SelectiveInitRes<Student>* ptr_s = svec.data();
    Student* ptr_v = vec.data();
    stdex::swap(ptr_s, ptr_v);
    // After swap: ptr_s now holds the value of vec.data() (Student*)
    //            ptr_v now holds the value of svec.data()
(SelectiveInitRes<Student>*)
    // Use reinterpret_cast to access the actual objects
    auto svec_ptr = reinterpret_cast<stdex::SelectiveInitRes<Student>*>(ptr_v);
    std::cout << "After pointer swap - svec_ptr points to Student with name: "
<< (*svec_ptr)->name << std::endl;

    auto print_students = [](const std::vector<Student>& students) -> void {
      for (const auto& p : students) {
        std::println("age: {}, name: {}, scores: {}", p.age, p.name, p.scores);
      }
      };

    std::vector<Student> students = {
      { 16,   "Bob", { 70, 90, 90, 60 } },
      { 18, "Alice", { 80, 80, 80, 70 } },
      { 17,   "Dog", { 90, 70, 78, 80 } },
      { 15,   "Cat", { 63, 80, 75, 75 } }
    };

    std::cout << "Original: ";
    print_students(students);

    // Method 1: Sort by x using member studenter
    radix_sort_by_member(students.begin(), students.end(), &Student::age);
    std::cout << "Sorted by x: ";
    print_students(students);
    return 0;
}
*/