#include "calculator.hpp"

#include <iostream>

int main() {
#if __cplusplus >= 202100L
  constexpr int cpp_standard = 23;
#elif __cplusplus >= 202002L
  constexpr int cpp_standard = 20;
#else
  constexpr int cpp_standard = 17;
#endif
  std::cout << "cpp-env example: C++" << cpp_standard << '\n';
  std::cout << "2 + 3 = " << example::add(2, 3) << '\n';
  return example::add(2, 3) == 5 ? 0 : 1;
}
