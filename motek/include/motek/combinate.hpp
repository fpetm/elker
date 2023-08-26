#pragma once
#include "log.hpp"
#include <iostream>

#include <algorithm>
#include <string>

namespace motek {
constexpr size_t nCr(size_t n, size_t c) {
  size_t num = 1, denom = 1;
  for (size_t k = 0; k < c; k++) {
    num *= n - k;
    denom *= k + 1;
  }

  return num / denom;
}

constexpr std::vector<std::vector<size_t>> combination(size_t n, size_t s) {
  // std::cout << n << " : " << s << "\n";
  if (n < s || n < 0)
    return {};
  std::vector<bool> mask(n);
  std::fill(mask.end() - s, mask.end(), true);

  std::vector<std::vector<size_t>> r(nCr(n, s));
  size_t k = 0;
  do {
    r[k].resize(s);
    size_t j = 0;
    for (size_t i = 0; i < n; i++) {
      if (mask[i])
        r[k][j++] = i;
    }
    k++;
  } while (std::next_permutation(mask.begin(), mask.end()));

  return r;
}
} // namespace motek
