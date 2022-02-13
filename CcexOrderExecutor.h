#pragma once

#include "OrderExecutor.h"

#include <ccapi_cpp/ccapi_macro.h>

#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/json.hpp>

#include <map>
#include <vector>
#include <tuple>

namespace beast     = boost::beast;         // from <boost/beast.hpp>
namespace http      = beast::http;          // from <boost/beast/http.hpp>
namespace net       = boost::asio;          // from <boost/asio.hpp>
namespace ssl       = boost::asio::ssl;     // from <boost/asio/ssl.hpp>

using     tcp       = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

class CcexOrderExecutor : public OrderExecutor 
{
private:
    static const std::map<OrderType, std::string> order_type_names_;
    static const std::map<Side, std::string> side_names_;

    std::string order_message_;
    
    std::string api_key_;
    std::string api_secret_;
    std::string pass_phrase_;
    int expiry_;
    std::string exchange_;
    static const std::map<std::string, std::tuple<std::string, std::string, std::string>> exchange_key_names_;
public:
    CcexOrderExecutor(
        const std::string&, int, 
        const std::string& api_key, 
        const std::string& api_secret, 
        const std::string& pass_phrase = "");
    ~CcexOrderExecutor();
    void new_order(const std::string& symbol, Side side, double orderQty, OrderType type, double price = -1.0) override;
    boost::json::object result() const;
};
