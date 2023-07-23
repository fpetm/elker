#pragma once
#include "skin.hpp"
#include <iostream>
#pragma warning(push, 0)
#include <Eigen/Dense>
#pragma warning(pop)

namespace elker {
	class TradeUp;

	static constexpr int g_nLevels = 5;
	static constexpr std::array<float, g_nLevels> g_Levels = { 0.06f, 0.11f, 0.265f, 0.415f, 0.725f};
	class Calculator {
	public:
		Calculator(std::shared_ptr<SkinDB> db);
		std::shared_ptr<SkinDB> getDB() const { return m_DB; }

		std::string ExportTradeUp(TradeUp& tradeup);

		void Bruteforce();

		bool Compute(TradeUp &tradeup) const;
		void ComputeStatistical(TradeUp& tradeup) const;

		std::shared_ptr<SkinDB> m_DB;

	private:
		Eigen::VectorXf m_Prices[g_nLevels*2];
		Eigen::VectorXf m_MappedPrices[g_nLevels * 2];
		Eigen::VectorXf m_MappedPricesWithFees[g_nLevels * 2];

		Eigen::VectorXf m_Factor[g_nLevels * 2];
		Eigen::MatrixXf m_Transformer[g_nLevels * 2];
	};

	class TradeUp {
	public:
		TradeUp(size_t n, int l) : nSkins(n), level(l), computed(false) {
			mask.resize(nSkins);
			cost = grossreturn = netreturn = variance = stddev = vmr = profitchance = 0;

			Clear();
		}

		void Clear() {
			computed = false;
			cost = grossreturn = netreturn = variance = stddev = vmr = profitchance = 0;
			for (int i = 0; i < nSkins; i++) {
				mask(i) = 0;
			}
		}
	public:
		float cost, grossreturn, netreturn;
		float variance, stddev, vmr, profitchance, ev;
		Eigen::VectorXf probability;

		bool computed;

		size_t nSkins;
		Eigen::VectorXf mask;
		int level;
	};
}