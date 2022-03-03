#include "cpp_crypto_algosConfig.h"
#include "CcSimpleMR.h"
#include "TradeStreamMaker.h"
#include "profitview_util.h"
#include <CcexOrderExecutor.h>

#include <iostream>

int main(int argc, char *argv[])
{
	enum{ name_arg
		, exchange_arg
		, api_key_arg
		, api_secret_arg
		, lookback_arg
		, reversion_level_arg
		, base_quantity_arg
		, symbol_args 
		};

	if (argc < symbol_args) {
		// report version
		std::cout 
			<< argv[0]  
			<< " Version " 
			<< cpp_crypto_algos_VERSION_MAJOR << "."
			<< cpp_crypto_algos_VERSION_MINOR << std::endl;
		std::cout 
			<< "Usage: " << argv[name_arg] << " exchange_key exchange_secret lookback reversion_multiple base_quantity data_type:market:symbol [data_type:market:symbol ...]" << std::endl;
		return 1;
	}

	CcexOrderExecutor executor{argv[exchange_arg], 5, argv[api_key_arg], argv[api_secret_arg]};

	std::string symbols{argv[symbol_args]};
	auto first_colon = symbols.find(':');
	auto second_colon = symbols.find(':', first_colon + 1);
	auto market{symbols.substr(first_colon + 1, second_colon - first_colon - 1)};
	auto symbol{symbols.substr(second_colon + 1)};

	std::vector<std::string> symbol_vector{symbol};
	TradeStreamMaker::register_stream<CcSimpleMR>("CcSimpleMR", 
		&executor, 
		std::stoi(argv[lookback_arg]),
		std::stod(argv[reversion_level_arg]), 
		std::stod(argv[base_quantity_arg]));

	TradeStreamMaker::get("CcSimpleMR").subscribe(market, symbol_vector);

	std::cout << "Press enter to quit" << std::endl;
	std::cin.get();

	return 0;
}