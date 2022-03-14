#pragma once

#include "trade_stream_maker.hpp"

#include <ccapi_cpp/ccapi_session.h>

#include <boost/range/adaptor/indexed.hpp>

#include <map>

namespace ccapi 
{
class CcTradeHandler : public EventHandler
{
public:
    CcTradeHandler(const std::string& trade_stream_name) 
    : trade_stream_name_ {trade_stream_name}
    , sessionOptions_    {                 }
    , sessionConfigs_    {                 }
    , session_           { nullptr         }
    {
        session_ = std::make_unique<Session>(sessionOptions_, sessionConfigs_, this);
    }

    void subscribe(const std::string& market, const std::vector<std::string>& symbol_list)
    {   
        market_ = market;  // Assuming for the moment that `subscribe` is called only once and there's only 1 market
        std::vector<Subscription> subs;
        for (auto& symbol : symbol_list) 
            subs.emplace_back(market, symbol, "TRADE", "", symbol);
        session_->subscribe(subs);
    }

    bool processEvent(const Event& event, Session* session) override
    {
        if (event.getType() == Event::Type::SUBSCRIPTION_DATA) {
            for (const auto& message : event.getMessageList()) {
                const auto& cid{message.getCorrelationIdList()}; // Assumes correlation list matchs elements - which it should
                for (const auto& [index, element] : message.getElementList() | boost::adaptors::indexed(0)) {
                    const auto& e{element.getNameValueMap()};
                    profitview::TradeStreamMaker::get(trade_stream_name_).onStreamedTrade(
                        { std::stod(e.at("LAST_PRICE"))
                        , e.at("IS_BUYER_MAKER") == "1" ? profitview::Side::Buy : profitview::Side::Sell
                        , std::stod(e.at("LAST_SIZE"))
                        , market_
                        , cid[index]
                        , message.getTime().time_since_epoch().count()/1'000'000'000}
                    );
                }
            }
        }
        return true;
    }

    ~CcTradeHandler()
    {
        session_->stop();
    }

private:
    const std::string trade_stream_name_; // @note Can't be a reference (why? not understood)
    SessionOptions sessionOptions_;
    SessionConfigs sessionConfigs_;
    std::unique_ptr<Session> session_;

    std::string market_;
};
}
