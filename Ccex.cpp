#include "Ccex.h"

#include <boost/json.hpp>
#include "CcexOrderExecutor.h"

boost::json::object
Ccex::new_order(const std::string& symbol, Side side, double orderQty, OrderType type, double price) {
    CcexOrderExecutor executor{exchange_, 5, api_key_, api_secret_, pass_phrase_};
    executor.new_order(symbol, side, orderQty, type, price);
    return executor.result();
}
