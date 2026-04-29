#include <catch2/catch_test_macros.hpp>

#include <sandbox_order_executor.hpp>

using profitview::OrderType;
using profitview::SandboxOrderExecutor;
using profitview::Side;

TEST_CASE("SandboxOrderExecutor records a single new_order", "[e2e][sandbox]")
{
    SandboxOrderExecutor ex;
    ex.new_order("BTC-USD", Side::Buy, 1.5, OrderType::Limit, 100.0);

    auto const orders = ex.recorded_orders();
    REQUIRE(orders.size() == 1);
    REQUIRE(orders[0].symbol == "BTC-USD");
    REQUIRE(orders[0].side == Side::Buy);
    REQUIRE(orders[0].order_qty == 1.5);
    REQUIRE(orders[0].type == OrderType::Limit);
    REQUIRE(orders[0].price == 100.0);
}

TEST_CASE("SandboxOrderExecutor preserves FIFO order of calls", "[e2e][sandbox]")
{
    SandboxOrderExecutor ex;
    ex.new_order("A", Side::Buy, 1.0, OrderType::Market, 0.0);
    ex.new_order("B", Side::Sell, 2.0, OrderType::Limit, 50.0);

    auto const orders = ex.recorded_orders();
    REQUIRE(orders.size() == 2);
    REQUIRE(orders[0].symbol == "A");
    REQUIRE(orders[1].symbol == "B");
}
