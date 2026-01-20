// module partitions
export module containers : associative;
import std;

export namespace containers {
std::map<std::string_view, int> get_map_sv_i()
{
  return std::map<std::string_view, int> {};
}
}  // namespace containers