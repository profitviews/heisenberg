#pragma once

#include "trade_stream_maker.hpp"
#include "trade_stream.hpp"

#include <ccapi_cpp/ccapi_session.h>

#include <boost/range/adaptor/indexed.hpp>

#include <map>

namespace ccapi
{
using namespace profitview;
class CcTradeHandler : public EventHandler
{
public:
    CcTradeHandler(const std::string& trade_stream_name)
        : trade_stream_name_{trade_stream_name}
        , sessionOptions_{}
        , sessionConfigs_{}
        , session_{nullptr}
        , stream_{TradeStreamMaker::get(trade_stream_name_)}
    {
        session_ = std::make_unique<Session>(sessionOptions_, sessionConfigs_, this);
    }

    void subscribe(const std::string& market, const std::vector<std::string>& symbol_list)
    {
        market_ = market;    // Assuming for the moment that `subscribe` is called only
                             // once and there's only 1 market
        std::vector<Subscription> subscriptions;
        for (auto& symbol : symbol_list)
            subscriptions.emplace_back(market, symbol, "TRADE", "",
                                       symbol);    // <-- using `symbol` as correlation id
        session_->subscribe(subscriptions);
    }

    bool processEvent(const Event& event, Session* session) override
    {
        if (event.getType() == Event::Type::SUBSCRIPTION_DATA)
        {
            for (const auto& message : event.getMessageList())
            {
                const auto& cid{message.getCorrelationIdList()};    // Assumes correlation list matchs
                                                                    // elements - which it should
                for (const auto& [index, element] : message.getElementList() | boost::adaptors::indexed(0))
                {
                    const auto& e{element.getNameValueMap()};
                    stream_.onStreamedTrade(
                            {std::stod(e.at("LAST_PRICE")),
                             e.at("IS_BUYER_MAKER") == "1" ? Side::Buy : Side::Sell,
                             std::stod(e.at("LAST_SIZE")),
                             market_,
                             cid[index],
                             message.getTime().time_since_epoch().count() / 1'000'000'000});
                }
            }
        }
        return true;
    }

    ~CcTradeHandler() { session_->stop(); }

private:
    const std::string trade_stream_name_;    // @note Can't be a reference (why? not understood)
    SessionOptions sessionOptions_;
    SessionConfigs sessionConfigs_;
    std::unique_ptr<Session> session_;
    TradeStream& stream_;

    std::string market_;
};
}    // namespace ccapi
