#include "elker.hpp"
#include <motek/calculator.hpp>
#include <motek/skin.hpp>

constexpr motek::wear_t g_wFN = motek::WearValueFromFloat(0.060);
constexpr motek::wear_t g_wMW = motek::WearValueFromFloat(0.110);
constexpr motek::wear_t g_wFT = motek::WearValueFromFloat(0.265);
constexpr motek::wear_t g_wWW = motek::WearValueFromFloat(0.415);
constexpr motek::wear_t g_wBS = motek::WearValueFromFloat(0.725);

const std::vector<std::vector<motek::wear_t>> g_WearTuples = {
    {g_wFN},        {g_wMW},        {g_wFT},        {g_wWW},        {g_wBS},
    {g_wFN, g_wMW}, {g_wMW, g_wFT}, {g_wFT, g_wWW}, {g_wWW, g_wBS},
};

int main() {
  motek::Log::Init();

  EK_INFO("Welcome to elker v{}-{}", elker::cmake::elker_version,
          elker::cmake::git_hash);

  EK_INFO("{} {} {} {} {}", g_wFN, g_wMW, g_wFT, g_wWW, g_wBS);

  std::shared_ptr<motek::SkinDB> database =
      std::make_shared<motek::SkinDB>("./resources/skins.csv");
  motek::Calculator calc(database);

  calc.Bruteforce(g_WearTuples, 1);

  return 0;
}
