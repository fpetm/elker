#pragma once

#include "tradeup.hpp"

namespace motek {
using wear_config_t = std::unordered_map<SkinCondition, size_t>;
using wear_configs_t = std::unordered_map<wear_t, wear_config_t>;

wear_configs_t
generate_wear_variations(const std::vector<std::vector<wear_t>> &wear_tuples,
                         bool stattrak);

class Calculator {
public:
  explicit Calculator(std::shared_ptr<SkinDB> database);
  [[nodiscard]] std::shared_ptr<SkinDB> getDB() const { return m_DB; }

  [[nodiscard]] std::string ExportTradeUp(TradeUp &tradeup) const;

  void Bruteforce(const std::vector<std::vector<wear_t>> &wear_tuples,
                  size_t max_depth) const;

  [[nodiscard]] float ComputeGross(TradeUp &tradeup) const;
  [[nodiscard]] float ComputeCost(TradeUp &tradeup) const;

  void ComputeStatistical(TradeUp &tradeup) const;

  std::shared_ptr<SkinDB> m_DB;

private:
  Eigen::VectorXf m_Prices[2][SkinRarity::Contraband][g_WearRangeMax];
  Eigen::VectorXf m_MappedPrices[2][SkinRarity::Contraband][g_WearRangeMax];
  Eigen::VectorXf m_MappedPricesWithFees[2][SkinRarity::Contraband]
                                        [g_WearRangeMax];
  Eigen::VectorXf m_PricesCompressed[2][SkinRarity::Contraband];

  Eigen::SparseVector<float> m_Factor[SkinRarity::Contraband];
  Eigen::SparseMatrix<float> m_Transformer[SkinRarity::Contraband];
};

} // namespace motek
