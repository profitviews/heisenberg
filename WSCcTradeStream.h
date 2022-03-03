#include "TradeStreamMaker.h"

#include <ccapi_cpp/ccapi_session.h>

#include <iterator>
#include <memory>

namespace ccapi 
{
class CcTradeHandler : public EventHandler
{
public:
    CcTradeHandler(const std::string& trade_stream_name) 
    : trade_stream_name_ {trade_stream_name}
    , sessionOptions_ {         }
    , sessionConfigs_ {         }
    , session_        { nullptr }
    {
        session_ = std::make_unique<Session>(sessionOptions_, sessionConfigs_, this);
    }

    void subscribe(const std::string& market, const std::vector<std::string>& symbol_list)
    {   
        market_ = market;  // Assuming for the moment that `subscribe` is called only once and there's only 1 market
        std::vector<Subscription> subs;
        for (auto& symbol : symbol_list)
        {
            auto s{std::make_shared<Subscription>(market, symbol, "TRADE", "", symbol)};
            subscriptions_[{market, symbol}] = s;
            subs.emplace_back(*s);
        }
        session_->subscribe(subs);

    }

    bool processEvent(const Event& event, Session* session) override
    {
        if (event.getType() == Event::Type::SUBSCRIPTION_DATA) {
            for (const auto& message : event.getMessageList()) {
                auto cid{message.getCorrelationIdList().begin()}; // Assumes correlation list matchs elements - which it should
                for (const auto& element : message.getElementList()) {
                    const auto& e{element.getNameValueMap()};
                    TradeStreamMaker::make.at(trade_stream_name_)->onStreamedTrade(
                        { std::stod(e.at("LAST_PRICE"))
                        , e.at("IS_BUYER_MAKER") == "1" ? TradeData::Side::Buy : TradeData::Side::Sell
                        , std::stod(e.at("LAST_SIZE"))
                        , market_
                        , *cid
                        , message.getTime().time_since_epoch().count()/1'000'000'000}
                    );
                    ++cid;
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
    struct SubscriptionData 
    {
        std::string market, symbol;
        friend auto operator<=>(const SubscriptionData&, const SubscriptionData&) = default;
    };
    std::map<SubscriptionData, std::shared_ptr<Subscription>> subscriptions_;
};
}

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