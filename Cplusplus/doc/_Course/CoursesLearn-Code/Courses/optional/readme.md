前言
小彭老师C++实战演练系列之“自己实现所有STL容器”。比起艰深的STL源码解读，不如自己动手实现STL，更能生动直观地掌握底层技术细节，破除模棱两可心理，了解常见的最佳实践。这是本系列试水的第六课，自己实现std::optional可选值容器。本期视频中我们介绍了如何借助tag类、union惰性初始化、万能引用、完美转发等技巧，实现一个可以存放任意类型的可选值容器。最终借助所学知识，封装出了和标准库一样的可选值容器std::optional，完美支持存放任意类型的值，同时支持为空。in_place和nullopt等tag类重载构造函数的妙用真是醍醐灌顶，我们不仅实现了value、value_or、emplace、*和->运算符，还实现了C++23新增的transform、and_then、or_else，彻底迈向函数式！介绍了C++17的CTAD机制，感受if-auto语法的简洁有力。最终，还顺便介绍了名字空间的ADL机制如何帮助多态，并实现了自适配swap。且C++14就能编译，如果你的编译器无法升级到C++17，可以集成小彭老师的Optional类，直奔C++23。如果反响较好，点赞超过300立即加更下一期。

1. tag 类的技巧

2. 为什么 has_value 不 public 成员？

3. 封装 exception 类

4. 为什么异常类的 what 不返回 std::string 呢？

5. 怎样处理包装一个没有构造函数的 T 呢？ union 不会默认初始化，placement-new and placement-delete

6. 为什么要定义四个 value() 函数 const 和 非 const

7. 为什么 value_or 有两个函数 一个是 const &,另一个是 &&？因为 const & 是可以被 this 调用的，不仅仅是 const this。&& 是需要移出来的，因为它马上就要死了

8. 为什么拷贝构造函数中不写 m_value = ? 因为它还没有构造出来呢，怎么调用 赋值呢！

9. 为什么移动构造函数中仍然需要std::move(that.m_value)？因为类型衰变了，右值引用是左值

10. 析构函数里面不需要将m_has_value 设为 false!

11. 为什么构造函数和赋值构造中使用到了 m_value(std::move(value))？因为是我们的value我们可以右值优化！

12. 自己给自己赋值使用 = ，注意右值的时候将 that 清空？包括 m_value 和 m_has_value

13. 为什么需要 emplace 函数进行优化？因为 optc = C(1,2); 会先调用构造函数，再调用移动构造

14. 为什么emplace 函数 m_has_value 要最后置为 true？防止析构的时候二次释放内存。

15. reset() 就相当于 =nullopt

16. operator= 要注意自赋值的问题！

17. 参数包注意万能引用和完美转发

18. 相关函数加上 noexcept , 并且比较烧脑的加上 noexcept(std::is_nothrow_move_assignable<T>) 或者烧脑的加上 noexcept(noexcept(T(std::move(std::declval<T>())))

19. 使 Option 支持 * 语法，同样是四个函数，且不做任何检查 为 noexcept

20. 什么时候需要用到 address_of ？当我们的类定义了 operator& 的时候！

21. 定义了 opreator bool() 之后会有什么问题呢？解决这个问题需要添加 explicit

22. 实现 operator==() 需要实现 1）和 nullopt判断 2）和另一个类型Optional<U> 比较

23. 没有CTAD支持的情况我们可以怎么做？提供 makeOptional 函数。

24. using RetType = std::decay_t<decltype(f(m_value))> 为什么不如 using RetType = std::remove_cvref_t<decltype(f(m_value))> ? 因为 decay 的衰变很强会将 int[] -> int * ，因此常常用来作为函数的参数；而返回值我们需要一处 cv 和 ref 即可 。如果没有 C++20 使用 remove_cv_t<remove_reference_t<>>

25. 为什么 value_or 比 or_else 效率要高？因为惰性求值，or_else 可以不一定求值

26. 为什么 operator=(T &&value) 要比 operator=(T value) 安全呢？因为 operator=(T value) 内部执行时中途产生异常会造成外部的变量无法使用，因为它(调用 operator= 的作用域）已经到了函数参数中去了。

27. 为什么使用 using std::swap ？使用到了 ADL ，如果你没有实现 swap 则可以使用你实现的 swap，如果你没有实现则跳转到 std 当中寻找。


