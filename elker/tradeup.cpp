#include "tradeup.hpp"
#include <iostream>
#include <thread>
#include <functional>
#include <fstream>
#include "util.hpp"
#include "log.hpp"
#include "partitions.hpp"

// computation amount
#define L2
//#define L3

#define MT

namespace elker {
	static size_t g_TradeUpCount;
	static std::mutex g_TradeUpCountMutex;

	static bool g_Finished = false;
	static std::mutex g_FinishedMutex;

	Calculator::Calculator(std::shared_ptr<SkinDB> db) : m_DB(db) {
		EK_INFO("Building calculator..");
		
		for (SkinRarity rarity : {SkinRarity::Consumer, SkinRarity::Industrial, SkinRarity::MilSpec, SkinRarity::Restricted, SkinRarity::Classified, SkinRarity::Covert}) {
			const auto ids = m_DB->m_SkinIDsByRarity[rarity];
			const auto hids = m_DB->m_SkinIDsByRarity[rarity + 1];
			const size_t skinc = ids.size();
			const size_t higherc = hids.size();
			
			

			for (int level = 0; level < g_nLevels * 2; level++) {
				m_Prices[rarity][level].resize(skinc);
				m_MappedPrices[rarity][level].resize(skinc);
				m_MappedPricesWithFees[rarity][level].resize(skinc);
				m_Factor[rarity][level].resize(skinc);
				m_Transformer[rarity][level].resize(higherc, skinc);
			}

			if (rarity == SkinRarity::Covert) continue;
			for (int level = 0; level < g_nLevels * 2; level++) {
				for (const size_t id : ids) {
					const Skin skin = m_DB->m_Skins[id];
					SkinCondition condition = ConditionFromFloat(g_Levels[level / 2], level & 1);
					SkinCondition mapped_condition = MapCondition(skin, g_Levels[level / 2], level & 1);

					m_Prices[rarity][level].insertBack(skin.m_rID) = skin.m_PricesBuy[condition];
					m_MappedPrices[rarity][level].insertBack(skin.m_rID) = skin.m_PricesSell[mapped_condition];
					m_MappedPricesWithFees[rarity][level].insertBack(skin.m_rID) = skin.m_PricesSell[mapped_condition] * 0.85f - 0.01f;
				}
			}

			for (int level = 0; level < g_nLevels * 2; level++) {
				std::vector<Eigen::Triplet<float>> transformer_triplets;
				auto& factor = m_Factor[rarity][level];
				auto& transformer = m_Transformer[rarity][level];


				for (const SkinCollection& collection : db->m_Collections) {
					float higherc = collection.m_SkinsByRarity[rarity + 1].size();

					for (const Skin& first : collection.m_SkinsByRarity[rarity]) {
						factor.insertBack(first.m_rID) = higherc;
						for (const Skin& second : collection.m_SkinsByRarity[rarity + 1]) {
							transformer_triplets.push_back({ (int) second.m_rID,(int)first.m_rID , 1 });
						}
					}
				}
				transformer.setFromTriplets(transformer_triplets.begin(), transformer_triplets.end());
				transformer.makeCompressed();
			}
		}
		EK_INFO("Built calculator");
	}

	bool Calculator::Compute(TradeUp& tradeup) const {
		const float factor = tradeup.mask.dot(m_Factor[tradeup.rarity][tradeup.level]);
		const Eigen::SparseVector<float> probability = (m_Transformer[tradeup.rarity][tradeup.level] * tradeup.mask) / tradeup.mask.dot(m_Factor[tradeup.rarity][tradeup.level]);

		const float gross = probability.dot(m_MappedPricesWithFees[tradeup.rarity+1][tradeup.level]);
		const float cost = tradeup.mask.dot(m_Prices[tradeup.rarity][tradeup.level]);

		return (gross / cost) > 1.2f && cost < 20.0f;
	}

	void Calculator::ComputeStatistical(TradeUp& tradeup) const {
		const float factor = tradeup.mask.dot(m_Factor[tradeup.rarity][tradeup.level]);
		const Eigen::SparseVector<float> probability = (m_Transformer[tradeup.rarity][tradeup.level] * tradeup.mask) / tradeup.mask.dot(m_Factor[tradeup.rarity][tradeup.level]);

		const float gross = probability.dot(m_MappedPricesWithFees[tradeup.rarity + 1][tradeup.level]);
		const float cost = tradeup.mask.dot(m_Prices[tradeup.rarity][tradeup.level]);

		tradeup.probability = probability;
		tradeup.cost = cost;
		tradeup.grossreturn = gross;
		tradeup.ev = tradeup.grossreturn - tradeup.cost;
		tradeup.netreturn = tradeup.probability.dot(m_MappedPricesWithFees[tradeup.rarity][tradeup.level]);
#if 0
		const Eigen::VectorXf m2 = ((m_MappedPricesWithFees[tradeup.rarity][tradeup.level].array() - tradeup.cost - tradeup.grossreturn) * (m_MappedPricesWithFees[tradeup.rarity][tradeup.level].array() - tradeup.cost - tradeup.grossreturn)).matrix();
		tradeup.variance = tradeup.probability.dot(m2);
		tradeup.stddev = std::sqrt(tradeup.variance);
		tradeup.vmr = tradeup.variance / tradeup.grossreturn;
		tradeup.profitchance = (m_MappedPricesWithFees[tradeup.rarity][tradeup.level].array() - tradeup.cost).array().cwiseSign().cwiseMax(0).matrix().dot(tradeup.probability);
#endif
	}

	bool ValidateTradeUp(const TradeUp& tradeup) {
		if (tradeup.computed == false) return false;
		if (tradeup.cost <= 0) return false;

		return true;
	}

	std::string Calculator::ExportTradeUp(TradeUp& tradeup) {
		ComputeStatistical(tradeup);
		std::stringstream ss;
		const std::vector<Skin> skins = m_DB->GetSkins();
		const std::vector<size_t> ids_by_rarity = m_DB->m_SkinIDsByRarity[tradeup.rarity];
		const std::vector<size_t> hids_by_rarity = m_DB->m_SkinIDsByRarity[tradeup.rarity + 1];

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

		// TODO this is extremely slow
		const Eigen::VectorXf mask = tradeup.mask.toDense();
		const Eigen::VectorXf probabilities = tradeup.mask.toDense();

		for (int i = 0; i < ids_by_rarity.size(); i++) {
			if (mask(i) > 0.0f) {
				size_t id = ids_by_rarity[i];
				const int amount = static_cast<int>(mask(i));
				for (int j = 0; j < amount; j++) {
					const SkinCondition condition = ConditionFromFloat(g_Levels[tradeup.level / 2], tradeup.level & 1);
					ss << StringFromWeaponType(skins[id].m_WeaponType) << " | " << skins[id].m_Name << " ($" << skins[id].m_PricesBuy[condition] << "), ";
				}
			}
		}

		std::vector<Skin> outskins;
		std::vector<float> chances;
		for (int i = 0; i < hids_by_rarity.size(); i++) {
			if (probabilities(i) > 0.0f) {
				size_t id = hids_by_rarity[i];
				const SkinCondition condition = MapCondition(skins[id], g_Levels[tradeup.level / 2], tradeup.level & 1);
				ss << StringFromWeaponType(skins[id].m_WeaponType) << " | " << skins[id].m_Name << " (" << StringFromWeaponCondition(condition) << ")," << std::to_string(skins[id].m_PricesSell[condition]) << "," << std::to_string(probabilities(i)) << ",";
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

			EK_INFO("  Searched total of {:15d} tradeups ({:8d} last second)", current, current - lastCount);
			lastCount = current;

			std::lock_guard<std::mutex> finishedguard(g_FinishedMutex);
			if (g_Finished) finished = true;
		}
	}

	void BruteforceCondition(const Calculator& calculator, int level, std::vector<TradeUp>& tradeups) {
		size_t nSkins = calculator.m_DB->m_Skins.size();
		const SkinCondition condition = ConditionFromFloat(g_Levels[level / 2], level & 1);

		std::vector<size_t> ids_by_rarity[SkinRarity::Contraband + 1];

		for (SkinRarity rarity : {SkinRarity::Consumer, SkinRarity::Industrial, SkinRarity::MilSpec, SkinRarity::Restricted, SkinRarity::Classified}) {
			for (size_t id = 0; id < calculator.m_DB->m_SkinIDsByRarity[rarity].size(); id++) {
				if (calculator.m_DB->m_Skins[calculator.m_DB->m_SkinIDsByRarity[rarity][id]].m_Banned[condition] == false) {
					ids_by_rarity[rarity].push_back(id);
				}
			}
		}

		for (SkinRarity rarity : {SkinRarity::Consumer, SkinRarity::Industrial, SkinRarity::MilSpec, SkinRarity::Restricted, SkinRarity::Classified}) {
			size_t nRSkins = calculator.m_DB->m_SkinIDsByRarity[rarity].size();
			for (size_t id1 : ids_by_rarity[rarity]) {
				TradeUp tradeup(nRSkins, level, rarity);
				size_t l_TradeUpCount = 0;

				tradeup.mask.insert(id1) = 10;
				l_TradeUpCount++;
				if (calculator.Compute(tradeup)) tradeups.push_back(tradeup);
				tradeup.mask.insert(id1) = 0;

#ifdef L2
				for (size_t id2 : ids_by_rarity[rarity]) {
					if (id2 == id1) continue;
					TradeUp tradeup(nRSkins, level, rarity);
					for (const auto p : p2) {
						const float A = p[0];
						const float B = p[1];

						if (A + B != 10.0f)
							std::cout << A << " " << B << "\n";
						tradeup.A = A;
						tradeup.B = B;
						tradeup.id1 = id1;
						tradeup.id2 = id2;

						tradeup.mask.insert(id1) = A;
						tradeup.mask.insert(id2) = B;

						l_TradeUpCount++;
						if (calculator.Compute(tradeup)) tradeups.push_back(tradeup);

					}

#ifdef L3
					for (size_t id3 : ids_by_rarity[rarity]) {
						if (id3 == id2 || id3 == id1) continue;
						TradeUp tradeup(nRSkins, level, rarity);
						for (float v1 = 1.0f; v1 <= 10.0f; v1 += 1.0f) {
							for (float v2 = 1.0f; v2 <= 10.0f; v2 += 1.0f) {
								if (v1 + v2 >= 10) continue;
								const float A = v1;
								const float B = v2;
								const float C = 10.0f - v1 - v2;

								tradeup.mask.insert(id1) = A;
								tradeup.mask.insert(id2) = B;
								tradeup.mask.insert(id3) = C;

								if (calculator.Compute(tradeup)) tradeups.push_back(tradeup);

							}
						}
						tradeup.mask.insert(id1) = 0;
						tradeup.mask.insert(id2) = 0;
						tradeup.mask.insert(id3) = 0;

						std::lock_guard<std::mutex> guard(g_TradeUpCountMutex);
						g_TradeUpCount += l_TradeUpCount;
					}
#else

					std::lock_guard<std::mutex> guard(g_TradeUpCountMutex);
					g_TradeUpCount += l_TradeUpCount;
#endif
				}
#endif
			}
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
#ifdef MT
		for (int level = 0; level < g_nLevels * 2; level++) {
			threads[level] = std::thread(BruteforceCondition, std::cref(*this), level, std::ref(tradeups[level]));
		}

		reporter = std::thread(ReportAmount);
		for (int level = 0; level < g_nLevels * 2; level++) {
			threads[level].join();
		}
#else
		reporter = std::thread(ReportAmount);
		for (int level = 0; level < g_nLevels * 2; level++) {
			BruteforceCondition(std::cref(*this), level, std::ref(tradeups[level]));
		}
#endif

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

		EK_INFO("Succesful bruteforcing, serched {}, found {} profitable tradeups in {} seconds ({:.2f} tradeups/second)!", g_TradeUpCount, tupt, time, g_TradeUpCount / time);
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
