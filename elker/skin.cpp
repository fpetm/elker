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

    SkinDB::SkinDB(std::string skinpath) {
        auto data = parseCSV(skinpath);

        for (auto& row : data) {
            if (row[0] == "NAME") continue;
            //std::cout << row[0] << "\n";
            std::string name = row[0];
            WeaponType type = WeaponType::None;
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