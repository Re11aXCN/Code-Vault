#pragma once

#include <string>
#include <variant>

using namespace std;

// 模版分离实例化
// 通过声明特化类型之后，就能够将模版实现拿到cpp中实现，不会暴露出信息
// .h中实现会暴露信息，同时.h展开代码也少，编译时间快，也避免了代码膨胀

using Object = variant<int, double, string>; // 40字节，永远按最大的存然后加8字节
// string32 + 8字节是因为有个index取类型，index是size_t8字节

/*
static vector<variant<int, double, string>> objects; // 效率低些，且内存大，你计算double相加不能够推导出下一个类型是什么，不能矢量化
// 高效，方便矢量化连续计算快(因为紧挨着)
static vector<int> int_objects;
static vector<double> double_objects;
static vector<string> string_objects;
// 更进一步
static tuple< vector<int>, vector<double>, vector<string> > tuple_objects;
获取通过get<0>(tuple_objects),	get<1>(tuple_objects)
*/
void add_object(Object o);
void print_objects();
