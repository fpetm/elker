#include "tradeup.hpp"
#include <iostream>

namespace elker {
	Calculator::Calculator(std::shared_ptr<SkinDB> db) : m_DB(db) {
		const unsigned int skinc = db->GetSkins().size();
		for (SkinCondition condition : {BS, WW, FT, MW, FN, BS_ST, WW_ST, FT_ST, MW_ST, FN_ST}) {
			m_Prices[condition].resize(skinc);
			m_PricesWithFees[condition].resize(skinc);
			m_Factor[condition].resize(skinc);
			m_Transformer[condition].resize(skinc, skinc);

			for (unsigned int i = 0; i < skinc; i++) {
				m_Prices[condition](i) = 0;
				m_Factor[condition](i) = 0;

				for (unsigned int j = 0; j < skinc; j++) {
					m_Transformer[condition](i, j) = 0;
				}
			}
		}

		for (Skin &skin : db->GetSkins()) {
			for (SkinCondition condition : {BS, WW, FT, MW, FN, BS_ST, WW_ST, FT_ST, MW_ST, FN_ST}) {
				m_Prices[condition](skin.m_ID) = skin.m_Prices[condition];
				m_PricesWithFees[condition](skin.m_ID) = skin.m_Prices[condition] * 0.85f;
			}
		}

		for (SkinCollection &collection : db->GetCollections()) {
			for (SkinCondition condition : {BS, WW, FT, MW, FN, BS_ST, WW_ST, FT_ST, MW_ST, FN_ST}) {
				auto& factor = m_Factor[condition];
				auto& transformer = m_Transformer[condition];


				for (SkinRarity rarity : {Consumer, Industrial, MilSpec, Restricted, Classified}) {
					SkinRarity higher = (SkinRarity)(rarity + 1);

					float higherc = 0;

					for (Skin skin : collection)
						if (skin.m_Rarity == higher)
							higherc += 1.0f;

					for (Skin &first : collection) {
						if (first.m_Rarity == rarity) {
							factor(first.m_ID) = higherc;
							for (Skin& second : collection) {
								if (second.m_Rarity == higher) {
									transformer(second.m_ID, first.m_ID) = 1;
								}
							}
						}
					}
				}
			}
		}


	}

	float Calculator::ExpectedValue(TradeUp& tradeup) {
		const Eigen::VectorXf probability = (m_Transformer[tradeup.condition] * tradeup.mask)/tradeup.mask.dot(m_Factor[tradeup.condition]);

		const float returnprice = probability.dot(m_PricesWithFees[tradeup.condition]);
		const float tradeupcost = tradeup.mask.dot(m_Prices[tradeup.condition]);
		return returnprice / tradeupcost;
	}

	float Calculator::Profit(TradeUp& tradeup) {
		const Eigen::VectorXf probability = (m_Transformer[tradeup.condition] * tradeup.mask) / tradeup.mask.dot(m_Factor[tradeup.condition]);
		const float returnprice = probability.dot(m_PricesWithFees[tradeup.condition]);
		const float tradeupcost = tradeup.mask.dot(m_Prices[tradeup.condition]);
		return returnprice - tradeupcost;
	}

	void Calculator::Bruteforce() {
		for (SkinCondition condition : {BS, WW, FT, MW, FN, BS_ST, WW_ST, FT_ST, MW_ST, FN_ST}) {
			for (Skin skin : m_DB->GetSkins()) {
				if (skin.m_Rarity == Classified) continue;
				TradeUp tradeup;
				tradeup.mask.resize(34);
				for (int i = 0; i < 34; i++) {
					tradeup.mask(i) = 0;
				}
				tradeup.mask(skin.m_ID) = 10;
				tradeup.condition = condition;

				float m = ExpectedValue(tradeup);
				if (m > 1) {
					std::cout << "10x" << StringFromWeaponType(skin.m_WeaponType) << " | " << skin.m_Name << " (" << StringFromWeaponCondition(condition) << ") " << " -> " << Profit(tradeup) << " eur profit" << std::endl;
				}
			}
		}
	}
}