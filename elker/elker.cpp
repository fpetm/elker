#include "elker.hpp"
#include "skin.hpp"
#include "tradeup.hpp"

int main()
{
	std::cout << "hello world" << std::endl;

	std::shared_ptr<elker::SkinDB> db = std::make_shared<elker::SkinDB>();
	elker::Calculator calc(db);

	elker::TradeUp tradeup;

	tradeup.mask.resize(34);
	for (int i = 0; i < 34; i++) {
		tradeup.mask(i) = 0;
	}

	for (const elker::Skin &skin : db->GetSkins())
		if (skin.m_Name == "Banana Cannon")
			tradeup.mask(skin.m_ID) = 10;

	tradeup.condition = elker::FN;

	std::cout << calc.ExpectedValue(tradeup) << "\n" << "\n";

	calc.Bruteforce();

	return 0;
}
