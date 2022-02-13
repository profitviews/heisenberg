#pragma once

#include <boost/json.hpp>
#include "Exchange.h"

class Ccex : public Exchange
{
public:
    Ccex(
        const std::string& exchange, 
        const std::string& api_key, 
        const std::string& api_secret,
        const std::string& pass_phrase = "")
    : exchange_   { exchange   }
    , api_key_    { api_key    }
    , api_secret_ { api_secret }
    , pass_phrase_{ pass_phrase }
    {}
    boost::json::object new_order(const std::string& symbol, Side side, double orderQty, OrderType type, double price = -1.0) override;

private:
    std::string exchange_; 
    std::string api_key_;
    std::string api_secret_;
    std::string pass_phrase_;
};