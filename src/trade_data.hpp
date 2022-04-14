#pragma once

#include <side.hpp>

#include <fmt/core.h>

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
    void print() const
    {
        fmt::print("Price: {}, ", price);
        fmt::print("Side: {}, ", toString(side));
        fmt::print("Size: {}, ", size);
        fmt::print("Source: {}, ", source);
        fmt::print("Symbol: {}, ", symbol);
        fmt::print("Time: {}", std::asctime(std::localtime(&time)));
    }
};

}    // namespace profitview