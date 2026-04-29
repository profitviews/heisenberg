#pragma once

#include "order_executor.hpp"

#include <mutex>
#include <string>
#include <vector>

namespace profitview
{

/** Deterministic in-process stand-in for a venue; records orders with no network or ccapi. */
struct RecordedOrder
{
    std::string symbol;
    Side side{};
    double order_qty{};
    OrderType type{};
    double price{};
};

class SandboxOrderExecutor : public OrderExecutor
{
public:
    void new_order(std::string const& symbol, Side side, double orderQty, OrderType type, double price = 0.0) override
    {
        std::lock_guard<std::mutex> lock(mu_);
        orders_.push_back(RecordedOrder{symbol, side, orderQty, type, price});
    }

    std::vector<RecordedOrder> recorded_orders() const
    {
        std::lock_guard<std::mutex> lock(mu_);
        return orders_;
    }

private:
    mutable std::mutex mu_;
    std::vector<RecordedOrder> orders_;
};

}    // namespace profitview
