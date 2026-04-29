#include <catch2/catch_test_macros.hpp>

#include <enum.hpp>
#include <order_type.hpp>
#include <side.hpp>

using profitview::fromString;
using profitview::OrderType;
using profitview::Side;
using profitview::toString;

TEST_CASE("Side toString / fromString", "[enum]")
{
    REQUIRE(toString(Side::Buy) == "Buy");
    REQUIRE(toString(Side::Sell) == "Sell");
    REQUIRE(fromString<Side>("Buy") == Side::Buy);
    REQUIRE(fromString<Side>("sell") == Side::Sell);
    REQUIRE_FALSE(fromString<Side>("bogus").has_value());
}

TEST_CASE("OrderType toString / fromString", "[enum]")
{
    REQUIRE(toString(OrderType::Limit) == "Limit");
    REQUIRE(toString(OrderType::Market) == "Market");
    REQUIRE(fromString<OrderType>("Limit") == OrderType::Limit);
    REQUIRE(fromString<OrderType>("market") == OrderType::Market);
    REQUIRE_FALSE(fromString<OrderType>("stop").has_value());
}
