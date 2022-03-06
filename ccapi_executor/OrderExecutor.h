#pragma once

#include <string>

class OrderExecutor
{
public:
    enum class Side { buy, sell };
    enum class OrderType { limit, market };

    OrderExecutor() = default;
    virtual ~OrderExecutor() = default;
    virtual void new_order(const std::string& symbol, Side side, double orderQty, OrderType type, double) = 0;
};