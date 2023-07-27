#include "elker.hpp"
#include "skin.hpp"
#include "tradeup.hpp"
#include "log.hpp"

#include <internal_use_only/config.hpp>

int main() {
	elker::Log::Init();

	std::shared_ptr<elker::SkinDB> db = std::make_shared<elker::SkinDB>();
	elker::Calculator calc(db);

	calc.Bruteforce();

	return 0;
}
