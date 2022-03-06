#include "cpp_crypto_algosConfig.h"
#include "CcexOrderExecutor.h"
#include "CcSimpleMR.h"
#include "program_options.hpp"
#include "trade_stream_maker.hpp"
#include "utils.hpp"

#include <iostream>

namespace profitview::algo
{

struct ProgramArgs
{
	std::string exchange;
	std::string apiKey;
	std::string apiSecret;
	std::string apiPhrase;
	int lookback = 0;
	double reversionLevel = 0.0;
	double baseQuantity = 0.0;
	std::vector<std::string> symbols;

	void addOptions(boost::program_options::options_description& options)
	{
        namespace po = boost::program_options;
        options.add_options()
			("exchange", po::value(&exchange)->required(), "Crypto Exchange to execute on.")
			("api_key", po::value(&apiKey)->required(), "API key for Cypto exchange.")
			("api_secret", po::value(&apiSecret)->required(), "API secret for Cypto exchange.")
			("api_phrase", po::value(&apiPhrase), "API phrase for Cypto exchange.")
			("lookback", po::value(&lookback)->required(), "Time period to look back")
			("reversion_level", po::value(&reversionLevel)->required(), "Mean reversion level.")
			("base_quantity", po::value(&baseQuantity)->required(), "Quantity to trade.")
			("symbol", po::value(&symbols)->multitoken()->required(), "Symbols for cypto assets to trade.")
		;		
	}
};

}

int main(int argc, char *argv[])
{
	using namespace profitview;
	algo::ProgramArgs options;
	auto const result = profitview::parseProgramOptions(argc, argv, options);
	if (result)
		return result.value();

	CcexOrderExecutor executor{options.exchange, options.apiKey, options.apiSecret, options.apiPhrase, 5};
	TradeStreamMaker::register_stream<CcSimpleMR>("CcSimpleMR", 
		&executor, 
		options.lookback,
		options.reversionLevel,
		options.baseQuantity);

	TradeStreamMaker::get("CcSimpleMR").subscribe(options.exchange, options.symbols);

	std::cout << "Press enter to quit" << std::endl;
	std::cin.get();
	return 0;
}