#pragma once
#include <algorithm>
#include <array>
#include <unordered_map>
#include <vector>

#include <cstdint>

namespace motek::combinatorics {
constexpr size_t g_BitCount = 4;
extern const std::array<std::vector<uint64_t>, 11> g_Compositions;

extern const std::array<std::vector<uint64_t>, 11> g_CompositionsAccumulated;

extern const std::array<std::vector<uint64_t>, 11> g_CompositionsFlat;

extern const std::unordered_map<uint64_t, std::vector<uint64_t>> g_Variations;

inline constexpr uint64_t at(uint64_t element, uint64_t index) {
  const uint64_t shiftn = 4 * index;
  const uint64_t mask = static_cast<uint64_t>(0xF) << shiftn;
  return (element & mask) >> shiftn;
}

constexpr size_t nCr(size_t n, size_t c) {
  size_t num = 1, denom = 1;
  for (size_t k = 0; k < c; k++) {
    num *= n - k;
    denom *= k + 1;
  }

  return num / denom;
}

std::vector<size_t> first_combination(size_t n, size_t t);

void next_combination(std::vector<size_t> &c, size_t t);

} // namespace motek::combinatorics
