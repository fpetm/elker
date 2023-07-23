#include "tradeup.hpp"
#include <iostream>
#include <thread>
#include <functional>
#include <fstream>
#include "util.hpp"
#include "log.hpp"

// computation amount
#define L2
//#define L3

namespace elker {
	static size_t g_TradeUpCount;
	static std::mutex g_TradeUpCountMutex;

	static bool g_Finished = false;
	static std::mutex g_FinishedMutex;

	Calculator::Calculator(std::shared_ptr<SkinDB> db) : m_DB(db) {
		EK_INFO("Building calculator..");
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

		for (const Skin& skin : db->GetSkins()) {
			for (int level = 0; level < g_nLevels * 2; level++) {
				SkinCondition condition = ConditionFromFloat(g_Levels[level / 2], level & 1);
				SkinCondition mapped_condition = MapCondition(skin, g_Levels[level / 2], level & 1);

				m_Prices[level](skin.m_ID) = skin.m_PricesSell[condition];
				m_MappedPrices[level](skin.m_ID) = skin.m_PricesBuy[mapped_condition];
				m_MappedPricesWithFees[level](skin.m_ID) = skin.m_PricesBuy[mapped_condition] * 0.85f;
			}
		}

		for (int level = 0; level < g_nLevels * 2; level++) {
			auto& factor = m_Factor[level];
			auto& transformer = m_Transformer[level];
			for (SkinCollection& collection : db->GetCollections()) {
				for (SkinRarity rarity : {Consumer, Industrial, MilSpec, Restricted, Classified}) {
					SkinRarity higher = (SkinRarity)(rarity + 1);

					float higherc = 0;

					for (Skin skin : collection)
						if (skin.m_Rarity == higher)
							higherc += 1.0f;

					for (Skin& first : collection) {
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
		EK_INFO("Built calculator");
	}

	bool Calculator::Compute(TradeUp& tradeup) const {
		const float factor = tradeup.mask.dot(m_Factor[tradeup.level]);
		const Eigen::VectorXf probability = (m_Transformer[tradeup.level] * tradeup.mask) / tradeup.mask.dot(m_Factor[tradeup.level]);

		const float gross = probability.dot(m_MappedPricesWithFees[tradeup.level]);
		const float cost = tradeup.mask.dot(m_Prices[tradeup.level]);

		tradeup.probability = probability;
		tradeup.cost = cost;
		tradeup.grossreturn = gross;

		tradeup.computed = true;
		return (gross / cost) > 1.0f && cost < 15.0f;
	}

	void Calculator::ComputeStatistical(TradeUp& tradeup) const {
		const Eigen::VectorXf m2 = ((m_MappedPricesWithFees[tradeup.level].array() - tradeup.cost - tradeup.grossreturn) * (m_MappedPricesWithFees[tradeup.level].array() - tradeup.cost - tradeup.grossreturn)).matrix();
		tradeup.variance = tradeup.probability.dot(m2);
		tradeup.stddev = std::sqrt(tradeup.variance);
		tradeup.vmr = tradeup.variance / tradeup.grossreturn;
		tradeup.profitchance = (m_MappedPricesWithFees[tradeup.level].array() - tradeup.cost).array().cwiseSign().cwiseMax(0).matrix().dot(tradeup.probability);
		tradeup.ev = tradeup.grossreturn - tradeup.cost;
		tradeup.netreturn = tradeup.probability.dot(m_MappedPricesWithFees[tradeup.level]);
	}

	bool ValidateTradeUp(const TradeUp& tradeup) {
		if (tradeup.computed == false) return false;
		if (tradeup.cost <= 0) return false;

		return true;
	}

	std::string Calculator::ExportTradeUp(TradeUp& tradeup) {
		if (!ValidateTradeUp(tradeup)) return "";
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

		size_t total = 0;
		for (int i = 0; i < tradeup.nSkins; i++) {
			if (tradeup.mask(i) > 0.0f) {
				const int amount = static_cast<int>(tradeup.mask(i));
				for (int j = 0; j < amount; j++) {
					total++;
					const SkinCondition condition = ConditionFromFloat(g_Levels[tradeup.level / 2], tradeup.level & 1);
					ss << StringFromWeaponType(skins[i].m_WeaponType) << " | " << skins[i].m_Name << " ($" << skins[i].m_PricesBuy[condition] << "), ";
				}
			}
		}
		if (total != 10)
			EK_INFO("what");

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

	void ReportAmount() {
		bool finished = false;

		size_t lastCount = 0;

		while (!finished) {
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(1000ms);

			std::lock_guard<std::mutex> countguard(g_TradeUpCountMutex);
			size_t current = g_TradeUpCount;

			EK_INFO("  Searched total of {:15d} tradeups ({:6d} last second)", current, current - lastCount);
			lastCount = current;

			std::lock_guard<std::mutex> finishedguard(g_FinishedMutex);
			if (g_Finished) finished = true;
		}
	}

	void BruteforceCondition(const Calculator& calculator, int level, std::vector<TradeUp>& tradeups) {
		size_t nSkins = calculator.m_DB->m_Skins.size();
		const SkinCondition condition = ConditionFromFloat(g_Levels[level / 2], level & 1);

		TradeUp tradeup(nSkins, level);
		for (size_t id1 = 0; id1 < nSkins; id1++) {
			size_t l_TradeUpCount = 0;
			const Skin s1 = calculator.m_DB->m_Skins[id1];
			if (s1.m_Banned[condition]) continue;
			const SkinRarity rarity = s1.m_Rarity;

			tradeup.Clear();
			tradeup.mask(id1) = 10;
			l_TradeUpCount++;
			//tradeup.var = 1;

			if (calculator.Compute(tradeup)) tradeups.push_back(tradeup);

#ifdef L2
			for (size_t id2 = id1 + 1; id2 < nSkins; id2++) {
				const Skin s2 = calculator.m_DB->m_Skins[id2];

				if (s2.m_Banned[condition]) continue;
				if (s1.m_Rarity != s2.m_Rarity) continue;

				for (float v1 = 1.0f; v1 <= 9.0f; v1 += 1.0f) {
					const float A = v1;
					const float B = 10.0f - v1;
					tradeup.Clear();
					tradeup.mask(id1) = A;
					tradeup.mask(id2) = B;

					l_TradeUpCount++;
					if (calculator.Compute(tradeup)) tradeups.push_back(tradeup);

				}

#ifdef L3
				for (size_t id3 = id2 + 2; id3 < nSkins; id3++) {
					const Skin s3 = calculator.m_DB->m_Skins[id3];
					if (s3.m_Banned[condition]) continue;
					if (s3.m_Rarity != rarity) continue;
					for (float v1 = 1.0f; v1 <= 10.0f; v1 += 1.0f) {
						for (float v2 = 1.0f; v2 <= 10.0f; v2 += 1.0f) {
							const float A = v1;
							const float B = v2;
							const float C = 10.0f - v1 - v2;
							if (C <= 0) continue;

							tradeup.Clear();
							tradeup.mask(id1) = A;
							tradeup.mask(id2) = B;
							tradeup.mask(id3) = C;

							if (calculator.Compute(tradeup)) tradeups.push_back(tradeup);
						}
					}
				}
#endif

			std::lock_guard<std::mutex> guard(g_TradeUpCountMutex);
			g_TradeUpCount += l_TradeUpCount;
			}
#endif
		}
	}

	void Calculator::Bruteforce() {
		EK_INFO("Bruteforcing...");
		std::ofstream of("b:/out_now_outher.csv");
		of << "Cost,EV,GrossProfit$,GrossProfit%,NetProfit$,NetProfit%,Profit%,Variance,Standard Deviation,VMR,Wear,StatTrak,";
		for (int i = 0; i < 10; i++) of << "Weapon" << i + 1 << ",";
		for (int i = 0; i < 20; i++) of << "Result" << i + 1 << "," << "Price" << i + 1 << "," << "Chance" << i + 1 << ",";
		of << "\n";
		std::thread threads[g_nLevels * 2], reporter;
		std::vector<TradeUp> tradeups[g_nLevels * 2];


		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

		for (int level = 0; level < g_nLevels * 2; level++) {
			threads[level] = std::thread(BruteforceCondition, std::cref(*this), level, std::ref(tradeups[level]));
		}
		reporter = std::thread(ReportAmount);

		for (int level = 0; level < g_nLevels * 2; level++) {
			threads[level].join();
		}


		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

		double time = std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();

		{
			std::lock_guard<std::mutex> guard(g_FinishedMutex);
			g_Finished = true;
		}
		reporter.join();


		size_t tupc = 0;
		for (int level = 0; level < g_nLevels * 2; level++)
			tupc += tradeups[level].size();
		const size_t tupt = tupc;


		EK_INFO("Succesful bruteforcing, found {} profitable tradeups in {} seconds ({} tradeups/second)!", g_TradeUpCount, time, g_TradeUpCount / time);
		EK_INFO("Exporting...");
		for (int level = 0; level < g_nLevels * 2; level++) {
			for (TradeUp tradeup : tradeups[level]) {
				of << ExportTradeUp(tradeup);
				tupc--;
			}
			EK_INFO("  Succesfully exported {:3d}/{:3d} ({:4.2f}%) tradeups", tupt - tupc, tupt, (static_cast<float>(tupt - tupc) / static_cast<float>(tupt)) * 100.0f);
		}
		of.close();
		EK_INFO("Sucessfully exported all tradeups");

	}
}
