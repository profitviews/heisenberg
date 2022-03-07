#include "ccex_order_executor.hpp"
#include "program_options.hpp"

#include <ccapi_cpp/ccapi_macro.h>

#include <boost/log/trivial.hpp>
#include <boost/json.hpp>
#include <boost/program_options.hpp>

#include <iostream>
#include <string>
#include <map>

namespace profitview
{

struct ProgramArgs
{
	std::string exchange;
    std::string symbol;
	std::string apiKey;
	std::string apiSecret;
	std::string apiPhrase;
    Side side = Side::Buy;
	double size = 0.0;
    OrderType type = OrderType::Limit;
	double price = 0.0;

	void addOptions(boost::program_options::options_description& options)
	{
        namespace po = boost::program_options;
		options.add_options()
			("exchange", po::value(&exchange)->required(), "Crypto Exchange to execute on.")
			("symbol", po::value(&symbol)->required(), "Symbol for cypto assets to trade.")
			("api_key", po::value(&apiKey)->required(), "API key for Cypto exchange.")
			("api_secret", po::value(&apiSecret)->required(), "API secret for Cypto exchange.")
			("api_phrase", po::value(&apiPhrase), "API phrase for Cypto exchange.")
//			("side", po::value(&side)->required(), "The side of the trade <buy|sell>.")
			("size", po::value(&size)->required(), "Size to trade.")
//			("type", po::value(&type)->required(), "The type of order <limit|market>.")
    		("price", po::value(&price)->required(), "Price to trade at.")
		;		
	}
};

}
auto main(int argc, char* argv[]) -> int
{
	using namespace profitview;
	ProgramArgs options;
	auto const result = parseProgramOptions(argc, argv, options);
	if (result)
		return result.value();

    const std::map<std::string, std::string> exchange_names = {
        {"ftx", CCAPI_EXCHANGE_NAME_FTX},
        {"coinbase", CCAPI_EXCHANGE_NAME_COINBASE},
        {"bitmex", CCAPI_EXCHANGE_NAME_BITMEX},
    };

    BOOST_LOG_TRIVIAL(info) << "Running Ccex test.";

    enum {name_arg, market_arg, symbol_arg, side_arg, size_arg, type_arg, price_arg, key_arg, secret_arg, phrase_arg};

    CcexOrderExecutor executor{
        exchange_names.at(options.exchange), 
        options.apiKey, 
        options.apiSecret, 
        options.apiPhrase, 
        0
    };

    BOOST_LOG_TRIVIAL(info) << argv[symbol_arg] << "Running: " << std::endl; 
    executor.new_order(
        options.symbol, 
        std::string(argv[side_arg]) == "buy" ? Side::Buy : Side::Sell, 
        options.size, 
        std::string(argv[type_arg]) == "limit" ? OrderType::Limit : OrderType::Market, 
        options.price);

    return 0;
}