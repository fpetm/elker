#include "collections.hpp"
#include <Eigen/Dense>

namespace elker {
	struct Calculator {
		SkinCollection *collections;
		unsigned int nCollections;

		Skin *skins;
		unsigned int nSkins;

		bool initialized;

		Eigen::VectorXf prices[SkinCondition::Max];
		Eigen::VectorXf factor[SkinCondition::Max];
		Eigen::MatrixXf transformer[SkinCondition::Max];
	};
	
	struct TradeUp {
		Eigen::VectorXf mask;
		SkinCondition condition;
	};

	float ExpectedValue(Calculator &calc, TradeUp &tradeup) {
		Eigen::VectorXf transformed = calc.transformer[tradeup.condition] * tradeup.mask;
		float factor = tradeup.mask.dot(calc.factor[tradeup.condition]);
		float returnprice = (transformed / factor).dot(calc.prices[tradeup.condition]);
		float tradeupcost = tradeup.mask.dot(calc.prices[tradeup.condition]);

		std::cout << returnprice << "/" << tradeupcost << "\n";
		return returnprice / tradeupcost;
	}

	// build calculator
	void BuildCalculator(Calculator &calculator) {
		if (calculator.collections == nullptr || calculator.nCollections == 0 || calculator.skins == nullptr || calculator.nSkins == 0) {
			return;
		}

		unsigned int counts[SkinCondition::Max] = { 0 };


		for (SkinCondition condition : {BS, WW, FT, MW, FN, BS_ST, WW_ST, FT_ST, MW_ST, FN_ST}) {
			calculator.prices[condition].resize(calculator.nSkins);
			calculator.factor[condition].resize(calculator.nSkins);
			calculator.transformer[condition].resize(calculator.nSkins, calculator.nSkins);

			for (unsigned int i = 0; i < calculator.nSkins; i++) {
				calculator.prices[condition](i) = 0;
				calculator.factor[condition](i) = 0;

				for (unsigned int j = 0; j < calculator.nSkins; j++) {
					calculator.transformer[condition](i, j) = 0;
				}
			}
		}

		for (unsigned int i = 0; i < calculator.nSkins; i++) {
			const Skin& skin = calculator.skins[i];

			calculator.prices[skin.condition](skin.id) = skin.price;

			counts[skin.condition]++;
		}

		for (unsigned int i = 0; i < calculator.nCollections; i++) {
			const SkinCollection& collection = calculator.collections[i];

			for (SkinCondition condition : {BS, WW, FT, MW, FN, BS_ST, WW_ST, FT_ST, MW_ST, FN_ST}) {
				auto& factor = calculator.factor[condition];
				auto& transformer = calculator.transformer[condition];


				for (SkinRarity rarity : {Consumer, Industrial, MilSpec, Restricted, Classified}) {
					SkinRarity higher = (SkinRarity)(rarity + 1);

					unsigned int higherc = 0;

					for (int j = 0; j < collection.nSkins; j++) {
						higherc += collection.skins[j]->rarity == higher && collection.skins[j]->condition == condition;
					}

					for (int j = 0; j < collection.nSkins; j++) {
						if (collection.skins[j]->rarity == rarity && collection.skins[j]->condition == condition) {
							factor(collection.skins[j]->id) = higherc;
							for (unsigned int k = 0; k < collection.nSkins; k++) {
								if (collection.skins[k]->rarity == higher && collection.skins[k]->condition == condition && collection.skins[j]->condition == condition) {
									transformer(collection.skins[k]->id, collection.skins[j]->id) = 1;
								}
							}
						}
					}
				}
			}
		}


		calculator.initialized = true;
	}
}