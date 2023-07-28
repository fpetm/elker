#include "elker.hpp"
#include <motek/log.hpp>
#include <motek/skin.hpp>
#include <motek/tradeup.hpp>

#include <internal_use_only/config.hpp>

int main() {
	motek::Log::Init();

	EK_INFO("Welcome to elker v{}-{}", elker::cmake::elker_version, elker::cmake::git_hash);

	std::shared_ptr<motek::SkinDB> db = std::make_shared<motek::SkinDB>("./resources/skins.csv");
	motek::Calculator calc(db);

	calc.Bruteforce();
	
	return 0;
}
