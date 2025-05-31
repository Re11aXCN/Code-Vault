#ifndef _LAMBDA_H_
#define _LAMBDA_H_
#include <iostream>
#include <string>
#include <functional>
#include <vector>
typedef void (*Func)(int, int);
void easy_use_lambda()
{
    // 1. 匿名调用
    [](int x, int y) { std::cout << x + y; }(1, 2);

    // 2. auto赋值调用
    auto autoFunc = [](std::string name) { std::cout << "Hello, " << name << "!" << std::endl; };
    autoFunc("World");

    //3. 函数指针赋值调用
    void (*ptrFunc)(int, int) = [](int x, int y) { std::cout << x + y; };
    ptrFunc(1, 2);
    Func defptrFunc = [](int x, int y) { std::cout << x + y; };
    defptrFunc(1, 2);

    //4. function对象赋值调用
    std::function<void(int, int)> functionFunc = [](int x, int y) { std::cout << x + y; };
    functionFunc(1, 2);
}

void lambda_capture_bind()
{
    std::size_t age = 18;
    std::string first_name = "Tom";

    // 1. 值捕获
    auto valueCapFunc = [age, first_name](std::string last_name) {
        std::cout << "Hello, " << first_name << " " << last_name
            << " (" << age << " years old)" << std::endl;
        };
    valueCapFunc("Jerry");

    // 2. 引用捕获
    auto refCapFunc = [&age, &first_name](std::string last_name) {
        std::cout << "Hello, " << first_name << " " << last_name
            << " (" << age << " years old)" << std::endl;
        };
    refCapFunc("Jerry");

    // 3. 隐式值捕获
    auto implicitValueCapFunc = [=](std::string last_name) {
        std::cout << "Hello, " << first_name << " " << last_name
            << " (" << age << " years old)" << std::endl;
        };
    implicitValueCapFunc("Jerry");

    // 4. 隐式引用捕获
    auto implicitRefCapFunc = [&](std::string last_name) {
        std::cout << "Hello, " << first_name << " " << last_name
            << " (" << age << " years old)" << std::endl;
        };
    implicitRefCapFunc("Jerry");

    std::vector<std::function<void(std::string)>> vecFunc;

    // 基础lambda
    vecFunc.push_back([](std::string name) {
        std::cout << "Hello, " << name << "!" << std::endl;
        });

    // 5. 危险，不要使用&捕获局部变量，出了lambda_capture函数作用域变量就被释放了
    vecFunc.push_back([&first_name, age](std::string name) {
        std::cout << "Hello, " << first_name << " " << name
            << " (" << age << " years old)" << std::endl;
        });

    // 6. 函数对象
    struct Print {
        void operator()(std::string name) {
            std::cout << "Hello, " << name << "!" << std::endl;
        }
    } print;

    vecFunc.push_back(print);

    auto boundPrint = std::bind(print, std::placeholders::_1);
    vecFunc.push_back(boundPrint);

    // 7. 带多个参数的lambda
    auto complex_param_func = [](std::string name, int age, bool is_male) {
        std::cout << "Hello, " << name << " ("
            << (is_male ? "male" : "female")
            << ", " << age << " years old)" << std::endl;
        };

    // 8. 正确使用std::bind固定部分参数
    auto boundComplex = std::bind(
        complex_param_func,
        std::placeholders::_1,  // 保留第一个参数
        25,                     // 固定年龄
        true                    // 固定性别
    );

    // 添加到容器（需要匹配签名）
    vecFunc.push_back(boundComplex);
    boundComplex("Bob"); // 相当于complex_param_func("Bob", 25, true)

    struct Person {
        void print(std::string name, int age, bool is_male) {
            std::cout << "Hello, " << name << " ("
                << (is_male ? "male" : "female")
                << ", " << age << " years old)" << std::endl;
        }
        static void static_print(std::string name, int age, bool is_male) {
            std::cout << "Hello, " << name << " ("
                << (is_male ? "male" : "female")
                << ", " << age << " years old)" << std::endl;
        }
    };
    // 9. 绑定类对象静态成员方法，不需要定义对象，函数需要加&
    auto person_print = std::bind(&Person::static_print, "Bob", std::placeholders::_2, true);
    //person_print(2); //错
    person_print("Bob", 2);//对
    person_print(2, true);//对
    // 10. 绑定类对象成员方法，需要定义对象，函数需要加&
    Person person;
    auto bound_person_print = std::bind(&Person::print, &person, std::placeholders::_1, std::placeholders::_2, true);
    bound_person_print("Bob", 2);

    // 11. 绑定全局函数，不需要加&
    auto global_print = std::bind(print, "Bob");

    // 12. function 和 bind
    std::function<void(std::string, int, bool)> func_obj1 = bound_person_print;
    std::function<void(Person&, std::string, int, bool)> func_obj2 = &Person::print;


}
#endif // !_LAMBDA_H_