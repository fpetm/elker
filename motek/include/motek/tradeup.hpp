#pragma once
#include "skin.hpp"
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <iostream>

#include <array>
#include <unordered_map>
#include <vector>

#include "partitions.hpp"

#include "log.hpp"

namespace motek {
class TradeUp {
public:
  TradeUp(size_t n, WearType avg_wear, bool st, SkinRarity r,
          SkinCondition cond_no_st);

  void Clear();

public:
  size_t nSkins;
  WearType average_wear;
  bool stattrak;
  SkinRarity rarity;
  SkinCondition condition_no_stattrak;

  bool computed;

  float cost, grossreturn, netreturn;
  float variance, stddev, vmr, profitchance, ev;

  Eigen::SparseVector<float> probability;

  Eigen::SparseVector<float> mask_vector;
  Eigen::SparseVector<float> mask_big;
};
} // namespace motek
