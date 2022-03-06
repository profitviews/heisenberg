#pragma once

#include "order_executor.hpp"

#include <ccapi_cpp/ccapi_logger.h>
#include <ccapi_cpp/ccapi_macro.h>
#include <ccapi_cpp/ccapi_session.h>

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

    public:
        CcexOrderHandler() = default;
        bool processEvent(const ccapi::Event &event, ccapi::Session *session) override
        {
            std::cout << "Received an event:\n" + event.toStringPretty(2, 2) << std::endl;
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

    void new_order(std::string const& symbol, Side side, double orderQty, OrderType type, double price = -1.0) override
    {
        SessionOptions session_options;
        SessionConfigs session_configs;
        CcexOrderHandler event_handler;

        enum { api_key, api_secret, pass_phrase };
        session_configs.setCredential({
            {std::get<api_key    >(exchange_key_names_.at(exchange_)), api_key_    },
            {std::get<api_secret >(exchange_key_names_.at(exchange_)), api_secret_ },
            {std::get<pass_phrase>(exchange_key_names_.at(exchange_)), pass_phrase_},
        });

        Session session(session_options, session_configs, &event_handler);

        Request request(Request::Operation::CREATE_ORDER, exchange_, symbol);
        request.appendParam({{"type", type == OrderType::Market ? "market" : "limit"},
                            {"side", side == Side::Buy ? "BUY" : "SELL"},
                            {"size", std::to_string(orderQty)},
                            {"price", type == OrderType::Limit ? std::to_string(price) : "0.0001"}});
        session.sendRequest(request);
        event_handler.wait();
        session.stop();
    }
};

}