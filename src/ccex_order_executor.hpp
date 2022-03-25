#pragma once

#include "order_executor.hpp"

#include <ccapi_cpp/ccapi_logger.h>
#include <ccapi_cpp/ccapi_macro.h>
#include <ccapi_cpp/ccapi_session.h>

#include <boost/describe/enum.hpp>

#include <atomic>
#include <map>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace ccapi
{
  Logger *Logger::logger = nullptr; // This line is needed.
}

namespace profitview
{

using ::ccapi::Request;
using ::ccapi::Session;
using ::ccapi::SessionConfigs;
using ::ccapi::SessionOptions;

class CcexOrderExecutor : public OrderExecutor
{
private:
    inline static const std::unordered_map<OrderType, std::string> order_type_names_{
        {OrderType::Limit, "Limit"}, {OrderType::Market, "Market"}};
    inline static const std::unordered_map<Side, std::string> side_names_{
        {Side::Buy, "Buy"}, {Side::Sell, "Sell"}};

    std::string order_message_;
    std::string api_key_;
    std::string api_secret_;
    std::string pass_phrase_;
    std::string exchange_;
    int expiry_;

    std::unordered_map<
        std::string, 
        std::tuple<std::string, std::string, Side, double, double, ccapi::TimePoint, std::string>
    > open_orders_;


    inline static const std::map<std::string, std::tuple<std::string, std::string, std::string>> exchange_key_names_{{
        {CCAPI_EXCHANGE_NAME_FTX, {CCAPI_FTX_API_KEY, CCAPI_FTX_API_SECRET, ""}},
        {CCAPI_EXCHANGE_NAME_BITMEX, {CCAPI_BITMEX_API_KEY, CCAPI_FTX_API_SECRET, ""}},
        {CCAPI_EXCHANGE_NAME_COINBASE, 
            {CCAPI_COINBASE_API_KEY, CCAPI_COINBASE_API_SECRET, CCAPI_COINBASE_API_PASSPHRASE}},
    }};

    class CcexOrderHandler : public ccapi::EventHandler
    {
    private:
        std::atomic_flag ordered_;

        CcexOrderExecutor* executor_;

    public:
        CcexOrderHandler(CcexOrderExecutor* executor)
        : executor_{executor}
        {}

        bool processEvent(const ccapi::Event &event, ccapi::Session *session) override
        {
            std::cout << "Received an event:\n" + event.toStringPretty(2, 2) << std::endl;
            const auto& m{event.getMessageList()};
            const auto& n{m[0].getElementList()[0].getNameValueMap()};
            std::cout << "Status: " << 
                (n.contains("STATUS") 
                ? n.at("STATUS") 
                : (n.contains("ERROR_MESSAGE") 
                    ? n.at("ERROR_MESSAGE") 
                    : "No status")) 
                << std::endl;
            if(n.contains("LIMIT_PRICE"))
            {
                executor_->add_open_order
                    ( m[0].getCorrelationIdList()[0]
                    , n.at("ORDER_ID")
                    , n.at("INSTRUMENT")
                    , n.at("SIDE") == "BUY" ? Side::Buy : Side::Sell
                    , std::stod(n.at("QUANTITY"))
                    , std::stod(n.at("LIMIT_PRICE"))
                    , m[0].getTimeReceived()
                    , n.at("STATUS")
                    );
            }
            ordered_.test_and_set();
            ordered_.notify_one();
            return true;
        }
        void wait() const
        {
            std::cout << "Waiting for order event" << std::endl;
            ordered_.wait(false);
        }
    };

    void add_open_order
        ( const std::string& cid
        , const std::string& order_id
        , const std::string& symbol
        , Side side
        , double size
        , double price
        , ccapi::TimePoint time
        , const std::string& status
        )
    {
        open_orders_[cid] = {order_id, symbol, side, size, price, time, status};
    }

    void adjust_exchange_params(const std::string& exchange, auto& params)
    {
        // Handling of Market orders differs between exchanges
        if(params.at("type") == "market") {
            if(exchange == CCAPI_EXCHANGE_NAME_FTX) {
                params["price"] = (params.at("side") == "BUY" ? 
                    // Force cross for synthetic market price
                    // @note makes assumptions on extremes of price
                    "1000000.0" : "0.0000001");  
            } else if(exchange == CCAPI_EXCHANGE_NAME_COINBASE) {
                params.erase("price");
            }
        }
    }

public:
    CcexOrderExecutor(
        const std::string &exchange,
        const std::string &api_key,
        const std::string &api_secret,
        const std::string &pass_phrase,
        int expiry)
    :   exchange_{exchange}
    ,   api_key_{api_key}
    ,   api_secret_{api_secret}
    ,   pass_phrase_{pass_phrase}
    ,   expiry_{expiry}
    {
    }

    friend class CcexOrderHandler;

    const auto& get_open_orders() const
    {
        return open_orders_;
    }

    void new_order
        ( std::string const& symbol
        , Side side
        , double orderQty
        , OrderType type
        , double price = 0.0
        ) override
    {
        SessionOptions session_options;
        SessionConfigs session_configs;
        CcexOrderHandler event_handler(this);

        enum { ApiKey, ApiSecret, PassPhrase };
        session_configs.setCredential(
            { {std::get<ApiKey    >(exchange_key_names_.at(exchange_)), api_key_    }
            , {std::get<ApiSecret >(exchange_key_names_.at(exchange_)), api_secret_ }
            , {std::get<PassPhrase>(exchange_key_names_.at(exchange_)), pass_phrase_}
            });

        Session session(session_options, session_configs, &event_handler);

        Request request(Request::Operation::CREATE_ORDER, exchange_, symbol);

        // @todo Properly handle Market orders.  Some exchanges don't have native Market orders
        //       Therefore create a type of cross limit to substitute
        std::map<std::string, std::string> params
            { {"type", type == OrderType::Market ? "market" : "limit"}
            , {"side", side == Side::Buy ? "BUY" : "SELL"}
            , {"size", std::to_string(orderQty)}
            , {"price", std::to_string(price)}
            };
        adjust_exchange_params(exchange_, params);
        request.appendParam(params);
        session.sendRequest(request);
        event_handler.wait();
        session.stop();
    }
};

}