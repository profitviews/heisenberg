#include "Ccex.h"

#include <boost/json.hpp>
#include "CcexOrderExecutor.h"

boost::json::object
Ccex::new_order(const std::string& symbol, Side side, int orderQty, OrderType type) {
    CcexOrderExecutor executor{exchange_, 5, api_key_, api_secret_};
    executor.new_order(symbol, side, orderQty, type);
    return executor.result();
}
