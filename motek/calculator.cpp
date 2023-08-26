#include <fstream>
#include <functional>
#include <iostream>
#include <motek/calculator.hpp>
#include <thread>

#include <motek/combinate.hpp>
#include <motek/log.hpp>
#include <motek/partitions.hpp>

#include <motek/variations.hpp>

// #define MT

namespace motek {
static size_t g_TradeUpCount;
static std::mutex g_TradeUpCountMutex;

static bool g_Finished = false;
static std::mutex g_FinishedMutex;

constexpr uint64_t g_VarBitCount = 4;
inline uint64_t variations_key(std::array<uint8_t, 8> param1,
                               std::array<uint8_t, 8> param2) {
  const std::array<std::array<uint8_t, 8>, 2> params = {param1, param2};
  uint64_t s = 0;

  for (size_t i = 0; i < 2; i++) {
    uint64_t part = 0;
    for (size_t j = 0; j < 8; j++) {
      part |= params[i][j] << (g_VarBitCount * j);
    }
    s |= part << (32 * i);
  }

  return s;
}

wear_configs_t
generate_wear_variations(const std::vector<std::vector<wear_t>> &wear_tuples,
                         bool stattrak) {
  // TODO rewrite this w/ compositions instead of partitions
  wear_configs_t values;

  for (std::vector<wear_t> wears : wear_tuples) {
    std::vector<std::array<int, 10>> partitions =
        create_partitions(wears.size());
    for (size_t i = 0; i < wears.size(); i++) {
      std::array<int, 10> w;
      for (size_t j = 0; j < 10; j++) {
        w[j] = i;
      }
      partitions.push_back(w);
    }

    for (auto partition : partitions) {
      wear_t avg = 0;
      wear_config_t map;

      for (size_t i = 0; i < 10; i++) {
        wear_t w = wears[partition[i]];
        map[ConditionFromFloat(w, stattrak)]++;

        avg += w;
        //          ws[i] = ConditionFromFloat(w, stattrak);
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

    if (rarity == SkinRarity::Covert)
      continue;
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
  // MT_INFO("{}x{} - {}x{}", tradeup.mask_matrix.rows(),
  // tradeup.mask_matrix.cols(),
  // m_PriceMatrix[tradeup.stattrak][tradeup.rarity].rows(),
  // m_PriceMatrix[tradeup.stattrak][tradeup.rarity].cols());
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

bool ValidateTradeUp(const TradeUp &tradeup) {
  return true;
  if (tradeup.m_Computed == false)
    return false;
  if (tradeup.m_Cost <= 0)
    return false;

  return true;
}

std::string Calculator::ExportTradeUp(TradeUp &tradeup) const {
  ComputeStatistical(tradeup);
  std::stringstream ss;
  const std::vector<Skin> skins = m_DB->m_Skins;
  const std::vector<size_t> ids_by_rarity =
      m_DB->m_SkinIDsByRarity[tradeup.m_Rarity];
  const std::vector<size_t> hids_by_rarity =
      m_DB->m_SkinIDsByRarity[tradeup.m_Rarity + 1];

  //		ss << tradeup.hash() << ",";
  ss << ""
     << ",";
  ss << tradeup.m_Cost << ",";
  ss << tradeup.m_GrossReturn << ",";
  ss << tradeup.m_GrossReturn - tradeup.m_Cost << ",";
  ss << (tradeup.m_GrossReturn / tradeup.m_Cost - 1.0f) * 100.0f << ",";
  ss << tradeup.m_NetReturn - tradeup.m_Cost << ",";
  ss << (tradeup.m_NetReturn / tradeup.m_Cost - 1.0f) * 100.0f << ",";
  ss << tradeup.m_ProfitChance * 100.0f << ",";

  ss << tradeup.m_AverageWear << ",";
  ss << tradeup.m_StatTrak << ",";

  const Eigen::VectorXf mask = tradeup.m_Mask.toDense();
  const Eigen::MatrixXf mask_big = tradeup.m_MaskBig.toDense();
  const Eigen::VectorXf probabilities = tradeup.m_Probability.toDense();

  for (Eigen::SparseVector<float>::InnerIterator it(tradeup.m_MaskBig); it;
       ++it) {
    int id = it.index() / 5;
    const int amount = static_cast<int>(it.value());
    for (int j = 0; j < amount; j++) {
      SkinCondition condition = SkinCondition(it.index() % g_ConditionCount);
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
        default:
          break;
        }
      }
      ss << StringFromWeaponType(skins[id].m_WeaponType) << " | "
         << skins[id].m_Name << " ("
         << ShortStringFromWeaponCondition(condition) << ") : $"
         << skins[id].m_PricesSell[condition] << ",";
    }
  }

  for (Eigen::SparseVector<float>::InnerIterator it(tradeup.m_MaskBig); it;
       ++it) {
    size_t id = it.index();
    const SkinCondition condition =
        MapCondition(skins[id], tradeup.m_AverageWear, tradeup.m_StatTrak);
    ss << StringFromWeaponType(skins[id].m_WeaponType) << " | "
       << skins[id].m_Name << " (" << ShortStringFromWeaponCondition(condition)
       << ")"
       << ",";
    ss << std::to_string(ValveTax(skins[id].m_PricesBuy[condition])) << ",";
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

  std::array<uint8_t, 8> array1 = {};

  SkinCondition condition = ConditionFromFloat(wear, stattrak);
  SkinCondition condition_non_st = ConditionFromFloat(wear, false);
  std::cout << kv.first << " : ";
  {
    size_t i = 0;
    for (auto k : wear_config) {
      //      std::cout << ShortStringFromWeaponCondition(k.first) << " : " <<
      //      k.second << " ; ";
      array1[i] = k.second;
      std::cout << int(array1[i]) << " , ";
      i++;
    }
  }
  EK_INFO("{:16X}", variations_key(array1, std::array<uint8_t, 8>{}));
  std::cout << "\n";
  return;

  std::vector<size_t> ids_by_rarity[SkinRarity::Contraband + 1];

  for (SkinRarity rarity :
       {SkinRarity::Consumer, SkinRarity::Industrial, SkinRarity::MilSpec,
        SkinRarity::Restricted, SkinRarity::Classified}) {
    for (size_t id = 0; id < calculator.m_DB->m_SkinIDsByRarity[rarity].size();
         id++) {
      if (calculator.m_DB
              ->m_Skins[calculator.m_DB->m_SkinIDsByRarity[rarity][id]]
              .m_Banned[condition] == false) {
        ids_by_rarity[rarity].push_back(id);
      }
    }
  }

  for (SkinRarity rarity :
       {SkinRarity::Consumer, SkinRarity::Industrial, SkinRarity::MilSpec,
        SkinRarity::Restricted, SkinRarity::Classified}) {
    size_t nRSkins = calculator.m_DB->m_SkinIDsByRarity[rarity].size();

    for (size_t depth = 0; depth < max_depth; depth++) {
      size_t l_TradeUpCount = 0;
      const std::vector<std::vector<size_t>> combinations =
          combination(ids_by_rarity[rarity].size(), depth + 1);
      for (size_t i = 0; i < combinations.size(); i++) {
        // const std::vector<size_t> ids = combinations[i];
        for (auto partition : ps[depth]) {
          TradeUp tradeup(nRSkins, wear, stattrak, rarity);

          for (size_t j = 0; j < depth + 1; j++) {
            const size_t id = ids_by_rarity[rarity][combinations[i][j]];
            const size_t p = partition[j];

            tradeup.m_Mask.insert(id) = p;
            tradeup.m_MaskBig.insert(id * 5 + condition_non_st) = p;
          }

          const auto gross = calculator.ComputeGross(tradeup);
          const auto cost = calculator.ComputeCost(tradeup);

          if (gross > cost) {
            tradeups.push_back(tradeup);
          }
          l_TradeUpCount++;
        }
      }
      std::lock_guard<std::mutex> guard(g_TradeUpCountMutex);
      g_TradeUpCount += l_TradeUpCount;
    }
  }
}

void Calculator::Bruteforce(const std::vector<std::vector<wear_t>> &wear_tuples,
                            size_t max_depth) const {
  MT_INFO("Bruteforcing...");

  wear_configs_t wear_configs[2] = {
      generate_wear_variations(wear_tuples, false),
      generate_wear_variations(wear_tuples, true),
  };

  std::thread reporter;
  std::vector<std::vector<TradeUp>> tradeups[2];

  std::chrono::steady_clock::time_point begin =
      std::chrono::steady_clock::now();

  // this is retarded but idc lol
  // it works
  // at least i hope it does
#ifdef MT
  reporter = std::thread(ReportAmount);
  std::vector<std::thread> threads[2];
  for (size_t stattrak = 0; stattrak <= 1; stattrak++) {
    size_t i = 0;
    for (auto keyvalue : wear_configs[stattrak]) {
      tradeups[stattrak].push_back(std::vector<TradeUp>());
      threads[stattrak].push_back(
          std::thread(BruteforceCondition, std::cref(*this), keyvalue, stattrak,
                      std::ref(tradeups[stattrak][i])));
      i++;
    }
  }

  for (size_t stattrak = 0; stattrak <= 1; stattrak++) {
    size_t i = 0;
    for (auto keyvalue : wear_configs[stattrak]) {
      threads[stattrak][i].join();
      i++;
    }
  }
#else
  // TODO making single threading work
  reporter = std::thread(ReportAmount);
  for (size_t stattrak = 0; stattrak <= 1; stattrak++) {
    size_t i = 0;
    for (auto keyvalue : wear_configs[stattrak]) {
      tradeups[stattrak].push_back(std::vector<TradeUp>());
      BruteforceCondition(std::cref(*this), max_depth, keyvalue, stattrak,
                          std::ref(tradeups[stattrak][i]));
      i++;
    }
  }
#endif

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
  of << "Hash,Cost,EV,GrossProfit$,GrossProfit%,NetProfit$,NetProfit%,Profit%,"
        "Variance,Standard Deviation,VMR,Wear,StatTrak,";
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
