#include "tradeup.hpp"
#include <iostream>
#include <thread>
#include <functional>
#include <fstream>
#include "util.hpp"

namespace elker {
	Calculator::Calculator(std::shared_ptr<SkinDB> db) : m_DB(db) {
		const size_t skinc = db->GetSkins().size();
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

		for (SkinCondition condition : {BS, WW, FT, MW, FN, BS_ST, WW_ST, FT_ST, MW_ST, FN_ST}) {
			auto& factor = m_Factor[condition];
			auto& transformer = m_Transformer[condition];
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
		const float factor = tradeup.mask.dot(m_Factor[tradeup.condition]);
		const Eigen::VectorXf probability = (m_Transformer[tradeup.condition] * tradeup.mask) / tradeup.mask.dot(m_Factor[tradeup.condition]);

		const float gross = probability.dot(m_PricesWithFees[tradeup.condition]);
		const float net = probability.dot(m_PricesWithFees[tradeup.condition]);
		const float cost = tradeup.mask.dot(m_Prices[tradeup.condition]);

		tradeup.probability = probability;
		tradeup.cost = cost;
		tradeup.grossreturn = gross;
		tradeup.netreturn = net;

		tradeup.computed = true;

		return gross > cost;
	}

	std::string Calculator::ExportTradeUp(TradeUp& tradeup) {
		if (tradeup.computed == false) return "";
		std::stringstream ss;
		std::vector<Skin> skins;
		std::vector<int> amounts;
		
		for (int i = 0; i < tradeup.nSkins; i++) {
			if (tradeup.mask(i) > 0.0f) {
				if (m_DB->GetSkins()[i].m_Rarity >= m_DB->GetCollections()[m_DB->GetSkins()[i].m_CollectionID].m_HighestRarity) return "";
				if (m_DB->GetSkins()[i].m_Prices[tradeup.condition] < 0) return "";
				skins.push_back(m_DB->GetSkins().at(i));
				amounts.push_back(static_cast<int>(tradeup.mask(i)));
			}
		}
		ss << tradeup.cost << ",";
		ss << tradeup.grossreturn - tradeup.cost << ",";
		ss << (tradeup.grossreturn / tradeup.cost - 1.0f) * 100.0f << ",";
		ss << tradeup.netreturn - tradeup.cost << ",";
		ss << (tradeup.netreturn / tradeup.cost - 1.0f) * 100.0f << ",";

		ss << StringFromWeaponCondition(tradeup.condition) << ",";


		for (int i = 0; i < skins.size(); i++) {
			for (int j = 0; j < amounts[i]; j++) {
				ss << StringFromWeaponType(skins[i].m_WeaponType) << " | " << skins[i].m_Name << ",";
			}
			//std::cout << amounts[i] << "x " << StringFromWeaponType(skins[i].m_WeaponType) << " | " << skins[i].m_Name << " (" << StringFromWeaponCondition(tradeup.condition) << ") + ";
		}
		const Eigen::VectorXf probability = (m_Transformer[tradeup.condition] * tradeup.mask) / tradeup.mask.dot(m_Factor[tradeup.condition]);

		std::vector<Skin> outskins;
		std::vector<float> chances;
		for (int i = 0; i < tradeup.nSkins; i++) {
			if (probability(i) > 0.0f) {
				ss << StringFromWeaponType(m_DB->GetSkins()[i].m_WeaponType) << " | " << m_DB->GetSkins()[i].m_Name << "," << std::to_string(m_DB->GetSkins()[i].m_Prices[tradeup.condition]) << "," << std::to_string(probability(i)) << ",";
				outskins.push_back(m_DB->GetSkins().at(i));
				chances.push_back(probability(i));
			}
		}

		ss << "\n";
		return ss.str();
	}

#define L2
//#define L3

	void BruteforceCondition(const Calculator &calculator, SkinCondition condition, std::vector<TradeUp> &tradeups) {
		size_t nSkins = calculator.m_DB->m_Skins.size();

		TradeUp tradeup(nSkins, condition);
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

#if 0 // the new method
				for (int i = 0; i < 10 * 10; i++) {
					const float A = i / 10;
					const float B = i % 10;
					if (B < A) continue;

					tradeup.mask(id1) = A;
					tradeup.mask(id2) = B;

					if (calculator.Compute(tradeup)) tradeups.push_back(tradeup);
					tradeup.Clear();
				}
#endif

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
		std::ofstream of("b:/out.csv");
		of << "Cost,GrossProfit$,GrossProfit%,NetProfit$,NetProfit%,Condition,";
		for (int i = 0; i < 10; i++) of << "Weapon" << i + 1 << ",";
		for (int i = 0; i < 20; i++) of << "Result" << i + 1 << "," << "Price" << i+1 << "," << "Chance" << i + 1 << ",";
		of << "\n";
		std::thread threads[SkinCondition::Max];
		std::vector<TradeUp> tradeups[SkinCondition::Max];
		for (SkinCondition condition : {BS, WW, FT, MW, FN, BS_ST, WW_ST, FT_ST, MW_ST, FN_ST}) {
			threads[condition] = std::thread(BruteforceCondition, std::cref(*this), condition, std::ref(tradeups[condition]));
		}
		for (SkinCondition condition : {BS, WW, FT, MW, FN, BS_ST, WW_ST, FT_ST, MW_ST, FN_ST}) {
			threads[condition].join();
		}
		for (SkinCondition condition : {BS, WW, FT, MW, FN, BS_ST, WW_ST, FT_ST, MW_ST, FN_ST}) {
			for (TradeUp tradeup : tradeups[condition]) {
				of << ExportTradeUp(tradeup);
			}
		}
		of.close();

	}
}