#pragma once

#include "OrderExecutor.h"

#include <boost/json.hpp>
#include <string>

using Side = OrderExecutor::Side;
using OrderType = OrderExecutor::OrderType;

class Exchange {
public:
    virtual boost::json::object new_order(const std::string& symbol, Side side, double orderQty, OrderType type, double price = -1.0) = 0;

    virtual ~Exchange(){};
};
