#pragma once
#include "skin.hpp"
#include <Eigen/Dense>

namespace elker {
	class TradeUp {
	public:
		Eigen::VectorXf mask;
		SkinCondition condition;
	};

	class Calculator {
	public:
		Calculator(std::shared_ptr<SkinDB> db);

		float ExpectedValue(TradeUp& tradeup);
		float Profit(TradeUp& tradeup);

		void Bruteforce();
	private:
		std::shared_ptr<SkinDB> m_DB;

		Eigen::VectorXf m_Prices[SkinCondition::Max];
		Eigen::VectorXf m_PricesWithFees[SkinCondition::Max];
		Eigen::VectorXf m_Factor[SkinCondition::Max];
		Eigen::MatrixXf m_Transformer[SkinCondition::Max];
	};
}