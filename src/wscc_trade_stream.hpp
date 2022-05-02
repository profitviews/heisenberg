#pragma once

#include "cc_trade_handler.hpp"
#include "trade_stream_maker.hpp"

#include <utils.hpp>

#include <fmt/core.h>

#include <string>
#include <vector>

namespace profitview
{

class WSCcTradeStream
    : public TradeStream
    , private ccapi::CcTradeHandler
{
public:
    WSCcTradeStream(std::string const& trade_stream_name)
        : CcTradeHandler(trade_stream_name)
    {}

    void onStreamedTrade(TradeData const& trade_data) override
    {
        trade_data.print();
    }

    void subscribe(std::string const& market, std::vector<std::string> const& symbol_list) override
    {
        CcTradeHandler::set_stream(this);
        CcTradeHandler::subscribe(market, symbol_list);
    }
};

}    // namespace profitview