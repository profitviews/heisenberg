#pragma once

#include "order_executor.hpp"

#include <ccapi_cpp/ccapi_logger.h>
#include <ccapi_cpp/ccapi_macro.h>
#include <ccapi_cpp/ccapi_session.h>

#include <boost/describe/enum.hpp>
#include <boost/log/trivial.hpp>

#include <atomic>
#include <map>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace ccapi
{
inline Logger* Logger::logger = nullptr;    // Required by ccapi; inline avoids ODR violations when header is included in multiple TUs.
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
        {OrderType::Limit,  "Limit" },
        {OrderType::Market, "Market"}
    };
    inline static const std::unordered_map<Side, std::string> side_names_{
        {Side::Buy,  "Buy" },
        {Side::Sell, "Sell"}
    };

    std::string order_message_;
    std::string api_key_;
    std::string api_secret_;
    std::string pass_phrase_;
    std::string sub_account_;
    std::string exchange_;
    int expiry_;

    std::unordered_map<
        std::string,
        std::tuple<std::string, std::string, Side, double, double, ccapi::TimePoint, std::string>>
        open_orders_;

    inline static const std::map<std::string, std::tuple<std::string, std::string, std::string, std::string>> exchange_key_names_{
        {
         {CCAPI_EXCHANGE_NAME_FTX, {CCAPI_FTX_API_KEY, CCAPI_FTX_API_SECRET, "", CCAPI_FTX_API_SUBACCOUNT}},
         {CCAPI_EXCHANGE_NAME_BITMEX, {CCAPI_BITMEX_API_KEY, CCAPI_BITMEX_API_SECRET, "", ""}},
         {CCAPI_EXCHANGE_NAME_COINBASE,
             {CCAPI_COINBASE_API_KEY, CCAPI_COINBASE_API_SECRET, CCAPI_COINBASE_API_PASSPHRASE, ""}},
         {CCAPI_EXCHANGE_NAME_KRAKEN, {CCAPI_KRAKEN_API_KEY, CCAPI_KRAKEN_API_SECRET, "", ""}},
         }
    };

    class CcexOrderHandler : public ccapi::EventHandler
    {
    private:
        std::atomic_flag ordered_;

        CcexOrderExecutor* executor_;

    public:
        CcexOrderHandler(CcexOrderExecutor* executor)
            : executor_{executor}
        {}

        bool processEvent(const ccapi::Event& event, ccapi::Session* session) override
        {
            BOOST_LOG_TRIVIAL(info) << "Received an event:\n" + event.toStringPretty(2, 2) << std::endl;
            const auto& m{event.getMessageList()};
            const auto& n{m[0].getElementList()[0].getNameValueMap()};
            BOOST_LOG_TRIVIAL(info) << "Status: "
                      << (n.contains("STATUS") ? n.at("STATUS")
                                               : (n.contains("ERROR_MESSAGE") ? n.at("ERROR_MESSAGE") : "No status"))
                      << std::endl;
            if (n.contains("LIMIT_PRICE"))
            {
                executor_->add_open_order(
                    m[0].getCorrelationIdList()[0],
                    n.at("ORDER_ID"),
                    n.at("INSTRUMENT"),
                    n.at("SIDE") == "BUY" ? Side::Buy : Side::Sell,
                    std::stod(n.at("QUANTITY")),
                    std::stod(n.at("LIMIT_PRICE")),
                    m[0].getTimeReceived(),
                    n.at("STATUS"));
            }
            ordered_.test_and_set();
            ordered_.notify_one();
            return true;
        }
        void wait() const
        {
            BOOST_LOG_TRIVIAL(info) << "Waiting for order event" << std::endl;
            ordered_.wait(false);
        }
    };

    void add_open_order(
        const std::string& cid,
        const std::string& order_id,
        const std::string& symbol,
        Side side,
        double size,
        double price,
        ccapi::TimePoint time,
        const std::string& status)
    {
        open_orders_[cid] = {order_id, symbol, side, size, price, time, status};
    }

    void adjust_exchange_params(const std::string& exchange, std::map<std::string, std::string>& params)
    {
        bool const market = [&]() {
            if (exchange == CCAPI_EXCHANGE_NAME_COINBASE)
                return params.count("type") && params.at("type") == "market";
            return params.count(CCAPI_EM_ORDER_TYPE) && params.at(CCAPI_EM_ORDER_TYPE) == "market";
        }();
        if (market)
        {
            if (exchange == CCAPI_EXCHANGE_NAME_COINBASE || exchange == CCAPI_EXCHANGE_NAME_KRAKEN)
                params.erase(CCAPI_EM_ORDER_LIMIT_PRICE);
        }
    }

public:
    CcexOrderExecutor(
        const std::string& exchange,
        const std::string& api_key,
        const std::string& api_secret,
        const std::string& pass_phrase,
        const std::string& sub_account)
        : exchange_{exchange}
        , api_key_{api_key}
        , api_secret_{api_secret}
        , pass_phrase_{pass_phrase}
        , sub_account_{sub_account}
    {}

    friend class CcexOrderHandler;

    const auto& get_open_orders() const { return open_orders_; }

    void new_order(
        std::string const& symbol, Side side, double orderQty, OrderType type, double price    // = 0.0
        ) override
    {
        SessionOptions session_options;
        SessionConfigs session_configs;
        CcexOrderHandler event_handler(this);

        enum
        {
            ApiKey,
            ApiSecret,
            PassPhrase,
            SubAccount
        };
        session_configs.setCredential({
            {std::get<ApiKey>(exchange_key_names_.at(exchange_)),     api_key_    },
            {std::get<ApiSecret>(exchange_key_names_.at(exchange_)),  api_secret_ },
            {std::get<PassPhrase>(exchange_key_names_.at(exchange_)), pass_phrase_},
            {std::get<SubAccount>(exchange_key_names_.at(exchange_)), sub_account_}
        });

        Session session(session_options, session_configs, &event_handler);
        Request request(Request::Operation::CREATE_ORDER, exchange_, symbol);

        std::map<std::string, std::string> params{
            {CCAPI_EM_ORDER_SIDE,        side == Side::Buy ? CCAPI_EM_ORDER_SIDE_BUY : CCAPI_EM_ORDER_SIDE_SELL},
            {CCAPI_EM_ORDER_QUANTITY,    std::to_string(orderQty)                                                         },
            {CCAPI_EM_ORDER_LIMIT_PRICE, std::to_string(price)                                                            },
        };
        std::string const ord_type = type == OrderType::Market ? "market" : "limit";
        if (exchange_ == CCAPI_EXCHANGE_NAME_COINBASE)
            params["type"] = ord_type;
        else
            params[CCAPI_EM_ORDER_TYPE] = ord_type;

        adjust_exchange_params(exchange_, params);

        request.appendParam(params);
        session.sendRequest(request);
        event_handler.wait();
        session.stop();
    }
};

}    // namespace profitview