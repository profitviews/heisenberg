// #include "Ccex.h"
#include "CcexOrderExecutor.h"

#include <ccapi_cpp/ccapi_macro.h>

#include <boost/log/trivial.hpp>
#include <boost/json.hpp>

#include <iostream>
#include <string>
#include <map>

auto main(int argc, char* argv[]) -> int
{
    if(argc < 3) 
    {
        std::cout 
            << "Usage: " << argv[0] 
            << " API_key API_secret [Passphrase]"
            << std::endl;
        return 1;
    }

    const std::map<std::string, std::string> exchange_names = {
        {"ftx", CCAPI_EXCHANGE_NAME_FTX},
        {"coinbase", CCAPI_EXCHANGE_NAME_COINBASE},
        {"bitmex", CCAPI_EXCHANGE_NAME_BITMEX},
    };

    BOOST_LOG_TRIVIAL(info) << "Running Ccex test.";

    enum {MAIN_COMMAND_NAME, EXCHANGE, SYM, SIDE, SIZE, TYPE, PRICE, KEY, SECRET, PHRASE};

    CcexOrderExecutor executor{exchange_names.at(argv[EXCHANGE]), 0, argv[KEY], argv[SECRET], argc > 9 ? argv[PHRASE] : ""};

    using Side = OrderExecutor::Side;
    using OrderType = OrderExecutor::OrderType;
    BOOST_LOG_TRIVIAL(info) << argv[SYM] << "Running: " << std::endl; 
    executor.new_order(
        argv[SYM], 
        std::string(argv[SIDE]) == "buy" ? Side::buy : Side::sell, 
        std::stod(argv[SIZE]), 
        std::string(argv[TYPE]) == "limit" ? OrderType::limit : OrderType::market, 
        std::stod(argv[PRICE]));

    return 0;
}