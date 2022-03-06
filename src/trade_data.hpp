#pragma once

#include "side.hpp"

namespace profitview 
{

struct TradeData
{
    double price;
    Side side;
    double size;
    std::string source;
    std::string symbol;
    time_t time;
};

}