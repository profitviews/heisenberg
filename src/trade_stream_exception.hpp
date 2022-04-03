#pragma once

#include <stdexcept>
#include <string>

namespace profitview
{

struct TradeStreamException : public std::runtime_error
{
    TradeStreamException(std::string const& message)
        : std::runtime_error(message)
    {}
};

}    // namespace profitview