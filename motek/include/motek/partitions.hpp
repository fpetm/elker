#pragma once
#include <array>
#include <vector>

namespace motek {
	constexpr std::array<std::array<int, 1>, 1>   p1 = { { {10} } };
	constexpr std::array<std::array<int, 2>, 5>   p2 = { { {5, 5}, { 6, 4 }, { 7, 3 }, { 8, 2 }, { 9, 1 }} };
	constexpr std::array<std::array<int, 3>, 8>   p3 = { { {4, 3, 3}, {4, 4, 2}, {5, 3, 2}, {5, 4, 1}, {6, 2, 2}, {6, 3, 1}, {7, 2, 1}, {8, 1, 1} } };
	constexpr std::array<std::array<int, 4>, 9>   p4 = { { {3, 3, 2, 2}, {3, 3, 3, 1}, {4, 2, 2, 2}, {4, 3, 2, 1}, {4, 4, 1, 1}, {5, 2, 2, 1}, {5, 3, 1, 1}, {6, 2, 1, 1}, {7, 1, 1, 1} } };
	constexpr std::array<std::array<int, 5>, 7>   p5 = { { {2, 2, 2, 2, 2}, {3, 2, 2, 2, 1}, {3, 3, 2, 1, 1}, {4, 2, 2, 1, 1}, {4, 3, 1, 1, 1}, {5, 2, 1, 1, 1}, {6, 1, 1, 1, 1} } };
	constexpr std::array<std::array<int, 6>, 5>   p6 = { { {2, 2, 2, 2, 1, 1}, {3, 2, 2, 1, 1, 1}, {3, 3, 1, 1, 1, 1}, {4, 2, 1, 1, 1, 1}, {5, 1, 1, 1, 1, 1} } };
	constexpr std::array<std::array<int, 7>, 3>   p7 = { { {2, 2, 2, 1, 1, 1, 1}, { 3, 2, 1, 1, 1, 1, 1 }, { 4, 1, 1, 1, 1, 1, 1 } } };
	constexpr std::array<std::array<int, 8>, 2>   p8 = { { {2, 2, 1, 1, 1, 1, 1, 1}, {3, 1, 1, 1, 1, 1, 1, 1} } };
	constexpr std::array<std::array<int, 9>, 1>   p9 = { { {2, 1, 1, 1, 1, 1, 1, 1, 1} } };
	constexpr std::array<std::array<int, 10>, 1> p10 = { { {1, 1, 1, 1, 1, 1, 1, 1, 1, 1} } };

//  constexpr std::vector<std::array<int, 10>> partition(int n) {
  constexpr std::vector<std::array<int, 10>> create_partitions(int n) {
    std::vector<std::vector<int>> partitions;
    switch (n) {
      case  1: partitions = { {10} }; break;
	    case  2: partitions = { {5, 5}, { 6, 4 }, { 7, 3 }, { 8, 2 }, { 9, 1 }}; break;
	    case  3: partitions = { {4, 3, 3}, {4, 4, 2}, {5, 3, 2}, {5, 4, 1}, {6, 2, 2}, {6, 3, 1}, {7, 2, 1}, {8, 1, 1} }; break;
	    case  4: partitions = { {3, 3, 2, 2}, {3, 3, 3, 1}, {4, 2, 2, 2}, {4, 3, 2, 1}, {4, 4, 1, 1}, {5, 2, 2, 1}, {5, 3, 1, 1}, {6, 2, 1, 1}, {7, 1, 1, 1} }; break;
	    case  5: partitions = { {2, 2, 2, 2, 2}, {3, 2, 2, 2, 1}, {3, 3, 2, 1, 1}, {4, 2, 2, 1, 1}, {4, 3, 1, 1, 1}, {5, 2, 1, 1, 1}, {6, 1, 1, 1, 1} }; break;
	    case  6: partitions = { {2, 2, 2, 2, 1, 1}, {3, 2, 2, 1, 1, 1}, {3, 3, 1, 1, 1, 1}, {4, 2, 1, 1, 1, 1}, {5, 1, 1, 1, 1, 1} }; break;
	    case  7: partitions = { {2, 2, 2, 1, 1, 1, 1}, { 3, 2, 1, 1, 1, 1, 1 }, { 4, 1, 1, 1, 1, 1, 1 } }; break;
	    case  8: partitions = { {2, 2, 1, 1, 1, 1, 1, 1}, {3, 1, 1, 1, 1, 1, 1, 1} }; break;
	    case  9: partitions = { {2, 1, 1, 1, 1, 1, 1, 1, 1} }; break;
	    case 10: partitions = { {1, 1, 1, 1, 1, 1, 1, 1, 1, 1} }; break;
    }

    std::vector<std::vector<int>> accumulated(partitions.size());
    for (int i = 0; i < partitions.size(); i++) {
      const std::vector<int> partition = partitions[i];
      int s = 0;
      for (int j = 0; j < partition.size(); j++) {
        s += partition[j];
        accumulated[i].push_back(s);
      }
    }

    std::vector<std::array<int, 10>> values(partitions.size());

    for (int n = 0; n < partitions.size(); n++) {
      int k = 0;
      for (int i = 0; i < 10; i++) {
        if (accumulated[n][k] == i) k++;
        values[n][i] = k;
      }
    }

    return values;
  }
}
