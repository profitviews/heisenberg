#pragma once

#include "OrderExecutor.h"
#include "order_type.hpp"
#include "side.hpp"

#include <boost/json.hpp>
#include <string>

namespace profitview 
{

class Exchange {
public:
    virtual ~Exchange() = default;
    virtual boost::json::object new_order(std::string const& symbol, Side side, double orderQty, OrderType type, double price = -1.0) = 0;
};

}

