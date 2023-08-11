#pragma once
#include "skin.hpp"
#include <iostream>
#include <Eigen/Dense>
#include <Eigen/Sparse>

namespace motek {
	class TradeUp;

	static constexpr int g_nLevels = 5;
	static constexpr std::array<WearType, g_nLevels> g_Levels = { 60, 110, 265, 415, 725};

	class Calculator {
	public:
		Calculator(std::shared_ptr<SkinDB> db);
		std::shared_ptr<SkinDB> getDB() const { return m_DB; }

		std::string ExportTradeUp(TradeUp& tradeup) const;

		void Bruteforce();

        bool Compute(TradeUp &tradeup) const {
            return ComputeGross(tradeup) > ComputeCost(tradeup);
        }
		float ComputeGross(TradeUp &tradeup) const;
		float ComputeCost(TradeUp &tradeup) const;
		void ComputeStatistical(TradeUp& tradeup) const;

		std::shared_ptr<SkinDB> m_DB;

	private:
		Eigen::SparseVector<float> m_Prices[SkinRarity::Contraband][g_nLevels*2];
		Eigen::SparseVector<float> m_MappedPrices[SkinRarity::Contraband][g_nLevels * 2];
		Eigen::SparseVector<float> m_MappedPricesWithFees[SkinRarity::Contraband][g_nLevels * 2];

		Eigen::SparseVector<float> m_Factor[SkinRarity::Contraband][g_nLevels * 2];
		Eigen::SparseMatrix<float> m_Transformer[SkinRarity::Contraband][g_nLevels * 2];
	};

	class TradeUp {
	public:
		TradeUp(size_t n, size_t l, SkinRarity r) : nSkins(n), level(l), computed(false), rarity(r) {
			mask.resize(nSkins);
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
		Eigen::SparseVector<float> mask;
		size_t level;
	};
}
