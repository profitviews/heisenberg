#pragma once

#include <boost/json.hpp>
#include <string>

class OrderExecutor
{
public:
    OrderExecutor(){}
    virtual ~OrderExecutor(){}
    enum class Side { buy, sell };
    enum class OrderType { limit, market };
    virtual void new_order(const std::string& symbol, Side side, double orderQty, OrderType type, double) = 0;
    virtual boost::json::object result() const = 0;
};