#include "CcexOrderExecutor.h"

#include <boost/log/trivial.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/strand.hpp>
#include <boost/json.hpp>
#include <openssl/hmac.h>

#include <iostream>
#include <unordered_map>
#include <iomanip>

const std::map<OrderExecutor::OrderType, std::string> CcexOrderExecutor::order_type_names_{
    {OrderType::limit, "Limit"}, {OrderType::market, "Market"}
};

const std::map<OrderExecutor::Side, std::string> CcexOrderExecutor::side_names_{
    {Side::buy, "Buy"}, {Side::sell, "Sell"}
};

CcexOrderExecutor::CcexOrderExecutor(const std::string& exchange, int expiry, const std::string& api_key, const std::string& api_secret)
: exchange_     {exchange   }
, api_key_      {api_key    }
, api_secret_   {api_secret }
, expiry_       {expiry     }
{
    // @todo
}

void CcexOrderExecutor::new_order(const std::string& symbol, Side side, int orderQty, OrderType type)
{
    order_message_ = {
    "{"
        "\"symbol\":\""  + symbol                     + "\","
        "\"ordType\":\"" + order_type_names_.at(type) + "\","
        "\"side\":\""    + side_names_.at(side)       + "\","
        "\"orderQty\":"  + std::to_string(orderQty)   +
    "}"};

}

boost::json::object CcexOrderExecutor::result() const
{
    using namespace boost::json;

    // @todo

    object headers, body;
    parser p;

    return {{"headers", headers }, {"body", p.release() }};
}

CcexOrderExecutor::~CcexOrderExecutor()
{
}


// Private methods
