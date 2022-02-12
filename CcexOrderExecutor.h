#pragma once

#include "OrderExecutor.h"
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/json.hpp>

#include <map>
#include <vector>

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
    const std::string address{"www.whatever.com"};
    const std::string SSL_port{"443"};

    std::string order_message_;
    
    // Timing
    struct timespec start_, end_;
    
    std::string api_key_;
    std::string api_secret_;
    int expiry_;
    std::string exchange_;
public:
    CcexOrderExecutor(const std::string&, int, const std::string& api_key, const std::string& api_secret);
    ~CcexOrderExecutor();
    void new_order(const std::string& symbol, Side side, int orderQty, OrderType type) override;
    boost::json::object result() const;
};
