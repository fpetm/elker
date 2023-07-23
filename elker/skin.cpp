#include "skin.hpp"
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

namespace elker {
    std::vector<std::vector<std::string>> parseCSV(const std::string& filename) {
        std::vector<std::vector<std::string>> data;

        // Open the file
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open the file: " << filename << std::endl;
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

        // Close the file
        file.close();

        return data;
    }

    static WeaponType WeaponTypeFromString(std::string s) {
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

        std::cout << s << "\n";
        return WeaponType::None;
    }

    SkinDB::SkinDB(std::string skinpath) {
        auto data = parseCSV(skinpath);

        for (auto& row : data) {
            if (row[0] == "NAME") continue;
            //std::cout << row[0] << "\n";
            std::string name = row[0];
            WeaponType type = WeaponTypeFromString(row[3]);
            SkinRarity rarity = Contraband;
            std::string collection = row[1];
            float price[10] = { 0 };

            bool good = false;
            for (SkinCondition condition : {BS, WW, FT, MW, FN, BS_ST, WW_ST, FT_ST, MW_ST, FN_ST}) {
                price[condition] = std::stof(row[4 + (int)condition]);
                good |= price[condition] != -1;
            }
            if (!good) { continue; }

            if (m_Collections.size() == 0) {
                m_Collections.push_back(SkinCollection(collection, 0));
            }
            else if (m_Collections[m_Collections.size()-1].m_Name != collection) {
                m_Collections.push_back(SkinCollection(collection, m_Collections.size()));
            }

            if (row[2] == "Consumer Grade") rarity = SkinRarity::Consumer;
            else if (row[2] == "Industrial Grade") rarity = SkinRarity::Industrial;
            else if (row[2] == "Mil-Spec") rarity = SkinRarity::MilSpec;
            else if (row[2] == "Restricted") rarity = SkinRarity::Restricted;
            else if (row[2] == "Classified") rarity = SkinRarity::Classified;
            else if (row[2] == "Covert") rarity = SkinRarity::Covert;
            else if (row[2] == "Contraband") rarity = SkinRarity::Contraband;
            else std::cout << row[0] << " : " << "unknown rarity " << row[2] << "\n";

            for (SkinCondition condition : {BS, WW, FT, MW, FN, BS_ST, WW_ST, FT_ST, MW_ST, FN_ST}) {
                m_Collections[m_Collections.size() - 1].AddSkin(Skin(name, price[condition], rarity, condition, type, m_Skins.size(), m_Skins.size()/10));
                m_Skins.push_back(m_Collections[m_Collections.size() - 1].LastSkin());
            }
        }
    }
}