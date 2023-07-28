#pragma once
#include "skin.hpp"
#include <iostream>
#pragma warning(push, 0)
#include <Eigen/Dense>
#include <Eigen/Sparse>
#pragma warning(pop)

namespace motek {
	class TradeUp;

	static constexpr int g_nLevels = 5;
	static constexpr std::array<float, g_nLevels> g_Levels = { 0.06f, 0.11f, 0.265f, 0.415f, 0.725f};
	class Calculator {
	public:
		Calculator(std::shared_ptr<SkinDB> db);
		std::shared_ptr<SkinDB> getDB() const { return m_DB; }

		std::string ExportTradeUp(TradeUp& tradeup) const;

		void Bruteforce();

		bool Compute(TradeUp &tradeup) const;
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
			mask.setZero();
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