#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <motek/calculator.hpp>
#include <thread>

#include <motek/log.hpp>

#include "combinatorics.hpp"

//#define MT

namespace motek {
static size_t g_TradeUpCount;
static std::mutex g_TradeUpCountMutex;

static bool g_Finished = false;
static std::mutex g_FinishedMutex;

constexpr uint64_t g_VarBitCount = 4;
inline uint64_t generate_single_key(std::array<uint8_t, 8> param) {
  uint64_t key = 0;
  for (size_t j = 0; j < 8; j++) {
    key |= param[j] << (g_VarBitCount * j);
  }
  return key;
}
inline uint64_t generate_double_key(uint64_t lower, uint64_t upper) {
  constexpr uint64_t shift_amount = 32ULL;
  uint64_t key = lower | (upper << (shift_amount));
  return key;
}

wear_configs_t
generate_wear_variations(const std::vector<std::vector<wear_t>> &wear_tuples,
                         bool stattrak) {
  // TODO rewrite this w/ compositions instead of partitions
  wear_configs_t values;

  for (std::vector<wear_t> wears : wear_tuples) {
    std::vector<uint64_t> compositions =
        combinatorics::g_CompositionsFlat[wears.size()];
    for (uint64_t composition : compositions) {
      wear_t avg = 0;
      wear_config_t map;

      for (size_t i = 0; i < 10; i++) {
        wear_t w = wears[combinatorics::at(composition, i)];
        map[ConditionFromFloat(w, stattrak)]++;

        avg += w;
      }

      avg /= 10;

      values[avg] = map;
    }
  }

  return values;
}

Calculator::Calculator(std::shared_ptr<SkinDB> database) : m_DB(database) {
  MT_INFO("Building calculator..");

  for (SkinRarity rarity :
       {SkinRarity::Consumer, SkinRarity::Industrial, SkinRarity::MilSpec,
        SkinRarity::Restricted, SkinRarity::Classified, SkinRarity::Covert}) {
    const std::vector<size_t> &ids = m_DB->m_SkinIDsByRarity[rarity];
    const std::vector<size_t> &hids = m_DB->m_SkinIDsByRarity[rarity + 1];
    const size_t skinc = ids.size();
    const size_t higherc = hids.size();

    for (bool stattrak : {false, true}) {
      for (wear_t wear = g_WearRangeMin; wear < g_WearRangeMax; wear++) {
        m_Prices[stattrak][rarity][wear].resize(skinc);
        m_MappedPrices[stattrak][rarity][wear].resize(skinc);
        m_MappedPricesWithFees[stattrak][rarity][wear].resize(skinc);
      }
    }

    m_Factor[rarity].resize(skinc);
    m_Transformer[rarity].resize(higherc, skinc);

    if (rarity == SkinRarity::Covert) {
      continue;
    }
    for (bool stattrak : {false, true}) {
      for (wear_t wear = g_WearRangeMin; wear < g_WearRangeMax; wear++) {
        for (const size_t id : ids) {
          const Skin skin = m_DB->m_Skins[id];
          SkinCondition condition = ConditionFromFloat(wear, stattrak);
          SkinCondition mapped_condition = MapCondition(skin, wear, stattrak);

          m_Prices[stattrak][rarity][wear](skin.m_rID) =
              skin.m_PricesSell[condition];
          m_MappedPrices[stattrak][rarity][wear](skin.m_rID) =
              skin.m_PricesBuy[mapped_condition];
          m_MappedPricesWithFees[stattrak][rarity][wear](skin.m_rID) =
              ValveTax(skin.m_PricesBuy[mapped_condition]);
        }
      }

      m_PricesCompressed[stattrak][rarity].resize(skinc * g_ConditionCount);
      for (const size_t id : ids) {
        const Skin skin = m_DB->m_Skins[id];
        for (SkinCondition condition : {BS, WW, FT, MW, FN}) {
          SkinCondition c = condition;
          if (stattrak) {
            switch (condition) {
            case BS:
              c = BS_ST;
              break;
            case WW:
              c = WW_ST;
              break;
            case FT:
              c = FT_ST;
              break;
            case MW:
              c = MW_ST;
              break;
            case FN:
              c = FN_ST;
              break;
            default:
              break;
            }
          }
          m_PricesCompressed[stattrak][rarity](
              skin.m_rID * g_ConditionCount + condition) = skin.m_PricesSell[c];
        }
      }
    }

    std::vector<Eigen::Triplet<float>> transformer_triplets;

    auto &factor = m_Factor[rarity];
    auto &transformer = m_Transformer[rarity];

    for (const SkinCollection &collection : m_DB->m_Collections) {
      float hc = collection.m_SkinsByRarity[rarity + 1].size();

      for (const Skin &first : collection.m_SkinsByRarity[rarity]) {
        factor.insertBack(first.m_rID) = hc;
        for (const Skin &second : collection.m_SkinsByRarity[rarity + 1]) {
          transformer_triplets.push_back({static_cast<int>(second.m_rID),
                                          static_cast<int>(first.m_rID), 1});
        }
      }
    }
    transformer.setFromTriplets(transformer_triplets.begin(),
                                transformer_triplets.end());
    transformer.makeCompressed();
  }
  MT_INFO("Built calculator");
}

float Calculator::ComputeGross(TradeUp &tradeup) const {
  const float gross =
      ((m_Transformer[tradeup.m_Rarity] * tradeup.m_Mask) /
       tradeup.m_Mask.dot(m_Factor[tradeup.m_Rarity]))
          .dot(m_MappedPricesWithFees[tradeup.m_StatTrak][tradeup.m_Rarity + 1]
                                     [tradeup.m_AverageWear]);
  return gross;
}
float Calculator::ComputeCost(TradeUp &tradeup) const {
  const float cost = tradeup.m_MaskBig.dot(
      m_PricesCompressed[tradeup.m_StatTrak][tradeup.m_Rarity]);
  return cost;
}

void Calculator::ComputeStatistical(TradeUp &tradeup) const {
  const float factor = tradeup.m_Mask.dot(m_Factor[tradeup.m_Rarity]);
  const Eigen::SparseVector<float> probability =
      (m_Transformer[tradeup.m_Rarity] * tradeup.m_Mask) / factor;

  const float gross = probability.dot(
      m_MappedPricesWithFees[tradeup.m_StatTrak][tradeup.m_Rarity + 1]
                            [tradeup.m_AverageWear]);
  const float cost = tradeup.m_MaskBig.dot(
      m_PricesCompressed[tradeup.m_StatTrak][tradeup.m_Rarity]);

  tradeup.m_Probability = probability;
  tradeup.m_Cost = cost;
  tradeup.m_GrossReturn = gross;
  tradeup.m_NetReturn =
      probability.dot(m_MappedPrices[tradeup.m_StatTrak][tradeup.m_Rarity + 1]
                                    [tradeup.m_AverageWear]);

  tradeup.m_ProfitChance =
      (m_MappedPricesWithFees[tradeup.m_StatTrak][tradeup.m_Rarity + 1]
                             [tradeup.m_AverageWear]
                                 .array() -
       tradeup.m_Cost)
          .array()
          .cwiseSign()
          .cwiseMax(0)
          .matrix()
          .dot(probability.toDense());
  tradeup.m_Computed = true;
}

std::string Calculator::ExportTradeUp(TradeUp &tradeup) const {
  ComputeStatistical(tradeup);
  std::stringstream ss;
  const std::vector<Skin> skins = m_DB->m_Skins;
  const std::vector<size_t> ids_by_rarity =
      m_DB->m_SkinIDsByRarity[tradeup.m_Rarity];
  const std::vector<size_t> hids_by_rarity =
      m_DB->m_SkinIDsByRarity[tradeup.m_Rarity + 1];

  ss << tradeup.m_Cost << ",";
  ss << tradeup.m_GrossReturn << ",";
  ss << tradeup.m_GrossReturn - tradeup.m_Cost << ",";
  ss << (tradeup.m_GrossReturn / tradeup.m_Cost - 1.0F) * 100.0F << ",";
  ss << tradeup.m_NetReturn - tradeup.m_Cost << ",";
  ss << (tradeup.m_NetReturn / tradeup.m_Cost - 1.0F) * 100.0F << ",";
  ss << tradeup.m_ProfitChance * 100.0F << ",";

  ss << tradeup.m_AverageWear << ",";
  ss << tradeup.m_StatTrak << ",";


  for (Eigen::SparseVector<float>::InnerIterator it(tradeup.m_MaskBig); it; ++it) {
      size_t id = ids_by_rarity[it.index() / 5];
      const int amount = static_cast<int>(it.value());
      for (int j = 0; j < amount; j++) {
        SkinCondition condition = SkinCondition(it.index() % 5);
        if (tradeup.m_StatTrak) {
          switch (condition) {
          case BS:
            condition = BS_ST;
            break;
          case WW:
            condition = WW_ST;
            break;
          case FT:
            condition = FT_ST;
            break;
          case MW:
            condition = MW_ST;
            break;
          case FN:
            condition = FN_ST;
            break;
          }
        }
        ss << StringFromWeaponType(skins[id].m_WeaponType) << " | "
           << skins[id].m_Name << " ("
           << ShortStringFromWeaponCondition(condition) << ") : $"
           << skins[id].m_PricesSell[condition] << ",";
      }
  }

  for (Eigen::SparseVector<float>::InnerIterator it(tradeup.m_Probability); it; ++it) {
      size_t id = hids_by_rarity[it.index()];
      const SkinCondition condition =
          MapCondition(skins[id], tradeup.m_AverageWear, tradeup.m_StatTrak);
      ss << StringFromWeaponType(skins[id].m_WeaponType) << " | "
         << skins[id].m_Name << " ("
         << ShortStringFromWeaponCondition(condition) << ")"
         << ",";
      ss << std::to_string(skins[id].m_PricesBuy[condition] * 0.87f - 0.01f)
         << ",";
      ss << std::to_string(it.value()) << ", ";
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

    MT_INFO("  Searched total of {:15d} tradeups ({:8d} last second)", current,
            current - lastCount);
    lastCount = current;

    std::lock_guard<std::mutex> finishedguard(g_FinishedMutex);
    if (g_Finished)
      finished = true;
  }
}

void BruteforceCondition(const Calculator &calculator, size_t max_depth,
                         std::pair<wear_t, wear_config_t> kv, bool stattrak,
                         std::vector<TradeUp> &tradeups) {
  wear_t wear = kv.first;
  wear_config_t wear_config = kv.second;

  std::array<uint8_t, 8> condition_amount_array = {};
  std::array<SkinCondition, 8> condition_type_array = {};
  std::array<SkinCondition, 8> condition_type_with_st_array = {};
  uint64_t condition_count = wear_config.size();
  {
    size_t index = 0;
    for (const auto &[condition, amount] : wear_config) {
      condition_type_array[index] = condition;
      condition_type_with_st_array[index] = SkinCondition(condition + 5*stattrak);
      condition_amount_array[index] = amount;
      index++;
    }
  }
  const uint64_t condition_key = generate_single_key(condition_amount_array);

  SkinCondition condition = ConditionFromFloat(wear, stattrak);
  SkinCondition condition_non_st = ConditionFromFloat(wear, false);
  std::vector<size_t> ids_by_rarity[SkinRarity::Contraband + 1];

  for (SkinRarity rarity :
       {SkinRarity::Consumer, SkinRarity::Industrial, SkinRarity::MilSpec,
        SkinRarity::Restricted, SkinRarity::Classified}) {
    for (size_t id = 0; id < calculator.m_DB->m_SkinIDsByRarity[rarity].size();
         id++) {
      bool banned = false;
      for (int i = 0; i < condition_count; i++) {
        if (calculator.m_DB
                ->m_Skins[calculator.m_DB->m_SkinIDsByRarity[rarity][id]]
                  .m_Banned[condition_type_with_st_array[i]] == true) {
          banned = true;
        }
      }
      if (!banned) {
        ids_by_rarity[rarity].push_back(id);
      }
    }
  }

  for (SkinRarity rarity :
       {SkinRarity::Consumer, SkinRarity::Industrial, SkinRarity::MilSpec,
        SkinRarity::Restricted, SkinRarity::Classified}) {
    size_t nRSkins = calculator.m_DB->m_SkinIDsByRarity[rarity].size();

    for (size_t depth = 1; depth <= max_depth; depth++) {
      size_t l_TradeUpCount = 0;

      std::vector<size_t> ids =
          combinatorics::first_combination(ids_by_rarity[rarity].size(), depth);
      for (uint64_t i = 0;
           i < combinatorics::nCr(ids_by_rarity[rarity].size(), depth); i++) {
        for (uint64_t composition : combinatorics::g_Compositions[depth]) {
          TradeUp tradeup(nRSkins, wear, stattrak, rarity);

          for (size_t i = 0; i < depth; i++) {
            tradeup.SetAmount(ids_by_rarity[rarity][ids[i]],
                                   combinatorics::at(composition, i));
          }

          const float gross = calculator.ComputeGross(tradeup);

          const uint64_t combined_key =
              generate_double_key(composition, condition_key);

          const std::vector<uint64_t> &variations =
              combinatorics::g_Variations.at(combined_key);

          for (uint64_t variation : variations) {
            for (int k = 0; k < depth * condition_count; k++) {
              const uint64_t id = ids_by_rarity[rarity][ids[k % depth]];
              SkinCondition local_condition = condition_type_array[k / depth];
              uint64_t amount = combinatorics::at(variation, k);
              tradeup.SetAmountCondition(id, amount, local_condition);
            }
            const float cost = calculator.ComputeCost(tradeup);
            if (cost < gross) {
              tradeups.push_back(tradeup);
            }
            tradeup.m_MaskBig.setZero();
          }
          l_TradeUpCount++;
        }
        combinatorics::next_combination(ids, depth);
      }
      std::lock_guard<std::mutex> guard(g_TradeUpCountMutex);
      g_TradeUpCount += l_TradeUpCount;
    }
  }
}

void Calculator::Bruteforce(const std::vector<std::vector<wear_t>> &wear_tuples,
                            size_t max_depth, bool multithreaded) const {
  MT_INFO("Bruteforcing...");

  wear_configs_t wear_configs = generate_wear_variations(wear_tuples, false);

  std::thread reporter;
  std::vector<std::vector<TradeUp>> tradeups[2];

  std::chrono::steady_clock::time_point begin =
      std::chrono::steady_clock::now();

  if (multithreaded) {
    reporter = std::thread(ReportAmount);
    std::vector<std::thread> threads[2];
    for (size_t stattrak = 0; stattrak <= 1; stattrak++) {
      size_t index = 0;
      tradeups[stattrak].resize(wear_configs.size());
      for (auto keyvalue : wear_configs) {
        tradeups[stattrak][index] = std::vector<TradeUp>();
        threads[stattrak].emplace_back(std::thread(
            BruteforceCondition, std::cref(*this), max_depth, keyvalue,
            stattrak, std::ref(tradeups[stattrak][index])));
        index++;
      }
    }

    for (size_t stattrak = 0; stattrak <= 1; stattrak++) {
      size_t i = 0;
      for (auto keyvalue : wear_configs) {
        threads[stattrak][i].join();
        i++;
      }
    }
  } else {
    // TODO making single threading work
    reporter = std::thread(ReportAmount);
    for (size_t stattrak = 0; stattrak <= 1; stattrak++) {
      size_t i = 0;
      for (auto keyvalue : wear_configs) {
        tradeups[stattrak].push_back(std::vector<TradeUp>());
        BruteforceCondition(std::cref(*this), max_depth, keyvalue, stattrak,
                            std::ref(tradeups[stattrak][i]));
        i++;
      }
    }
  }

  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

  double time =
      std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();

  {
    std::lock_guard<std::mutex> guard(g_FinishedMutex);
    g_Finished = true;
  }

  reporter.join();

  size_t tupt = 0;

  std::vector<TradeUp> all_tradeups;
  //		all_tradeups.reserve(tupt);
  for (size_t stattrak = 0; stattrak <= 1; stattrak++) {
    for (std::vector<TradeUp> tradeuplist : tradeups[stattrak]) {
      tupt += tradeuplist.size();
      for (TradeUp tradeup : tradeuplist) {
        all_tradeups.push_back(tradeup);
      }
    }
  }
  MT_INFO("Succesful bruteforcing, serched {}, found {} profitable tradeups in "
          "{} seconds ({:.2f} tradeups/second)!",
          g_TradeUpCount, tupt, time, g_TradeUpCount / time);

  ////std::sort(all_tradeups.begin(), all_tradeups.end(),
  ////	[](TradeUp t1, TradeUp t2) {return ((t1.grossreturn / t1.cost) <
  ///(t2.grossreturn / t2.cost)); });

  std::ofstream of("out.csv");
  of << "Cost,EV,GrossProfit$,GrossProfit%,NetProfit$,NetProfit%,Profit%,"
        "Wear,StatTrak,";
  for (int i = 0; i < 10; i++)
    of << "Weapon" << i + 1 << ",";
  for (int i = 0; i < 20; i++)
    of << "Result" << i + 1 << ","
       << "Price" << i + 1 << ","
       << "Chance" << i + 1 << ",";
  of << "\n";
  MT_INFO("Exporting...");
  int n = 0;
  for (TradeUp t : all_tradeups) {
    of << ExportTradeUp(t);
  }
  of.close();
  MT_INFO("Sucessfully exported all tradeups");
}

} // namespace motek
