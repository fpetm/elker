﻿#include "elker.hpp"
#include "skin.hpp"
#include "tradeup.hpp"
#include "log.hpp"

int main() {
	elker::Log::Init();

	std::shared_ptr<elker::SkinDB> db = std::make_shared<elker::SkinDB>();
	elker::Calculator calc(db);

	TradeUp t("BAAAABJrYg1iTazlYk2sAWJNrA1iTawB", db->m_SkinIDsByRarity[elker::MilSpec].size(), 0);
	calc.Bruteforce();

	return 0;
}
