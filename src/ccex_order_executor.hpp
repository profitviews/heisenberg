#pragma once

/// Live-exchange OrderExecutor via ccapi: one long-lived Session, correlation IDs, pending completions.
/// See docs/ccex-order-executor.md.

#include "order_executor.hpp"

#include <ccapi_cpp/ccapi_logger.h>
#include <ccapi_cpp/ccapi_macro.h>
#include <ccapi_cpp/ccapi_session.h>

#include <boost/describe/enum.hpp>
#include <boost/log/trivial.hpp>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <string>
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
    /// Maximum time to wait for the exchange callback after CREATE_ORDER (ccapi delivers asynchronously).
    static constexpr std::chrono::seconds order_response_timeout_{120};

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

    /// Session::sendRequest is thread-safe; processEvent runs on ccapi EventDispatcher thread(s). Pending promises pair
    /// waiter threads with dispatcher completions via correlation id (see upstream ccapi README "Thread safety").
    std::mutex pending_mu_;
    std::unordered_map<std::string, std::shared_ptr<std::promise<void>>> pending_;
    std::atomic<std::uint64_t> correlation_seq_{0};

    SessionOptions session_options_;
    SessionConfigs session_configs_;

    class CcexOrderHandler : public ccapi::EventHandler
    {
        CcexOrderExecutor* executor_;

    public:
        explicit CcexOrderHandler(CcexOrderExecutor* executor)
            : executor_{executor}
        {}

        bool processEvent(const ccapi::Event& event, ccapi::Session* session) override
        {
            return executor_->on_ccapi_order_event(event);
        }
    };

    CcexOrderHandler handler_;
    std::mutex session_start_mu_;
    std::unique_ptr<Session> session_;

    std::string next_correlation_id()
    {
        return "pv-" + std::to_string(correlation_seq_.fetch_add(std::uint64_t{1}, std::memory_order_relaxed));
    }

    void ensure_session()
    {
        std::lock_guard<std::mutex> lock(session_start_mu_);
        if (!session_)
        {
            session_ = std::make_unique<Session>(session_options_, session_configs_, &handler_);
        }
    }

    bool on_ccapi_order_event(const ccapi::Event& event)
    {
        BOOST_LOG_TRIVIAL(info) << "Received an event:\n" + event.toStringPretty(2, 2) << std::endl;
        const auto& messages = event.getMessageList();
        if (messages.empty())
        {
            BOOST_LOG_TRIVIAL(warning) << "Empty message list in order event" << std::endl;
            return true;
        }

        const auto& msg = messages[0];
        const auto& correl_ids = msg.getCorrelationIdList();
        if (correl_ids.empty())
        {
            BOOST_LOG_TRIVIAL(warning) << "No correlation id in order event" << std::endl;
            return true;
        }
        std::string const cid = correl_ids[0];

        const auto& elem_list = msg.getElementList();
        if (elem_list.empty())
        {
            BOOST_LOG_TRIVIAL(warning) << "Empty element list for correlation id " << cid << std::endl;
        }
        else
        {
            const auto& n = elem_list[0].getNameValueMap();
            BOOST_LOG_TRIVIAL(info) << "Status: "
                      << (n.contains("STATUS") ? n.at("STATUS")
                                               : (n.contains("ERROR_MESSAGE") ? n.at("ERROR_MESSAGE") : "No status"))
                      << std::endl;
            if (n.contains("LIMIT_PRICE"))
            {
                add_open_order(
                    cid,
                    n.at("ORDER_ID"),
                    n.at("INSTRUMENT"),
                    n.at("SIDE") == "BUY" ? Side::Buy : Side::Sell,
                    std::stod(n.at("QUANTITY")),
                    std::stod(n.at("LIMIT_PRICE")),
                    msg.getTimeReceived(),
                    n.at("STATUS"));
            }
        }

        std::shared_ptr<std::promise<void>> promise_to_finish;
        {
            std::lock_guard<std::mutex> lock(pending_mu_);
            auto const it = pending_.find(cid);
            if (it != pending_.end())
            {
                promise_to_finish = std::move(it->second);
                pending_.erase(it);
            }
        }
        if (promise_to_finish)
        {
            try
            {
                promise_to_finish->set_value();
            }
            catch (std::future_error const&)
            {
                // promise already satisfied
            }
        }
        else
        {
            BOOST_LOG_TRIVIAL(warning) << "No pending waiter for correlation id " << cid
                                       << " (duplicate or timed-out response)" << std::endl;
        }

        return true;
    }

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
        , handler_(this)
    {
        enum
        {
            ApiKey,
            ApiSecret,
            PassPhrase,
            SubAccount
        };
        session_configs_.setCredential({
            {std::get<ApiKey>(exchange_key_names_.at(exchange_)),     api_key_    },
            {std::get<ApiSecret>(exchange_key_names_.at(exchange_)),  api_secret_ },
            {std::get<PassPhrase>(exchange_key_names_.at(exchange_)), pass_phrase_},
            {std::get<SubAccount>(exchange_key_names_.at(exchange_)), sub_account_}
        });
    }

    CcexOrderExecutor(CcexOrderExecutor const&) = delete;
    CcexOrderExecutor& operator=(CcexOrderExecutor const&) = delete;
    CcexOrderExecutor(CcexOrderExecutor&&) = delete;
    CcexOrderExecutor& operator=(CcexOrderExecutor&&) = delete;

    ~CcexOrderExecutor() override
    {
        std::lock_guard<std::mutex> lock(session_start_mu_);
        if (session_)
        {
            session_->stop();
            session_.reset();
        }
    }

    friend class CcexOrderHandler;

    const auto& get_open_orders() const { return open_orders_; }

    void new_order(
        std::string const& symbol, Side side, double orderQty, OrderType type, double price    // = 0.0
        ) override
    {
        ensure_session();

        std::string const cid = next_correlation_id();
        auto completion = std::make_shared<std::promise<void>>();
        std::future<void> const done = completion->get_future();
        {
            std::lock_guard<std::mutex> lock(pending_mu_);
            pending_[cid] = std::move(completion);
        }

        Request request(Request::Operation::CREATE_ORDER, exchange_, symbol, cid);

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
        session_->sendRequest(request);
        BOOST_LOG_TRIVIAL(info) << "Waiting for order event cid=" << cid << std::endl;
        if (done.wait_for(order_response_timeout_) != std::future_status::ready)
        {
            BOOST_LOG_TRIVIAL(error) << "Timed out waiting for order response after "
                                     << order_response_timeout_.count() << "s (cid=" << cid << ")";
            std::lock_guard<std::mutex> lock(pending_mu_);
            pending_.erase(cid);
        }
    }
};

}    // namespace profitview
