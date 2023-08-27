#include "elker.hpp"
#include <motek/calculator.hpp>
#include <motek/log.hpp>
#include <motek/skin.hpp>

#include <internal_use_only/config.hpp>

constexpr motek::wear_t g_wFN = motek::WearValueFromFloat(0.060);
constexpr motek::wear_t g_wMW = motek::WearValueFromFloat(0.110);
constexpr motek::wear_t g_wFT = motek::WearValueFromFloat(0.265);
constexpr motek::wear_t g_wWW = motek::WearValueFromFloat(0.415);
constexpr motek::wear_t g_wBS = motek::WearValueFromFloat(0.725);

int main() {
  motek::Log::Init();

  EK_INFO("Welcome to elker v{}-{}", elker::cmake::elker_version,
          elker::cmake::git_hash);

  EK_INFO("{} {} {} {} {}", g_wFN, g_wMW, g_wFT, g_wWW, g_wBS);

  auto wear_vars = motek::generate_wear_variations({
      {g_wFN}, {g_wMW}, {g_wFT}, {g_wWW}, {g_wBS},
      {g_wFN, g_wMW},
      {g_wMW, g_wFT},
      {g_wFT, g_wWW},
      {g_wWW, g_wBS},
    }, false);

  for (auto wv : wear_vars) {
    std::cout << wv.first << " : ";

    for (auto kv : wv.second) {
      std::cout << motek::ShortStringFromWeaponCondition(kv.first) << ":" << kv.second << " ; ";
    }

    std::cout << std::endl;
  }

  return 0;

  std::shared_ptr<motek::SkinDB> db =
      std::make_shared<motek::SkinDB>("./resources/skins.csv");
  motek::Calculator calc(db);

  calc.Bruteforce(
      {
          //{60},{110},
          {60, 110},
          {110, 60},
          {110, 265},
          {265, 110},
      },
      2);

  return 0;
}
