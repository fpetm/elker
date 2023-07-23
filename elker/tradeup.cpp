#include "tradeup.hpp"
#include <iostream>

namespace elker {
	Calculator::Calculator(std::shared_ptr<SkinDB> db) {
		unsigned int counts[SkinCondition::Max] = { 0 };

		const unsigned int skinc = db->GetSkins().size();
		for (SkinCondition condition : {BS, WW, FT, MW, FN, BS_ST, WW_ST, FT_ST, MW_ST, FN_ST}) {
			m_Prices[condition].resize(skinc);
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
			m_Prices[skin.m_Condition](skin.m_ID) = skin.m_Price;

			counts[skin.m_Condition]++;
		}

		for (SkinCollection &collection : db->GetCollections()) {
			for (SkinCondition condition : {BS, WW, FT, MW, FN, BS_ST, WW_ST, FT_ST, MW_ST, FN_ST}) {
				auto& factor = m_Factor[condition];
				auto& transformer = m_Transformer[condition];


				for (SkinRarity rarity : {Consumer, Industrial, MilSpec, Restricted, Classified}) {
					SkinRarity higher = (SkinRarity)(rarity + 1);

					float higherc = 0;

					for (Skin skin : collection)
						if (skin.m_Rarity == higher && skin.m_Condition == condition)
							higherc += 1.0f;

					for (Skin &first : collection) {
						if (first.m_Rarity == rarity && first.m_Condition == condition) {
							factor(first.m_ID) = higherc;
							for (Skin& second : collection) {
								if (second.m_Rarity == higher && second.m_Condition == condition) {
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
		const Eigen::VectorXf transformed = m_Transformer[tradeup.condition] * tradeup.mask;
		const float factor = tradeup.mask.dot(m_Factor[tradeup.condition]);
		const float returnprice = (transformed / factor).dot(m_Prices[tradeup.condition]);
		const float tradeupcost = tradeup.mask.dot(m_Prices[tradeup.condition]);
		return returnprice / tradeupcost;
	}
}