// Module implementation unit for the module deitel.math.
#include <numeric>;

module deitel.math;  // this file's contents belong to module deitel.math

namespace deitel::math {
// average function's implementation
double average(const std::vector<int>& values)
{
  double total { std::accumulate(values.begin(), values.end(), 0.0) };
  return total / values.size();
}
}  // namespace deitel::math