#pragma once

#include "trade_stream_maker.hpp"

#include "CcTradeHandler.h"

#include <iostream>
#include <string>
#include <iterator>
#include <memory>

namespace profitview {
 
class WSCcTradeStream : public TradeStream, private ccapi::CcTradeHandler
{
public:
    WSCcTradeStream(std::string const& trade_stream_name) 
    :   CcTradeHandler(trade_stream_name)
    {
    }

    void onStreamedTrade(TradeData const& trade_data) override
    {
        std::cout << "Price: " << trade_data.price << std::endl;
        std::cout << "Side: " << (trade_data.side == Side::Buy ? "Buy" : "Sell") << std::endl;
        std::cout << "Size: " << trade_data.size << std::endl;
        std::cout << "Source: " << trade_data.source << std::endl;
        std::cout << "Symbol: " << trade_data.symbol << std::endl;
        std::cout << "Time: " << std::string{std::asctime(std::localtime(&trade_data.time))} << std::endl;
    }

    void subscribe(std::string const& market,  std::vector<std::string> const& symbol_list)
    {
        CcTradeHandler::subscribe(market, symbol_list);
    }

};

}