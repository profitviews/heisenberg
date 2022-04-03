#pragma once

#include "order_type.hpp"
#include "side.hpp"

#include <string>

namespace profitview
{

class OrderExecutor
{
public:
    OrderExecutor() = default;
    virtual ~OrderExecutor() = default;
    virtual void new_order(std::string const& symbol, Side side, double orderQty, OrderType type, double) = 0;
};

}    // namespace profitview
