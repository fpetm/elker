#pragma once
#include <array>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

namespace motek {
typedef short WearType;
constexpr WearType g_WearRangeMin = 0;
constexpr WearType g_WearRangeMax = 1000;

constexpr WearType g_WearRange1 =
    WearType(0.07 * (g_WearRangeMax - g_WearRangeMin) + g_WearRangeMin);
constexpr WearType g_WearRange2 =
    WearType(0.15 * (g_WearRangeMax - g_WearRangeMin) + g_WearRangeMin);
constexpr WearType g_WearRange3 =
    WearType(0.38 * (g_WearRangeMax - g_WearRangeMin) + g_WearRangeMin);
constexpr WearType g_WearRange4 =
    WearType(0.45 * (g_WearRangeMax - g_WearRangeMin) + g_WearRangeMin);

constexpr double WearValueToFloat(WearType wear) {
  return float(wear) / float(g_WearRangeMax);
}

enum WeaponClass {
  Knives = 1 << 8,
  Pistol = 1 << 9,
  Heavy = 1 << 10,
  SMG = 1 << 11,
  Rifle = 1 << 12,
};

enum WeaponType {
  None = 0,

  Knife = WeaponClass::Knives | 1,

  CZ75Auto = WeaponClass::Pistol | 1,
  DesertEagle = WeaponClass::Pistol | 2,
  DualBerettas = WeaponClass::Pistol | 3,
  FiveSeVen = WeaponClass::Pistol | 4,
  Glock18 = WeaponClass::Pistol | 5,
  P2000 = WeaponClass::Pistol | 6,
  P250 = WeaponClass::Pistol | 7,
  R8Revolver = WeaponClass::Pistol | 8,
  Tec9 = WeaponClass::Pistol | 9,
  USPS = WeaponClass::Pistol | 10,

  MAG7 = WeaponClass::Heavy | 1,
  Nova = WeaponClass::Heavy | 2,
  SawedOff = WeaponClass::Heavy | 3,
  XM1014 = WeaponClass::Heavy | 4,
  M249 = WeaponClass::Heavy | 5,
  Negev = WeaponClass::Heavy | 6,

  MAC10 = WeaponClass::SMG | 1,
  MP5SD = WeaponClass::SMG | 2,
  MP7 = WeaponClass::SMG | 3,
  MP9 = WeaponClass::SMG | 4,
  P90 = WeaponClass::SMG | 5,
  PPBizon = WeaponClass::SMG | 6,
  UMP45 = WeaponClass::SMG | 7,

  AK47 = WeaponClass::Rifle | 1,
  AUG = WeaponClass::Rifle | 2,
  FAMAS = WeaponClass::Rifle | 3,
  Galil = WeaponClass::Rifle | 4,
  M4A1S = WeaponClass::Rifle | 5,
  M4A4 = WeaponClass::Rifle | 6,
  SG553 = WeaponClass::Rifle | 7,
  AWP = WeaponClass::Rifle | 8,
  SSG08 = WeaponClass::Rifle | 9,
  G3SG1 = WeaponClass::Rifle | 10,
  SCAR20 = WeaponClass::Rifle | 11,
};

enum SkinRarity {
  Consumer,   // Gray
  Industrial, // Blue
  MilSpec,    // Dark Blue
  Restricted, // Purple
  Classified, // Pink
  Covert,     // Red

  Contraband, // Howl
};

enum SkinCondition {
  BS = 0,
  WW = 1,
  FT = 2,
  MW = 3,
  FN = 4,

  BS_ST = 5,
  WW_ST = 6,
  FT_ST = 7,
  MW_ST = 8,
  FN_ST = 9,

  Max = 10,
};

class Skin {
public:
  Skin(std::string name, std::array<float, SkinCondition::Max> prices_sell,
       std::array<float, SkinCondition::Max> prices_buy, SkinRarity rarity,
       WeaponType weapontype, WearType wmin, WearType wmax, size_t id,
       size_t cid, size_t rid)
      : m_Name(name), m_PricesSell(prices_sell), m_PricesBuy(prices_buy),
        m_Rarity(rarity), m_WeaponType(weapontype), wear_min(wmin),
        wear_max(wmax), m_ID(id), m_CollectionID(cid), m_rID(rid) {
    m_Banned = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  }

public:
  std::string m_Name;
  std::array<float, SkinCondition::Max> m_PricesSell;
  std::array<float, SkinCondition::Max> m_PricesBuy;
  SkinRarity m_Rarity;
  WeaponType m_WeaponType;
  WearType wear_min, wear_max;
  size_t m_ID, m_CollectionID, m_rID;
  std::array<bool, SkinCondition::Max> m_Banned;
};

class SkinCollection {
public:
  SkinCollection(const std::string &name, size_t id)
      : m_Name(name), m_ID(id), m_HighestRarity(SkinRarity::Covert) {}
  void AddSkin(Skin s) { m_Skins.push_back(s); }

  Skin &LastSkin() { return m_Skins[m_Skins.size() - 1]; }

  size_t Amount() const { return m_Skins.size(); }

  Skin &operator[](size_t n) { return m_Skins[n]; }

  std::string m_Name = "";
  size_t m_ID;
  SkinRarity m_HighestRarity;
  std::vector<Skin> m_SkinsByRarity[SkinRarity::Contraband + 1];

  using container = std::vector<Skin>;
  using iterator = typename container::iterator;
  using const_iterator = typename container::const_iterator;

  iterator begin() { return m_Skins.begin(); }
  iterator end() { return m_Skins.end(); }
  const_iterator begin() const { return m_Skins.begin(); }
  const_iterator end() const { return m_Skins.end(); }

private:
  std::vector<Skin> m_Skins;
};

class SkinDB {
public:
  SkinDB(std::string skinpath);
  void AddCollection(SkinCollection &collection) {
    m_Collections.push_back(collection);
    for (auto &skin : collection) {
      m_Skins.push_back(skin);
    }
  }

  std::vector<Skin> GetSkins() { return m_Skins; }
  std::vector<SkinCollection> GetCollections() { return m_Collections; }

public:
  std::vector<SkinCollection> m_Collections;
  std::vector<Skin> m_Skins;

  std::vector<size_t> m_SkinIDsByRarity[SkinRarity::Contraband + 1];
};

WeaponType WeaponTypeFromString(std::string s);
std::string StringFromWeaponType(WeaponType type);
std::string StringFromWeaponCondition(SkinCondition condition);
std::string ShortStringFromWeaponCondition(SkinCondition condition);
SkinCondition ConditionFromFloat(WearType wear, bool st);
SkinCondition MapCondition(const Skin &skin, WearType wear, bool stattrak);
} // namespace motek
