#include <catch2/catch_test_macros.hpp>

#include <motek/calculator.hpp>

TEST_CASE("Database is tested", "[database]")
{
  motek::Log::Init();
  std::shared_ptr<motek::SkinDB> database = std::make_shared<motek::SkinDB>("./resources/skins.csv");
  REQUIRE(!database->m_Skins.empty());
}
