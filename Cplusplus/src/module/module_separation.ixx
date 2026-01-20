module;
#include <vector>

export module deitel.math;  // introduces the module name

namespace deitel::math {
// exported function average; namespace deitel::math implicitly exported
export double average(const std::vector<int>& values);

// non-exported function cube is not implicitly exported
int cube(int x) { return x * x * x; }

};  // namespace deitel::math