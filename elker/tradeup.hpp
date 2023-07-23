#pragma once
#include "skin.hpp"
#include <iostream>
#pragma warning(push, 0)
#include <Eigen/Dense>
#pragma warning(pop)

namespace elker {
	class TradeUp;

	class Calculator {
	public:
		Calculator() {}
		Calculator(std::shared_ptr<SkinDB> db);
		std::shared_ptr<SkinDB> getDB() const { return m_DB; }

		std::string ExportTradeUp(TradeUp& tradeup);

		void Bruteforce();

		bool Compute(TradeUp &tradeup) const;
		void ComputeStatistical(TradeUp& tradeup) const;

		std::shared_ptr<SkinDB> m_DB;

	private:
		Eigen::VectorXf m_Prices[SkinCondition::Max];
		Eigen::VectorXf m_PricesWithFees[SkinCondition::Max];
		Eigen::VectorXf m_Factor[SkinCondition::Max];
		Eigen::MatrixXf m_Transformer[SkinCondition::Max];
	};

	class TradeUp {
	public:
		TradeUp(size_t n, SkinCondition cond) : nSkins(n), condition(cond), computed(false) {
			mask.resize(nSkins);
			cost = grossreturn = netreturn = variance = stddev = vmr = 0;

			Clear();
		}

		void Clear() {
			computed = false;
			cost = grossreturn = netreturn = variance = stddev = vmr = 0;
			for (int i = 0; i < nSkins; i++) {
				mask(i) = 0;
			}
		}
	public:
		float cost, grossreturn, netreturn;
		float variance, stddev, vmr;
		Eigen::VectorXf probability;

		bool computed;

		size_t nSkins;
		Eigen::VectorXf mask;
		SkinCondition condition;
	};
}