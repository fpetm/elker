#include <motek/tradeup.hpp>
#include <iostream>
#include <thread>
#include <functional>
#include <fstream>
#include <bitset>

#include <motek/log.hpp>

#include "base64.hpp"
#include "partitions.hpp"

// computation amount
#define L2
#define L3
//#define L4

#define MT

namespace motek {
	static size_t g_TradeUpCount;
	static std::mutex g_TradeUpCountMutex;

	static bool g_Finished = false;
	static std::mutex g_FinishedMutex;

	Calculator::Calculator(std::shared_ptr<SkinDB> db) : m_DB(db) {
		MT_INFO("Building calculator..");
		
		for (SkinRarity rarity : {SkinRarity::Consumer, SkinRarity::Industrial, SkinRarity::MilSpec, SkinRarity::Restricted, SkinRarity::Classified, SkinRarity::Covert}) {
			const std::vector<size_t> &ids = m_DB->m_SkinIDsByRarity[rarity];
			const std::vector<size_t>& hids = m_DB->m_SkinIDsByRarity[rarity + 1];
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

					m_Prices[rarity][level].insertBack(skin.m_rID) = skin.m_PricesSell[condition];
					m_MappedPrices[rarity][level].insertBack(skin.m_rID) = skin.m_PricesBuy[mapped_condition];
					m_MappedPricesWithFees[rarity][level].insertBack(skin.m_rID) = skin.m_PricesBuy[mapped_condition] * 0.87f - 0.01f;
				}
			}

			for (int level = 0; level < g_nLevels * 2; level++) {
				std::vector<Eigen::Triplet<float>> transformer_triplets;

				auto& factor = m_Factor[rarity][level];
				auto& transformer = m_Transformer[rarity][level];

				for (const SkinCollection& collection : db->m_Collections) {
					float hc = collection.m_SkinsByRarity[rarity + 1].size();

					for (const Skin& first : collection.m_SkinsByRarity[rarity]) {
						factor.insertBack(first.m_rID) = hc;
						for (const Skin& second : collection.m_SkinsByRarity[rarity + 1]) {
							transformer_triplets.push_back({ static_cast<int>(second.m_rID), static_cast<int>(first.m_rID), 1 });
						}
					}
				}
				transformer.setFromTriplets(transformer_triplets.begin(), transformer_triplets.end());
				transformer.makeCompressed();
			}
		}
		MT_INFO("Built calculator");
	}

	bool Calculator::Compute(TradeUp& tradeup) const {
		//const float factor = tradeup.mask.dot(m_Factor[tradeup.rarity][tradeup.level]);
		//const Eigen::SparseVector<float> probability =;;

		const float gross = ((m_Transformer[tradeup.rarity][tradeup.level] * tradeup.mask) / tradeup.mask.dot(m_Factor[tradeup.rarity][tradeup.level])).dot(m_MappedPricesWithFees[tradeup.rarity + 1][tradeup.level]);
		const float cost = tradeup.mask.dot(m_Prices[tradeup.rarity][tradeup.level]);

		return gross > cost;
	}

	void Calculator::ComputeStatistical(TradeUp& tradeup) const {
		// TODO WHY THE FUCK IS THIS NOT FUCKING WORKING
		const float factor = tradeup.mask.dot(m_Factor[tradeup.rarity][tradeup.level]);
		const Eigen::SparseVector<float> probability = (m_Transformer[tradeup.rarity][tradeup.level] * tradeup.mask) / factor;

		const float gross = probability.dot(m_MappedPricesWithFees[tradeup.rarity + 1][tradeup.level]);
		const float cost = tradeup.mask.dot(m_Prices[tradeup.rarity][tradeup.level]);

		tradeup.probability = probability;
		tradeup.cost = cost;
		tradeup.grossreturn = gross;
		tradeup.ev = tradeup.grossreturn - tradeup.cost;
		tradeup.netreturn = tradeup.probability.dot(m_MappedPrices[tradeup.rarity+1][tradeup.level]);

		//const Eigen::VectorXf m2 = ((m_MappedPricesWithFees[tradeup.rarity][tradeup.level].toDense().array() - tradeup.cost - tradeup.grossreturn) * (m_MappedPricesWithFees[tradeup.rarity][tradeup.level].toDense().array() - tradeup.cost - tradeup.grossreturn)).matrix();
		//tradeup.variance = tradeup.probability.dot(m2);
		//tradeup.stddev = std::sqrt(tradeup.variance);
		//tradeup.vmr = tradeup.variance / tradeup.grossreturn;
		//tradeup.profitchance = (m_MappedPricesWithFees[tradeup.rarity][tradeup.level].toDense().array() - tradeup.cost).array().cwiseSign().cwiseMax(0).matrix().dot(tradeup.probability.toDense());

	}

	bool ValidateTradeUp(const TradeUp& tradeup) {
		if (tradeup.computed == false) return false;
		if (tradeup.cost <= 0) return false;

		return true;
	}

	std::string Calculator::ExportTradeUp(TradeUp& tradeup) const {
		ComputeStatistical(tradeup);
		std::stringstream ss;
		const std::vector<Skin> skins = m_DB->m_Skins;
		const std::vector<size_t> ids_by_rarity = m_DB->m_SkinIDsByRarity[tradeup.rarity];
		const std::vector<size_t> hids_by_rarity = m_DB->m_SkinIDsByRarity[tradeup.rarity + 1];

		ss << tradeup.hash() << ",";
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

		const Eigen::VectorXf mask = tradeup.mask.toDense();
		const Eigen::VectorXf probabilities = tradeup.probability.toDense();

		for (int i = 0; i < ids_by_rarity.size(); i++) {
			if (mask(i) > 0.0f) {
				size_t id = ids_by_rarity[i];
				const int amount = static_cast<int>(mask(i));
				for (int j = 0; j < amount; j++) {
					const SkinCondition condition = ConditionFromFloat(g_Levels[tradeup.level / 2], tradeup.level & 1);
					ss << StringFromWeaponType(skins[id].m_WeaponType) << " | " << skins[id].m_Name << " ($" << skins[id].m_PricesSell[condition] << "),";
				}
			}
		}

		for (int i = 0; i < hids_by_rarity.size(); i++) {
			if (probabilities(i) > 0.0f) {
				size_t id = hids_by_rarity[i];
				const SkinCondition condition = MapCondition(skins[id], g_Levels[tradeup.level / 2], tradeup.level & 1);
				ss << StringFromWeaponType(skins[id].m_WeaponType) << " | " << skins[id].m_Name << " (" << ShortStringFromWeaponCondition(condition) << ")" << ",";
				ss << std::to_string(skins[id].m_PricesBuy[condition] * 0.87f - 0.01f) << ",";
				ss << std::to_string(probabilities(i)) << ", ";
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

			MT_INFO("  Searched total of {:15d} tradeups ({:8d} last second)", current, current - lastCount);
			lastCount = current;

			std::lock_guard<std::mutex> finishedguard(g_FinishedMutex);
			if (g_Finished) finished = true;
		}
	}

	void BruteforceCondition(const Calculator& calculator, int level, std::vector<TradeUp>& tradeups) {
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
				size_t l_TradeUpCount = 0;
				TradeUp tradeup(nRSkins, level, rarity);
				tradeup.mask.coeffRef(id1) = 10;
				if (calculator.Compute(tradeup)) tradeups.push_back(tradeup);
				l_TradeUpCount++;

#ifdef L2
				for (size_t id2 : ids_by_rarity[rarity]) {
					if (id2 == id1) continue;
					for (const auto p : p2) {
						const float A = p[0];
						const float B = p[1];

						TradeUp tradeup(nRSkins, level, rarity);
						tradeup.mask.coeffRef(id1) = A;
						tradeup.mask.coeffRef(id2) = B;

						if (calculator.Compute(tradeup)) tradeups.push_back(tradeup);
						l_TradeUpCount++;
					}

#ifdef L3
					for (size_t id3 : ids_by_rarity[rarity]) {
						if (id3 == id2 || id3 == id1) continue;
						for (const auto p : p3) {
							TradeUp tradeup(nRSkins, level, rarity);
							const float A = p[0];
							const float B = p[1];
							const float C = p[2];

							tradeup.mask.insert(id1) = A;
							tradeup.mask.insert(id2) = B;
							tradeup.mask.insert(id3) = C;

							if (calculator.Compute(tradeup)) tradeups.push_back(tradeup);
							l_TradeUpCount++;
						}
#ifdef L4
						for (size_t id4 : ids_by_rarity[rarity]) {
							if (id4 == id3 || id4 == id2 || id4 == id1) continue;
							for (const auto p : p4) {
								TradeUp tradeup(nRSkins, level, rarity);
								const float A = p[0];
								const float B = p[1];
								const float C = p[2];
								const float D = p[3];

								tradeup.mask.insert(id1) = A;
								tradeup.mask.insert(id2) = B;
								tradeup.mask.insert(id3) = C;
								tradeup.mask.insert(id4) = D;

								if (calculator.Compute(tradeup)) tradeups.push_back(tradeup);
								l_TradeUpCount++;
							}
						}
#endif
					}
#endif
				}
#endif
				std::lock_guard<std::mutex> guard(g_TradeUpCountMutex);
				g_TradeUpCount += l_TradeUpCount;
			}
		}
	}

	void Calculator::Bruteforce() {
		MT_INFO("Bruteforcing...");
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

		std::vector<TradeUp> all_tradeups;
		all_tradeups.reserve(tupt);
		for (int level = 0; level < g_nLevels * 2; level++) {
			for (TradeUp tradeup : tradeups[level]) {
				all_tradeups.push_back(tradeup);
			}
		}
		MT_INFO("Succesful bruteforcing, serched {}, found {} profitable tradeups in {} seconds ({:.2f} tradeups/second)!", g_TradeUpCount, tupt, time, g_TradeUpCount / time);

		std::sort(all_tradeups.begin(), all_tradeups.end(),
			[](TradeUp t1, TradeUp t2) {return ((t1.grossreturn / t1.cost) > (t2.grossreturn / t2.cost)); });

		std::ofstream of("out.csv");
		of << "Hash,Cost,EV,GrossProfit$,GrossProfit%,NetProfit$,NetProfit%,Profit%,Variance,Standard Deviation,VMR,Wear,StatTrak,";
		for (int i = 0; i < 10; i++) of << "Weapon" << i + 1 << ",";
		for (int i = 0; i < 20; i++) of << "Result" << i + 1 << "," << "Price" << i + 1 << "," << "Chance" << i + 1 << ",";
		of << "\n";
		MT_INFO("Exporting...");
		for (TradeUp t : all_tradeups) {
			of << ExportTradeUp(t);
		}
		of.close();
		MT_INFO("Sucessfully exported all tradeups");

	}

	struct ShortTradeUp {
		bool			stattrak : 1;
		SkinRarity		rarity : 3;
		unsigned int	id0 : 10;
		unsigned int	condition0 : 3;
		unsigned int	id1 : 10;
		unsigned int	condition1 : 3;
		unsigned int	id2 : 10;
		unsigned int	condition2 : 3;
		unsigned int	id3 : 10;
		unsigned int	condition3 : 3;
		unsigned int	id4 : 10;
		unsigned int	condition4 : 3;
		unsigned int	id5 : 10;
		unsigned int	condition5 : 3;
		unsigned int	id6 : 10;
		unsigned int	condition6 : 3;
		unsigned int	id7 : 10;
		unsigned int	condition7 : 3;
		unsigned int	id8 : 10;
		unsigned int	condition8 : 3;
		unsigned int	id9 : 10;
		unsigned int	condition9 : 3;
	};

	ShortTradeUp ToShortTradeUp(TradeUp tradeup) {
		ShortTradeUp stu;
		stu.stattrak = tradeup.level & 1;
		stu.rarity = tradeup.rarity;

		const Eigen::VectorXf mask = tradeup.mask.toDense();

		int k = 0;
		SkinCondition cond[10] = { SkinCondition::Max };
		size_t ids[10] = { 0 };

		for (int i = 0; i < tradeup.nSkins; i++) {
			if (mask(i) > 0.0f) {
				const int amount = static_cast<int>(mask(i));
				for (int j = 0; j < amount; j++) {
					cond[k] = ConditionFromFloat(g_Levels[tradeup.level / 2], false);
					ids[k] = i;
					k++;
				}
			}
		}

		stu.condition0 = static_cast<unsigned int>(cond[0]);
		stu.id0 = ids[0];
		stu.condition1 = static_cast<unsigned int>(cond[1]);
		stu.id1 = ids[1];
		stu.condition2 = static_cast<unsigned int>(cond[2]);
		stu.id2 = ids[2];
		stu.condition3 = static_cast<unsigned int>(cond[3]);
		stu.id3 = ids[3];
		stu.condition4 = static_cast<unsigned int>(cond[4]);
		stu.id4 = ids[4];
		stu.condition5 = static_cast<unsigned int>(cond[5]);
		stu.id5 = ids[5];
		stu.condition6 = static_cast<unsigned int>(cond[6]);
		stu.id6 = ids[6];
		stu.condition7 = static_cast<unsigned int>(cond[7]);
		stu.id7 = ids[7];
		stu.condition8 = static_cast<unsigned int>(cond[8]);
		stu.id8 = ids[8];
		stu.condition9 = static_cast<unsigned int>(cond[9]);
		stu.id9 = ids[9];

		return stu;
	}

	TradeUp FromShortTradeUp(ShortTradeUp stu, int nSkins, int level) {
		TradeUp t(nSkins, level, stu.rarity);

		t.mask.coeffRef(stu.id0)++;
		t.mask.coeffRef(stu.id1)++;
		t.mask.coeffRef(stu.id2)++;
		t.mask.coeffRef(stu.id3)++;
		t.mask.coeffRef(stu.id4)++;
		t.mask.coeffRef(stu.id5)++;
		t.mask.coeffRef(stu.id6)++;
		t.mask.coeffRef(stu.id7)++;
		t.mask.coeffRef(stu.id8)++;
		t.mask.coeffRef(stu.id9)++;

		t.nSkins = nSkins;
		t.level = level;

		return t;
	}

	constexpr size_t BUF_SIZE_BYTES = sizeof(ShortTradeUp);

	TradeUp::TradeUp(std::string hash, size_t nRSkins, size_t l) {
		mask.resize(nSkins);
		cost = grossreturn = netreturn = variance = stddev = vmr = profitchance = 0;
		Clear();
		std::vector<unsigned char> b = base64_decode(hash);
		unsigned char buf[BUF_SIZE_BYTES];
		for (int i = 0; i < BUF_SIZE_BYTES; i++) {
			buf[i] = b[i];
		}
		ShortTradeUp stu;
		std::memcpy(&stu, buf, sizeof(stu));
		TradeUp t = FromShortTradeUp(stu, nRSkins, l);
		mask = t.mask;
		rarity = t.rarity;
		level = t.level;
		nSkins = t.nSkins;
	}

	std::string TradeUp::hash() const {
		unsigned char buf[BUF_SIZE_BYTES];
		ShortTradeUp stu = ToShortTradeUp(*this);
		std::memcpy(buf, &stu, BUF_SIZE_BYTES);
		return base64_encode(buf, BUF_SIZE_BYTES);
	}
}
