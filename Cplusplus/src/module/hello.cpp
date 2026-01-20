// import <print>;
// #include <print>
import std;
import hello;
import deitel.math;
import containers;
import views;

int main()
{
  std::println("value: {}", my_constexpr_func());
  std::println("value: {}", deitel::math::average({1, 2, 3, 4, 5}));
  // cube non export , compilation error
  // std::println("value: {}", deitel::math::cube(3));

  std::vector vec_int { containers::get_vector_i() };
  std::map map_sv_int { containers::get_map_sv_i() };

  auto sv { views::hello_world_string_view() };
  auto span { views::hello_world_span() };
  std::println("string_view: {}, size: {}", sv, sv.size());
  std::println("span: {}, size: {}", span, span.size());
  
  // 只能是auto，不能是TimeUtils::Time，因为TimeUtils::Time没有导出，命名空间TimeUtils没有导出
  // 我们只导出了一个封装获取TimeUtils::Time的方法 TimeUtils::get_utc_time
  // TimeUtils::Time 类型是可达的，但是不可见
  // 
  // TimeUtils::Time time_obj { TimeUtils::get_time_obj() }; // Error
  auto time_obj { TimeUtils::get_time_obj() };
  std::println("UTC {}", time_obj.get_utc_time());
  return 0;
}