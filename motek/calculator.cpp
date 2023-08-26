#include <bitset>
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
generate_wear_variations(const std::vector<std::vector<WearType>> &wear_tuples,
                         bool stattrak) {
  wear_configs_t values;

  for (std::vector<WearType> wears : wear_tuples) {
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
      WearType avg = 0;
      wear_config_t map;

      for (size_t i = 0; i < 10; i++) {
        WearType w = wears[partition[i]];
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

Calculator::Calculator(std::shared_ptr<SkinDB> db) : m_DB(db) {
  MT_INFO("Building calculator..");

  for (SkinRarity rarity :
       {SkinRarity::Consumer, SkinRarity::Industrial, SkinRarity::MilSpec,
        SkinRarity::Restricted, SkinRarity::Classified, SkinRarity::Covert}) {
    const std::vector<size_t> &ids = m_DB->m_SkinIDsByRarity[rarity];
    const std::vector<size_t> &hids = m_DB->m_SkinIDsByRarity[rarity + 1];
    const size_t skinc = ids.size();
    const size_t higherc = hids.size();

    for (bool stattrak : {false, true}) {
      for (WearType wear = g_WearRangeMin; wear < g_WearRangeMax; wear++) {
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
      for (WearType wear = g_WearRangeMin; wear < g_WearRangeMax; wear++) {
        for (const size_t id : ids) {
          const Skin skin = m_DB->m_Skins[id];
          SkinCondition condition = ConditionFromFloat(wear, stattrak);
          SkinCondition mapped_condition = MapCondition(skin, wear, stattrak);

          m_Prices[stattrak][rarity][wear](skin.m_rID) =
              skin.m_PricesSell[condition];
          m_MappedPrices[stattrak][rarity][wear](skin.m_rID) =
              skin.m_PricesBuy[mapped_condition];
          m_MappedPricesWithFees[stattrak][rarity][wear](skin.m_rID) =
              skin.m_PricesBuy[mapped_condition] * 0.87f - 0.01f;
        }
      }

      m_PricesCompressed[stattrak][rarity].resize(skinc * 5);
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
            case BS_ST:
              c = BS_ST;
              break;
            case WW_ST:
              c = WW_ST;
              break;
            case FT_ST:
              c = FT_ST;
              break;
            case MW_ST:
              c = MW_ST;
              break;
            case FN_ST:
              c = FN_ST;
              break;
            default:
              c = BS_ST;
              break;
            }
          }
          m_PricesCompressed[stattrak][rarity](skin.m_rID * 5 + condition) =
              skin.m_PricesSell[c];
        }
      }
    }

    for (int level = 0; level < g_nLevels * 2; level++) {
      std::vector<Eigen::Triplet<float>> transformer_triplets;

      auto &factor = m_Factor[rarity];
      auto &transformer = m_Transformer[rarity];

      for (const SkinCollection &collection : db->m_Collections) {
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
  }
  MT_INFO("Built calculator");
}

float Calculator::ComputeGross(TradeUp &tradeup) const {
  const float gross =
      ((m_Transformer[tradeup.rarity] * tradeup.mask_vector) /
       tradeup.mask_vector.dot(m_Factor[tradeup.rarity]))
          .dot(m_MappedPricesWithFees[tradeup.stattrak][tradeup.rarity + 1]
                                     [tradeup.average_wear]);
  return gross;
}
float Calculator::ComputeCost(TradeUp &tradeup) const {
  // MT_INFO("{}x{} - {}x{}", tradeup.mask_matrix.rows(),
  // tradeup.mask_matrix.cols(),
  // m_PriceMatrix[tradeup.stattrak][tradeup.rarity].rows(),
  // m_PriceMatrix[tradeup.stattrak][tradeup.rarity].cols());
  const float cost = tradeup.mask_big.dot(
      m_PricesCompressed[tradeup.stattrak][tradeup.rarity]);
  return cost;
}

void Calculator::ComputeStatistical(TradeUp &tradeup) const {
  const float factor = tradeup.mask_vector.dot(m_Factor[tradeup.rarity]);
  const Eigen::SparseVector<float> probability =
      (m_Transformer[tradeup.rarity] * tradeup.mask_vector) / factor;

  const float gross = probability.dot(
      m_MappedPricesWithFees[tradeup.stattrak][tradeup.rarity + 1]
                            [tradeup.average_wear]);
  const float cost = tradeup.mask_big.dot(
      m_PricesCompressed[tradeup.stattrak][tradeup.rarity]);

  tradeup.probability = probability;
  tradeup.cost = cost;
  tradeup.grossreturn = gross;
  tradeup.ev = tradeup.grossreturn;
  tradeup.netreturn = tradeup.probability.dot(
      m_MappedPrices[tradeup.stattrak][tradeup.rarity + 1]
                    [tradeup.average_wear]);

  // const Eigen::VectorXf m2 =
  // ((m_MappedPricesWithFees[tradeup.rarity][tradeup.level].toDense().array() -
  // tradeup.cost - tradeup.grossreturn) *
  // (m_MappedPricesWithFees[tradeup.rarity][tradeup.level].toDense().array() -
  // tradeup.cost - tradeup.grossreturn)).matrix(); tradeup.variance =
  // tradeup.probability.dot(m2); tradeup.stddev = std::sqrt(tradeup.variance);
  // tradeup.vmr = tradeup.variance / tradeup.grossreturn;
  // tradeup.profitchance =
  // (m_MappedPricesWithFees[tradeup.rarity][tradeup.level].toDense().array() -
  // tradeup.cost).array().cwiseSign().cwiseMax(0).matrix().dot(tradeup.probability.toDense());
}

bool ValidateTradeUp(const TradeUp &tradeup) {
  if (tradeup.computed == false)
    return false;
  if (tradeup.cost <= 0)
    return false;

  return true;
}

std::string Calculator::ExportTradeUp(TradeUp &tradeup) const {
  ComputeStatistical(tradeup);
  std::stringstream ss;
  const std::vector<Skin> skins = m_DB->m_Skins;
  const std::vector<size_t> ids_by_rarity =
      m_DB->m_SkinIDsByRarity[tradeup.rarity];
  const std::vector<size_t> hids_by_rarity =
      m_DB->m_SkinIDsByRarity[tradeup.rarity + 1];

  //		ss << tradeup.hash() << ",";
  ss << ""
     << ",";
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

  ss << tradeup.average_wear << ",";
  ss << tradeup.stattrak << ",";

  const Eigen::VectorXf mask = tradeup.mask_vector.toDense();
  const Eigen::MatrixXf mask_big = tradeup.mask_big.toDense();
  const Eigen::VectorXf probabilities = tradeup.probability.toDense();

  for (size_t i = 0; i < ids_by_rarity.size() * 5; i++) {
    if (mask_big(i) > 0.0f) {
      size_t id = ids_by_rarity[i / 5];
      const int amount = static_cast<int>(mask_big(i));
      for (int j = 0; j < amount; j++) {
        SkinCondition condition = SkinCondition(i % 5);
        if (tradeup.stattrak) {
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
    };
  }

  for (size_t i = 0; i < hids_by_rarity.size(); i++) {
    if (probabilities(i) > 0.0f) {
      size_t id = hids_by_rarity[i];
      const SkinCondition condition =
          MapCondition(skins[id], tradeup.average_wear, tradeup.stattrak);
      ss << StringFromWeaponType(skins[id].m_WeaponType) << " | "
         << skins[id].m_Name << " ("
         << ShortStringFromWeaponCondition(condition) << ")"
         << ",";
      ss << std::to_string(skins[id].m_PricesBuy[condition] * 0.87f - 0.01f)
         << ",";
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

    MT_INFO("  Searched total of {:15d} tradeups ({:8d} last second)", current,
            current - lastCount);
    lastCount = current;

    std::lock_guard<std::mutex> finishedguard(g_FinishedMutex);
    if (g_Finished)
      finished = true;
  }
}

void BruteforceCondition(const Calculator &calculator,
                         std::pair<WearType, wear_config_t> kv, bool stattrak,
                         std::vector<TradeUp> &tradeups) {
  WearType wear = kv.first;
  wear_config_t wear_config = kv.second;

  std::array<uint8_t, 8> array1 = {};

  SkinCondition condition = ConditionFromFloat(wear, stattrak);
  SkinCondition condition_non_st = ConditionFromFloat(wear, false);
  std::cout << kv.first << " : ";
  size_t i = 0;
  for (auto k : wear_config) {
    //      std::cout << ShortStringFromWeaponCondition(k.first) << " : " <<
    //      k.second << " ; ";
    array1[i] = k.second;
    std::cout << int(array1[i]) << " , ";
    i++;
  }
  EK_INFO("{:16X}", variations_key(array1, std::array<uint8_t, 8>{}));
  std::cout << "\n";
  return;

  tradeups.reserve(1024);

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

    for (size_t depth = 0; depth < g_MaxDepth; depth++) {
      size_t l_TradeUpCount = 0;
      const std::vector<std::vector<size_t>> combinations =
          combination(ids_by_rarity[rarity].size(), depth + 1);
      for (size_t i = 0; i < combinations.size(); i++) {
        // const std::vector<size_t> ids = combinations[i];
        for (auto partition : ps[depth]) {
          TradeUp tradeup(nRSkins, wear, stattrak, rarity, condition_non_st);

          for (size_t j = 0; j < depth + 1; j++) {
            const size_t id = ids_by_rarity[rarity][combinations[i][j]];
            const size_t p = partition[j];

            tradeup.mask_vector.insert(id) = p;
            tradeup.mask_big.insert(id * 5 + condition_non_st) = p;
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

void Calculator::Bruteforce(
    const std::vector<std::vector<WearType>> &wear_tuples) {
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
      BruteforceCondition(std::cref(*this), keyvalue, stattrak,
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
