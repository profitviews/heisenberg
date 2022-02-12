#include "Ccex.h"

#include <boost/log/trivial.hpp>
#include <boost/json.hpp>

#include <iostream>

auto main(int argc, char** argv) -> int
{
    if(argc < 3) 
    {
        std::cout 
            << "Usage: " << argv[0] 
            << " API_key API_secret"
            << std::endl;
        return 1;
    }

    BOOST_LOG_TRIVIAL(info) << "Running Ccex test.";

    Ccex ccex("bitmex", argv[1], argv[2]);

    BOOST_LOG_TRIVIAL(info) << "XBTUSD 1 result: " << ccex.new_order("XBTUSD", Side::sell, 100, OrderType::market) << std::endl;
    BOOST_LOG_TRIVIAL(info) << "ETHUSD result: " << ccex.new_order("ETHUSD", Side::sell,  1, OrderType::market) << std::endl;
    BOOST_LOG_TRIVIAL(info) << "XRPUSD result: " << ccex.new_order("XRPUSD", Side::buy, 10, OrderType::market) << std::endl;

    return 0;
}