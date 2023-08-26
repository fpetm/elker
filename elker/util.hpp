#pragma once
#include <chrono>
#include <iostream>

namespace elker {
#define TIMER_START                                                            \
  {                                                                            \
    std::clock_t start = std::clock();
#define TIMER_END(msg, msg2)                                                   \
  std::cout << msg << " "                                                      \
            << static_cast<double>(std::clock() - start) /                     \
                   static_cast<double>(CLOCKS_PER_SEC)                         \
            << msg2;                                                           \
  }
} // namespace elker