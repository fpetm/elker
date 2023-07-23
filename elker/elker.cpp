#include "elker.hpp"
#include "skin.hpp"
#include "collections.hpp"
#include "tradeup.hpp"

int main()
{
	std::cout << "hello world" << std::endl;
	elker::InitSkins();

	auto collections = elker::GetCollections();
	auto skins = elker::GetSkins();

	elker::Calculator calculator;

	calculator.collections = collections.first;
	calculator.nCollections = collections.second;
	calculator.skins = skins.first;
	calculator.nSkins = skins.second;

	elker::BuildCalculator(calculator);

	elker::TradeUp tradeup;

	tradeup.mask.resize(340);

	for (int i = 0; i < 340; i++) {
		tradeup.mask(i) = 0;
	}



	elker::Skin *skinlist = skins.first;
	for (int i = 0; i < skins.second; i++) {
		if (skinlist[i].name == "Banana Cannon" && skinlist[i].condition == elker::FN) {
			std::cout << skinlist[i].id << "\n";
			tradeup.mask(skinlist[i].id) = 10;
		}
	}

	tradeup.condition = elker::FN;

	std::cout << elker::ExpectedValue(calculator, tradeup) << "\n";

	return 0;
}
