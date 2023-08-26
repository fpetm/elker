#pragma once

#include "tradeup.hpp"

namespace motek {
  static constexpr size_t g_MaxDepth = 2;
	static constexpr int g_nLevels = 5;
	static constexpr std::array<WearType, g_nLevels> g_Levels = { 60, 110, 265, 415, 725};

  typedef std::unordered_map<SkinCondition, size_t> wear_config_t;
  typedef std::unordered_map<WearType, wear_config_t> wear_configs_t;
  wear_configs_t generate_wear_variations(const std::vector<std::vector<WearType>>& wear_tuples, bool stattrak);

	class Calculator {
	public:
		Calculator(std::shared_ptr<SkinDB> db);
		std::shared_ptr<SkinDB> getDB() const { return m_DB; }

		std::string ExportTradeUp(TradeUp& tradeup) const;

	  void Bruteforce(const std::vector<std::vector<WearType>>& wear_tuples);

        bool Compute(TradeUp &tradeup) const {
			return ComputeGross(tradeup) > ComputeCost(tradeup);
           // return std::abs(double(ComputeGross(tradeup)) - double(ComputeCost(tradeup)) - 2.9121f) < 0.01f;
        }
		float ComputeGross(TradeUp &tradeup) const;
		float ComputeCost(TradeUp &tradeup) const;
		void ComputeStatistical(TradeUp& tradeup) const;

		std::shared_ptr<SkinDB> m_DB;

	private:
		Eigen::VectorXf m_Prices[2][SkinRarity::Contraband][g_WearRangeMax];
		Eigen::VectorXf m_MappedPrices[2][SkinRarity::Contraband][g_WearRangeMax];
		Eigen::VectorXf m_MappedPricesWithFees[2][SkinRarity::Contraband][g_WearRangeMax];
    Eigen::VectorXf m_PricesCompressed[2][SkinRarity::Contraband];

		Eigen::SparseVector<float> m_Factor[SkinRarity::Contraband];
		Eigen::SparseMatrix<float> m_Transformer[SkinRarity::Contraband];
	};

}
