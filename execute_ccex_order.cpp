#include "Ccex.h"

#include <ccapi_cpp/ccapi_macro.h>

#include <boost/log/trivial.hpp>
#include <boost/json.hpp>

#include <iostream>

auto main(int argc, char** argv) -> int
{
    if(argc < 3) 
    {
        std::cout 
            << "Usage: " << argv[0] 
            << " API_key API_secret [Passphrase]"
            << std::endl;
        return 1;
    }

    BOOST_LOG_TRIVIAL(info) << "Running Ccex test.";

    Ccex ccex(CCAPI_EXCHANGE_NAME_FTX, argv[1], argv[2]
    // , argv[3]
    );

    // BOOST_LOG_TRIVIAL(info) << "XBTUSD 1 result: " << ccex.new_order("MKR-BTC", Side::sell, 0.005, OrderType::limit, 0.048) << std::endl;
    BOOST_LOG_TRIVIAL(info) << "BTC-PERP 1 result: " 
        << ccex.new_order("BTC-PERP", Side::buy, 0.002, OrderType::limit, 40000.0) << std::endl;
    // BOOST_LOG_TRIVIAL(info) << "ETHUSD result: " << ccex.new_order("ETHUSD", Side::sell,  1, OrderType::market) << std::endl;
    // BOOST_LOG_TRIVIAL(info) << "XRPUSD result: " << ccex.new_order("XRPUSD", Side::buy, 10, OrderType::market) << std::endl;

    return 0;
}