#include "elker.hpp"
#include <motek/calculator.hpp>
#include <motek/log.hpp>
#include <motek/skin.hpp>

#include <internal_use_only/config.hpp>

int main() {
  motek::Log::Init();

  EK_INFO("Welcome to elker v{}-{}", elker::cmake::elker_version,
          elker::cmake::git_hash);

#if 0
  auto wear_vars = motek::generate_wear_variations({
    {60, 110},
    {110, 60},
    {110, 265},
    {265, 110},}, false);

  for (auto wv : wear_vars) {
    std::cout << wv.first << " : ";

    for (auto kv : wv.second) {
      std::cout << motek::ShortStringFromWeaponCondition(kv.first) << ":" << kv.second << " ; ";
    }

    std::cout << std::endl;
  }

  return 0;
#endif

  std::shared_ptr<motek::SkinDB> db =
      std::make_shared<motek::SkinDB>("./resources/skins.csv");
  motek::Calculator calc(db);

  calc.Bruteforce({
      //{60},{110},
      {60, 110},
      {110, 60},
      {110, 265},
      {265, 110},
  });

  return 0;
}
