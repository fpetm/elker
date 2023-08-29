#include <motek/combinatorics.hpp>

namespace motek::combinatorics {
std::vector<size_t> first_combination(size_t n, size_t t) {
  std::vector<size_t> c(t + 2);
  int j, x;
  j = x = 0;

  for (j = 1; j <= t; j++) {
    c[j - 1] = j - 1;
  }
  c[t] = n;
  c[t + 1] = 0;
  return c;
}

void next_combination(std::vector<size_t> &c, size_t t) {
  int j = 1;
  while (c[j - 1] + 1 == c[j]) {
    c[j - 1] = j - 1;
    j++;
    if (j > t)
      break;
  }
  c[j - 1]++;
}
} // namespace motek::combinatorics