#include "tradeup.hpp"
#include <iostream>
#include <thread>
#include <functional>
#include <fstream>
#include "util.hpp"
#include "log.hpp"

namespace elker {


	Calculator::Calculator(std::shared_ptr<SkinDB> db) : m_DB(db) {
		const size_t skinc = db->GetSkins().size();
		for (int level = 0; level < g_nLevels * 2; level++) {
			m_Prices[level].resize(skinc);
			m_MappedPrices[level].resize(skinc);
			m_MappedPricesWithFees[level].resize(skinc);
			m_Factor[level].resize(skinc);
			m_Transformer[level].resize(skinc, skinc);

			for (unsigned int i = 0; i < skinc; i++) {
				m_Prices[level](i) = 0;
				m_MappedPrices[level](i) = 0;
				m_Prices[level](i) = 0;
				m_MappedPricesWithFees[level](i) = 0;

				for (unsigned int j = 0; j < skinc; j++) {
					m_Transformer[level](i, j) = 0;
				}
			}
		}

		for (const Skin &skin : db->GetSkins()) {
			for (int level = 0; level < g_nLevels * 2; level++) {
				SkinCondition condition = ConditionFromFloat(g_Levels[level/2], level & 1);
				SkinCondition mapped_condition = MapCondition(skin, g_Levels[level/2], level&1);

				m_Prices[level](skin.m_ID) = skin.m_PricesSell[condition];
				m_MappedPrices[level](skin.m_ID) = skin.m_PricesBuy[mapped_condition];
				m_MappedPricesWithFees[level](skin.m_ID) = skin.m_PricesBuy[mapped_condition] * 0.85f;
			}
		}

		for (int level = 0; level < g_nLevels * 2; level++) {
			auto& factor = m_Factor[level];
			auto& transformer = m_Transformer[level];
			for (SkinCollection &collection : db->GetCollections()) {
				for (SkinRarity rarity : {Consumer, Industrial, MilSpec, Restricted, Classified}) {
					SkinRarity higher = (SkinRarity)(rarity + 1);

					float higherc = 0;

					for (Skin skin : collection)
						if (skin.m_Rarity == higher)
							higherc += 1.0f;
					
					for (Skin &first : collection) {
						if (first.m_Rarity >= collection.m_HighestRarity || first.m_Rarity != rarity) continue;
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

	bool Calculator::Compute(TradeUp& tradeup) const {
		const float factor = tradeup.mask.dot(m_Factor[tradeup.level]);
		const Eigen::VectorXf probability = (m_Transformer[tradeup.level] * tradeup.mask) / tradeup.mask.dot(m_Factor[tradeup.level]);

		const float gross = probability.dot(m_MappedPricesWithFees[tradeup.level]);
		const float net = probability.dot(m_MappedPricesWithFees[tradeup.level]);
		const float cost = tradeup.mask.dot(m_Prices[tradeup.level]);

		tradeup.probability = probability;
		tradeup.cost = cost;
		tradeup.grossreturn = gross;
		tradeup.netreturn = net;

		tradeup.computed = true;

		return gross > cost;
	}

	void Calculator::ComputeStatistical(TradeUp& tradeup) const {
		const Eigen::VectorXf m2 = ((m_MappedPricesWithFees[tradeup.level].array() - tradeup.cost - tradeup.grossreturn) * (m_MappedPricesWithFees[tradeup.level].array() - tradeup.cost - tradeup.grossreturn)).matrix();
		tradeup.variance = tradeup.probability.dot(m2);
		tradeup.stddev = std::sqrt(tradeup.variance);
		tradeup.vmr = tradeup.variance / tradeup.grossreturn;
		tradeup.profitchance = (m_MappedPricesWithFees[tradeup.level].array() - tradeup.cost).array().cwiseSign().cwiseMax(0).matrix().dot(tradeup.probability);
		tradeup.ev = tradeup.grossreturn - tradeup.cost;
	}

	std::string Calculator::ExportTradeUp(TradeUp& tradeup) {
		if (tradeup.computed == false) return "";
		ComputeStatistical(tradeup);
		std::stringstream ss;
		const std::vector<Skin> skins = m_DB->GetSkins();

		ss << tradeup.cost << ",";
		ss << tradeup.ev << ",";
		ss << tradeup.grossreturn - tradeup.cost << ",";
		ss << (tradeup.grossreturn / tradeup.cost - 1.0f) * 100.0f << ",";
		ss << tradeup.netreturn - tradeup.cost << ",";
		ss << (tradeup.netreturn / tradeup.cost - 1.0f) * 100.0f << ",";
		ss << tradeup.profitchance * 100.0f << ",";
		ss << tradeup.variance << ",";
		ss << tradeup.stddev << ",";
		ss << tradeup.vmr << ",";

		ss << g_Levels[tradeup.level / 2] << ",";
		ss << std::to_string(tradeup.level & 1) << ",";
		
		for (int i = 0; i < tradeup.nSkins; i++) {
			if (tradeup.mask(i) > 0.0f) {
				const int amount = static_cast<int>(tradeup.mask(i));
				for (int j = 0; j < amount; j++)
					ss << StringFromWeaponType(skins[i].m_WeaponType) << " | " << skins[i].m_Name << ",";
			}
		}

		std::vector<Skin> outskins;
		std::vector<float> chances;
		for (int i = 0; i < tradeup.nSkins; i++) {
			if (tradeup.probability(i) > 0.0f) {
				const SkinCondition condition = MapCondition(skins[i], g_Levels[tradeup.level / 2], tradeup.level & 1);
				ss << StringFromWeaponType(skins[i].m_WeaponType) << " | " << skins[i].m_Name << "," << std::to_string(skins[i].m_PricesSell[condition]) << "," << std::to_string(tradeup.probability(i)) << ",";
			}
		}

		ss << "\n";
		return ss.str();
	}

#define L2
//#define L3

	void BruteforceCondition(const Calculator &calculator, int level, std::vector<TradeUp> &tradeups) {
		size_t nSkins = calculator.m_DB->m_Skins.size();
		const SkinCondition condition = ConditionFromFloat(g_Levels[level / 2], level&1);

		TradeUp tradeup(nSkins, level);
		for (size_t id1 = 0; id1 < nSkins; id1++) {		
			const Skin s1 = calculator.m_DB->m_Skins[id1];
			if (s1.m_Banned[condition]) continue;
			const SkinRarity rarity = s1.m_Rarity;

			tradeup.mask(id1) = 10;

			if (calculator.Compute(tradeup)) tradeups.push_back(tradeup);

			tradeup.Clear();
#ifdef L2
			for (size_t id2 = id1+1; id2 < nSkins; id2++) {
				const Skin s2 = calculator.m_DB->m_Skins[id2];

				if (s2.m_Banned[condition]) continue;
				if (s1.m_Rarity != s2.m_Rarity) continue;

				for (float v1 = 1.0f; v1 <= 9.0f; v1 += 1.0f) {
					const float A = v1;
					const float B = 10.0f - v1;
					tradeup.mask(id1) = A;
					tradeup.mask(id2) = B;

					if (calculator.Compute(tradeup)) tradeups.push_back(tradeup);
					tradeup.Clear();
				}

#ifdef L3
				for (size_t id3 = id1 + 1; id3 < nSkins; id3++) {
					const Skin s3 = calculator.m_DB->m_Skins[id3];
					if (s3.m_Banned[condition]) continue;
					if (s3.m_Rarity != rarity) continue;
					for (float v1 = 1.0f; v1 <= 8.0f; v1 += 1.0f) {
						for (float v2 = v1 + 1.0f; v2 <= 9.0f; v2 += 1.0f) {
							const float A = v1;
							const float B = v2;
							const float C = 10.0f - v2 - v2;
							tradeup.mask(id1) = A;
							tradeup.mask(id2) = B;
							tradeup.mask(id3) = C;

							if (calculator.Compute(tradeup)) tradeups.push_back(tradeup);
							tradeup.Clear();
						}
					}
				}
#endif
			}
#endif
		}
	}

	void Calculator::Bruteforce() {
		EK_INFO("Bruteforcing...");
		std::ofstream of("b:/out.csv");
		of << "Cost,EV,GrossProfit$,GrossProfit%,NetProfit$,NetProfit%,Profit%,Variance,Standard Deviation,VMR,Wear,StatTrak,";
		for (int i = 0; i < 10; i++) of << "Weapon" << i + 1 << ",";
		for (int i = 0; i < 20; i++) of << "Result" << i + 1 << "," << "Price" << i+1 << "," << "Chance" << i + 1 << ",";
		of << "\n";
		std::thread threads[g_nLevels*2];
		std::vector<TradeUp> tradeups[g_nLevels * 2];
		for (int level = 0; level < g_nLevels * 2; level++) {
			threads[level] = std::thread(BruteforceCondition, std::cref(*this), level, std::ref(tradeups[level]));
		}
		for (int level = 0; level < g_nLevels * 2; level++) {
			threads[level].join();
		}

		size_t tupc = 0;
		for (int level = 0; level < g_nLevels * 2; level++)
			tupc += tradeups[level].size();

		EK_INFO("Succesful bruteforcing, found {} profitable tradeups!", tupc);

		for (int level = 0; level < g_nLevels * 2; level++) {
			for (TradeUp tradeup : tradeups[level]) {
				of << ExportTradeUp(tradeup);
			}
		}
		of.close();

	}
}