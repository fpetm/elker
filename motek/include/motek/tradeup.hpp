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
  TradeUp(size_t nskins, wear_t avg_wear, bool stattrak, SkinRarity rarity);
  void SetPrices(float cost, float grossreturn, float netreturn);
  void SetAmount(int id, int amount);
  void SetAmountCondition(int id, int amount,
                          SkinCondition condition_no_stattrak);

  void Clear();

  bool m_Computed;

  size_t m_NSkins;
  wear_t m_AverageWear;
  bool m_StatTrak;
  SkinRarity m_Rarity;
  //  SkinCondition m_ConditionNoStattrak;

  float m_Cost, m_GrossReturn, m_NetReturn;
  float m_ProfitChance;

  Eigen::SparseVector<float> m_Probability;

  Eigen::SparseVector<float> m_Mask;
  Eigen::SparseVector<float> m_MaskBig;
};
} // namespace motek
