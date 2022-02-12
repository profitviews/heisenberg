#pragma once

#include <boost/json.hpp>
#include "Exchange.h"

class Ccex : public Exchange
{
public:
    Ccex(const std::string& exchange, const std::string& api_key, const std::string& api_secret)
    : exchange_  { exchange   }
    , api_key_   { api_key    }
    , api_secret_{ api_secret }
    {}
    boost::json::object new_order(const std::string& symbol, Side side, int orderQty, OrderType type) override;

private:
    std::string exchange_; 
    std::string api_key_;
    std::string api_secret_;
};