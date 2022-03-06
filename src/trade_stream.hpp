#pragma once

#include "trade_data.hpp"

#include <string>
#include <vector>

namespace profitview 
{

class TradeStream 
{
public:
    TradeStream() = default;
    virtual ~TradeStream() = default;
	virtual void onStreamedTrade(TradeData const& trade_data) = 0;
    virtual void subscribe(std::string const& market, std::vector<std::string> const& symbol_list) = 0;
};

}