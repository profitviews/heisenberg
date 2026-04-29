#include <catch2/catch_test_macros.hpp>

#include <exchange_names.hpp>

#include <ccapi_cpp/ccapi_macro.h>

using profitview::ccapi_exchange_from_cli;

TEST_CASE("ccapi_exchange_from_cli aliases", "[exchange]")
{
    REQUIRE(ccapi_exchange_from_cli("kraken") == CCAPI_EXCHANGE_NAME_KRAKEN);
    REQUIRE(ccapi_exchange_from_cli("coinbase") == CCAPI_EXCHANGE_NAME_COINBASE);
    REQUIRE(ccapi_exchange_from_cli("ftx") == CCAPI_EXCHANGE_NAME_FTX);
    REQUIRE(ccapi_exchange_from_cli("bitmex") == CCAPI_EXCHANGE_NAME_BITMEX);
}

TEST_CASE("ccapi_exchange_from_cli pass-through unknown venue", "[exchange]")
{
    REQUIRE(ccapi_exchange_from_cli("customvenue") == std::string{"customvenue"});
}
