#pragma once

#include "cc_trade_handler.hpp"
#include "trade_stream_maker.hpp"

#include <fmt/core.h>

#include <string>
#include <vector>

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
        fmt::print("Price: {}, ", trade_data.price);
        fmt::print("Side: {}, ", toString(trade_data.side));
        fmt::print("Size: {}, ", trade_data.size);
        fmt::print("Source: {}, ", trade_data.source);
        fmt::print("Symbol: {}, ", trade_data.symbol);
        fmt::print("Time: {}", std::string{std::asctime(std::localtime(&trade_data.time))});
    }

    void subscribe(std::string const& market,  std::vector<std::string> const& symbol_list)
    {
        CcTradeHandler::subscribe(market, symbol_list);
    }

};

}