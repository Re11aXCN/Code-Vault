#pragma once

#include <string>
#include <variant>

using namespace std;

// ģ�����ʵ����
// ͨ�������ػ�����֮�󣬾��ܹ���ģ��ʵ���õ�cpp��ʵ�֣����ᱩ¶����Ϣ
// .h��ʵ�ֻᱩ¶��Ϣ��ͬʱ.hչ������Ҳ�٣�����ʱ��죬Ҳ�����˴�������

using Object = variant<int, double, string>; // 40�ֽڣ���Զ�����Ĵ�Ȼ���8�ֽ�
// string32 + 8�ֽ�����Ϊ�и�indexȡ���ͣ�index��size_t8�ֽ�

/*
static vector<variant<int, double, string>> objects; // Ч�ʵ�Щ�����ڴ�������double��Ӳ��ܹ��Ƶ�����һ��������ʲô������ʸ����
// ��Ч������ʸ�������������(��Ϊ������)
static vector<int> int_objects;
static vector<double> double_objects;
static vector<string> string_objects;
// ����һ��
static tuple< vector<int>, vector<double>, vector<string> > tuple_objects;
��ȡͨ��get<0>(tuple_objects),	get<1>(tuple_objects)
*/
void add_object(Object o);
void print_objects();
