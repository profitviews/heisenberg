#include "TradeStreamMaker.h"

#include "CcTradeHandler.h"

#include <iostream>
#include <string>
#include <iterator>
#include <memory>

using namespace ccapi;
class WSCcTradeStream : public TradeStream, private CcTradeHandler
{
public:
    WSCcTradeStream(const std::string& trade_stream_name) 
    : CcTradeHandler(trade_stream_name)
    {
    }

    ~WSCcTradeStream()
    {
    }

    void onStreamedTrade(const TradeData& trade_data) override
    {
        std::cout << "Price: " << trade_data.price << std::endl;
        std::cout << "Side: " << (trade_data.side == TradeData::Side::Buy ? "Buy" : "Sell") << std::endl;
        std::cout << "Size: " << trade_data.size << std::endl;
        std::cout << "Source: " << trade_data.source << std::endl;
        std::cout << "Symbol: " << trade_data.symbol << std::endl;
        std::cout << "Time: " << std::string{std::asctime(std::localtime(&trade_data.time))} << std::endl;
    }

    void subscribe(const std::string& market, const std::vector<std::string>& symbol_list)
    {
        CcTradeHandler::subscribe(market, symbol_list);
    }

};