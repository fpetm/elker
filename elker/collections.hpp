#pragma once
#include "skin.hpp"
#include <utility>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

namespace elker {
    static Skin g_Skins[16384];
	static unsigned int g_NSkins = 0;
    static SkinCollection g_Collections[128];
	static unsigned int g_NCollections = 0;
	static bool g_SkinsInit = false;

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

	void InitSkins(const char *skinpath = "C:/prog/elker/script/skins.csv") {
        auto data = parseCSV(skinpath);

        for (auto& row : data) {
            if (row[0] == "NAME") continue;
            //std::cout << row[0] << "\n";
            std::string name = row[0];
            WeaponType type = WeaponType::None;
            SkinRarity rarity;
            std::string collection = row[1];
            float price[10];

            bool good = false;
            for (SkinCondition condition : {BS, WW, FT, MW, FN, BS_ST, WW_ST, FT_ST, MW_ST, FN_ST}) {
                price[condition] = std::stof(row[4 + (int)condition]);
                good |= price[condition] != -1;
            }
            if (!good) { continue; }
            
            if (g_NCollections == 0) {
                g_Collections[0] = { collection , 0};
                g_NCollections++;
            } else if (g_Collections[g_NCollections - 1].name != collection) {
                g_Collections[g_NCollections] = { collection , g_NCollections };
                g_NCollections++;
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
                g_Skins[g_NSkins] = { name.c_str(), &g_Collections[g_NCollections - 1], price[0 + condition], rarity, condition, type, g_NSkins};
                g_Collections[g_NCollections - 1].skins[g_Collections[g_NCollections - 1].nSkins] = &g_Skins[g_NSkins];

                g_NSkins++;
                g_Collections[g_NCollections - 1].nSkins++;
            }
        }

		g_SkinsInit = true;
	}

	std::pair<SkinCollection*, unsigned int> GetCollections () {
		return std::pair<SkinCollection*, unsigned int>(g_Collections, g_NCollections);
	}

	std::pair<Skin*, unsigned int> GetSkins() {
		return std::pair<Skin*, unsigned int>(g_Skins, g_NSkins);
	}
}