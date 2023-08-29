#include "elker.hpp"
#include <motek/calculator.hpp>
#include <motek/combinatorics.hpp>
#include <motek/skin.hpp>
#include <vector>

const std::vector<double> g_Points = {0.06, 0.11, 0.16, 0.21, 0.26, 0.31,
                                      0.36, 0.41, 0.46, 0.51, 0.7};

int main() {
  std::vector<std::vector<motek::wear_t>> wear_tuples;
  for (int depth = 1; depth <= 2; depth++) {
    using namespace motek::combinatorics;
    auto combination = first_combination(g_Points.size(), depth);
    for (int i = 0; i < nCr(g_Points.size(), depth); i++) {
      std::vector<motek::wear_t> tuple;
      for (int j = 0; j < depth; j++) {
        tuple.push_back(motek::WearValueFromFloat(g_Points[combination[j]]));
      }
      wear_tuples.push_back(tuple);

      next_combination(combination, depth);
    }
  }

  constexpr motek::wear_t g_wFN = motek::WearValueFromFloat(0.060);
  constexpr motek::wear_t g_wMW = motek::WearValueFromFloat(0.110);
  constexpr motek::wear_t g_wFT = motek::WearValueFromFloat(0.23);
  constexpr motek::wear_t g_wWW = motek::WearValueFromFloat(0.410);
  constexpr motek::wear_t g_wBS = motek::WearValueFromFloat(0.720);

  const std::vector<std::vector<motek::wear_t>> g_WearTuples = {
      {g_wFN},        {g_wMW},        {g_wFT},        {g_wWW},        {g_wBS},
      {g_wFN, g_wMW}, {g_wMW, g_wFT}, {g_wFT, g_wWW}, {g_wWW, g_wBS},
  };

  // less go
  motek::Log::Init();

  EK_INFO("Welcome to elker v{}-{}", elker::cmake::elker_version,
          elker::cmake::git_hash);

  std::shared_ptr<motek::SkinDB> database =
      std::make_shared<motek::SkinDB>("./resources/skins.csv");
  std::unique_ptr<motek::Calculator> calc =
      std::make_unique<motek::Calculator>(database);

  auto tradeups = calc->Bruteforce(g_WearTuples, 2, true);
  calc->ExportTradeUps(tradeups, "./out.csv");

  return 0;
}
