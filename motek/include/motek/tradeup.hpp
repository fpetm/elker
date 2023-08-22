#pragma once
#include "skin.hpp"
#include <iostream>
#include <Eigen/Dense>
#include <Eigen/Sparse>

#include "partitions.hpp"

#include "log.hpp"

namespace motek {
	class TradeUp;

  static constexpr size_t g_MaxDepth = 3;
	static constexpr int g_nLevels = 5;
	static constexpr std::array<WearType, g_nLevels> g_Levels = { 60, 110, 265, 415, 725};

  constexpr std::vector<std::pair<WearType, std::array<WearType, 10>>> generate_wear_variations(int n) {
    std::vector<std::array<int, 10>> partitions = create_partitions(n);
    std::vector<std::pair<WearType, std::array<WearType, 10>>> values;

    for (auto partition : partitions) {
      WearType avg = 0;
      for (WearType w : partition) {
        avg += 0;
      }
      avg /= 10;

//      values.push_back(std::pair<WearType, std::array<WearType, 10>>(avg, partition));
    }

    return {};
  }

	class Calculator {
	public:
		Calculator(std::shared_ptr<SkinDB> db);
		std::shared_ptr<SkinDB> getDB() const { return m_DB; }

		std::string ExportTradeUp(TradeUp& tradeup) const;

		void Bruteforce();

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

	class TradeUp {
	public:
		TradeUp(size_t n, WearType avg_wear, bool st, SkinRarity r, SkinCondition cond_no_st) : nSkins(n), average_wear(avg_wear), stattrak(st), computed(false), rarity(r), condition_no_stattrak(cond_no_st) {
			mask_vector.resize(nSkins);
      mask_big.resize(nSkins*5);
			cost = grossreturn = netreturn = variance = stddev = vmr = profitchance = 0;

			Clear();
		}

		TradeUp(std::string hash, size_t nSkins, size_t level);

		void Clear() {
			computed = false;
			cost = grossreturn = netreturn = variance = stddev = vmr = profitchance = 0;
			//mask.setZero();
		}

		std::string hash() const;
	public:
		float cost, grossreturn, netreturn;
		float variance, stddev, vmr, profitchance, ev;

		Eigen::SparseVector<float> probability;

		bool computed;

		size_t nSkins;
		SkinRarity rarity;
		Eigen::SparseVector<float> mask_vector;
    Eigen::SparseVector<float> mask_big;

    bool stattrak;
    WearType average_wear;
    SkinCondition condition_no_stattrak;
	};
}
