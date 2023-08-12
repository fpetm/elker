#include <motek/skin.hpp>
#include <motek/log.hpp>

#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

namespace motek {
    std::vector<std::vector<std::string>> parseCSV(const std::string& filename) {
        std::vector<std::vector<std::string>> data;

        // Open the file
        std::ifstream file(filename);
        if (!file.is_open()) {
            MT_ERROR("Failed to open CSV ({})", filename);
            return data;
        }

        std::string line;
        while (std::getline(file, line)) {
            std::vector<std::string> row;
            std::stringstream lineStream(line);
            std::string cell;

            while (std::getline(lineStream, cell, ',')) {
                row.push_back(cell);
            }

            data.push_back(row);
        }
        MT_INFO("Succesfully parsed CSV ({})!", filename);

        // Close the file
        file.close();

        return data;
    }

    WeaponType WeaponTypeFromString(std::string s) {
        if (s == "CZ75-Auto") return WeaponType::CZ75Auto;
        if (s == "Desert Eagle") return WeaponType::DesertEagle;
        if (s == "Dual Berettas") return WeaponType::DualBerettas;
        if (s == "Five-SeveN") return WeaponType::FiveSeVen;
        if (s == "Glock-18") return WeaponType::Glock18;
        if (s == "P2000") return WeaponType::P2000;
        if (s == "P250") return WeaponType::P250;
        if (s == "R8 Revolver") return WeaponType::R8Revolver;
        if (s == "Tec-9") return WeaponType::Tec9;
        if (s == "USP-S") return WeaponType::USPS;
        if (s == "MAG-7") return WeaponType::MAG7;
        if (s == "Nova") return WeaponType::Nova;
        if (s == "Sawed-Off") return WeaponType::SawedOff;
        if (s == "XM1014") return WeaponType::XM1014;
        if (s == "M249") return WeaponType::M249;
        if (s == "Negev") return WeaponType::Negev;
        if (s == "MAC-10") return WeaponType::MAC10;
        if (s == "MP5-SD") return WeaponType::MP5SD;
        if (s == "MP7") return WeaponType::MP7;
        if (s == "MP9") return WeaponType::MP9;
        if (s == "P90") return WeaponType::P90;
        if (s == "PP-Bizon") return WeaponType::PPBizon;
        if (s == "UMP-45") return WeaponType::UMP45;
        if (s == "AK-47") return WeaponType::AK47;
        if (s == "AUG") return WeaponType::AUG;
        if (s == "FAMAS") return WeaponType::FAMAS;
        if (s == "Galil AR") return WeaponType::Galil;
        if (s == "M4A1-S") return WeaponType::M4A1S;
        if (s == "M4A4") return WeaponType::M4A4;
        if (s == "SG 553") return WeaponType::SG553;
        if (s == "AWP") return WeaponType::AWP;
        if (s == "SSG 08") return WeaponType::SSG08;
        if (s == "G3SG1") return WeaponType::G3SG1;
        if (s == "SCAR-20") return WeaponType::SCAR20;
        return WeaponType::None;
    }

    std::string StringFromWeaponType(WeaponType type) {
        switch (type) {
        case WeaponType::CZ75Auto: return "CZ75-Auto";
        case WeaponType::DesertEagle: return "Desert Eagle";
        case WeaponType::DualBerettas: return "Dual Berettas";
        case WeaponType::FiveSeVen: return "Five-SeveN";
        case WeaponType::Glock18: return "Glock-18";
        case WeaponType::P2000: return "P2000";
        case WeaponType::P250: return "P250";
        case WeaponType::R8Revolver: return "R8 Revolver";
        case WeaponType::Tec9: return "Tec-9";
        case WeaponType::USPS: return "USP-S";
        case WeaponType::MAG7: return "MAG-7";
        case WeaponType::Nova: return "Nova";
        case WeaponType::SawedOff: return "Sawed-Off";
        case WeaponType::XM1014: return "XM1014";
        case WeaponType::M249: return "M249";
        case WeaponType::Negev: return "Negev";
        case WeaponType::MAC10: return "MAC-10";
        case WeaponType::MP5SD: return "MP5-SD";
        case WeaponType::MP7: return "MP7";
        case WeaponType::MP9: return "MP9";
        case WeaponType::P90: return "P90";
        case WeaponType::PPBizon: return "PP-Bizon";
        case WeaponType::UMP45: return "UMP-45";
        case WeaponType::AK47: return "AK-47";
        case WeaponType::AUG: return "AUG";
        case WeaponType::FAMAS: return "FAMAS";
        case WeaponType::Galil: return "Galil AR";
        case WeaponType::M4A1S: return "M4A1-S";
        case WeaponType::M4A4: return "M4A4";
        case WeaponType::SG553: return "SG 553";
        case WeaponType::AWP: return "AWP";
        case WeaponType::SSG08: return "SSG 08";
        case WeaponType::G3SG1: return "G3SG1";
        case WeaponType::SCAR20: return "SCAR-20";
        case WeaponType::None: return "None";
        case WeaponType::Knife: return "Knife";
        default: {
            return "ERROR[" + std::to_string((int)type) + "]";
        };
        }
    }

    std::string StringFromWeaponCondition(SkinCondition condition) {
        switch (condition) {
        case SkinCondition::BS: return "Battle-Scarred";
        case SkinCondition::WW: return "Well-Worn";
        case SkinCondition::FT: return "Field-Tested";
        case SkinCondition::MW: return "Minimal Wear";
        case SkinCondition::FN: return "Factory New";
        case SkinCondition::BS_ST: return "StatTrak Battle-scarred";
        case SkinCondition::WW_ST: return "StatTrak Well-worn";
        case SkinCondition::FT_ST: return "StatTrak Field-tested";
        case SkinCondition::MW_ST: return "StatTrak Minimal Wear";
        case SkinCondition::FN_ST: return "StatTrak Factory New";
        case SkinCondition::Max: return "MAX";
        default: {
            return "ERROR[" + std::to_string((int)condition) + "]";
        };
        }
    }

    std::string ShortStringFromWeaponCondition(SkinCondition condition) {
        switch (condition) {
        case SkinCondition::BS: return "BS";
        case SkinCondition::WW: return "WW";
        case SkinCondition::FT: return "FT";
        case SkinCondition::MW: return "MW";
        case SkinCondition::FN: return "FN";
        case SkinCondition::BS_ST: return "BS ST";
        case SkinCondition::WW_ST: return "WW ST";
        case SkinCondition::FT_ST: return "FT ST";
        case SkinCondition::MW_ST: return "MW ST";
        case SkinCondition::FN_ST: return "FN ST";
        case SkinCondition::Max: return "MAX";
        default: {
            return "ERROR[" + std::to_string((int)condition) + "]";
        };
        }
    }

    SkinCondition ConditionFromFloat(WearType wear, bool st) {
        if (st) {
            if (wear >= g_WearRangeMin   && wear <= g_WearRange1) return SkinCondition::FN_ST;
            else if (wear > g_WearRange1 && wear <= g_WearRange2) return SkinCondition::MW_ST;
            else if (wear > g_WearRange2 && wear <= g_WearRange3) return SkinCondition::FT_ST;
            else if (wear > g_WearRange3 && wear <= g_WearRange4) return SkinCondition::WW_ST;
            else if (wear > g_WearRange4 && wear <= g_WearRangeMax) return SkinCondition::BS_ST;
        } else {
            if (wear >= g_WearRangeMin   && wear <= g_WearRange1) return SkinCondition::FN;
            else if (wear > g_WearRange1 && wear <= g_WearRange2) return SkinCondition::MW;
            else if (wear > g_WearRange2 && wear <= g_WearRange3) return SkinCondition::FT;
            else if (wear > g_WearRange3 && wear <= g_WearRange4) return SkinCondition::WW;
            else if (wear > g_WearRange4 && wear <= g_WearRangeMax) return SkinCondition::BS;
        }
        return SkinCondition::Max;
    }

    SkinCondition MapCondition(const Skin& skin, WearType wear, bool stattrak) {
        const WearType newfloat = WearType((skin.wear_max - skin.wear_min) * (wear/double(g_WearRangeMax-g_WearRangeMin)) + skin.wear_min);
        return ConditionFromFloat(newfloat, stattrak);
    }

    inline bool CollectionFilter(std::string collectionname) {
        (void)collectionname;
        return true;
    }

    SkinDB::SkinDB(std::string skinpath) {
        auto data = parseCSV(skinpath);

        for (auto& row : data) {
            if (row[0] == "NAME") continue;
            std::string collection = row[1];

            if (!CollectionFilter(collection)) continue;
            if (m_Collections.size() == 0) {
                m_Collections.push_back(SkinCollection(collection, 0));
            }
            else if (m_Collections[m_Collections.size() - 1].m_Name != collection) {
                m_Collections.push_back(SkinCollection(collection, m_Collections.size()));
            }
        }

        for (auto& row : data) {
            if (row[0] == "NAME") continue;
            std::string name = row[0];
            WeaponType type = WeaponTypeFromString(row[3]);
            SkinRarity rarity = Contraband;
            std::string collection = row[1];
            std::array<float, SkinCondition::Max> price_sell = { 0 }, price_buy = { 0 };
            float wear_min = std::stof(row[4]);
            float wear_max = std::stof(row[5]);

            if (!CollectionFilter(collection)) continue;

            for (SkinCondition condition : {BS, WW, FT, MW, FN, BS_ST, WW_ST, FT_ST, MW_ST, FN_ST}) {
                if (row[6 + (int)condition] == "") {
                    price_sell[condition] = -1;
                    continue;
                }
                price_sell[condition] = std::stof(row[6 + (int)condition]);
            }
            for (SkinCondition condition : {BS, WW, FT, MW, FN, BS_ST, WW_ST, FT_ST, MW_ST, FN_ST}) {
                if (row[16 + (int)condition] == "") {
                    price_buy[condition] = -1;
                    continue;
                }
                price_buy[condition] = std::stof(row[16 + (int)condition]);
            }

            if (row[2] == "Consumer Grade") rarity = SkinRarity::Consumer;
            else if (row[2] == "Industrial Grade") rarity = SkinRarity::Industrial;
            else if (row[2] == "Mil-Spec Grade") rarity = SkinRarity::MilSpec;
            else if (row[2] == "Restricted") rarity = SkinRarity::Restricted;
            else if (row[2] == "Classified") rarity = SkinRarity::Classified;
            else if (row[2] == "Covert") rarity = SkinRarity::Covert;
            else if (row[2] == "Contraband") continue;
            else MT_ERROR("Unknown rarity (skin: {}): {}", row[0], row[2]);

            for (SkinCollection& coll : m_Collections) {
                if (coll.m_Name == collection) {
                    coll.AddSkin(Skin(name, price_sell, price_buy, rarity, type, WearType(wear_min*g_WearRangeMax), WearType(wear_max*g_WearRangeMax), m_Skins.size(), coll.m_ID, m_SkinIDsByRarity[rarity].size()));
                    m_SkinIDsByRarity[rarity].push_back(m_Skins.size());
                    m_Skins.push_back(coll.LastSkin());
                }
            }

        }

        for (SkinCollection &collection : m_Collections) {
            SkinRarity highest = SkinRarity::Consumer;
            for (Skin skin : collection) {
                highest = highest < skin.m_Rarity ? skin.m_Rarity : highest;
            }
            collection.m_HighestRarity = highest;
            for (SkinCondition condition : {BS, WW, FT, MW, FN, BS_ST, WW_ST, FT_ST, MW_ST, FN_ST}) {
                for (Skin &skin : collection) {
                    if (skin.m_Rarity >= collection.m_HighestRarity || skin.m_PricesSell[condition] <= 0.03 || skin.m_PricesBuy[condition] <= 0.03) {
                        skin.m_Banned[condition] = true;
                        m_Skins[skin.m_ID].m_Banned[condition] = true;
                    }
                    else {
                        skin.m_Banned[condition] = false;
                        m_Skins[skin.m_ID].m_Banned[condition] = false;
                    }
                }
            }
            for (Skin& skin : collection) {
                collection.m_SkinsByRarity[skin.m_Rarity].push_back(skin);
            }
        }

        MT_INFO("Succesfully loaded {} skins and {} collections!", m_Skins.size(), m_Collections.size());
    }
}
