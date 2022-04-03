#pragma once

#include "side.hpp"

#include <ctime>
#include <string>

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

}    // namespace profitview