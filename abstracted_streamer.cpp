#include "WSCcTradeStream.h"
#include "TradeStreamMaker.h"

#include <iostream>
#include <cstdlib>

int main(int argc, char *argv[])
{
	enum { name_arg
		 , profitview_api_arg
		 , symbol_args
		 ;

	if (argc < symbol_args) {
		std::cout 
			<< "Usage: " << argv[name_arg] << " api_key_name data_type:market:symbol" << std::endl;
		return 1;
	}

	std::string symbols{argv[symbol_args]};
	auto first_colon = symbols.find(':');
	auto second_colon = symbols.find(':', first_colon + 1);
	auto market{symbols.substr(first_colon + 1, second_colon - first_colon - 1)};
	auto symbol{symbols.substr(second_colon + 1)};

	std::vector<std::string> symbol_vector{symbol};
	TradeStreamMaker::register_stream<WSCcTradeStream>("WSCcStream");
	TradeStreamMaker::get("WSCcStream").subscribe(market, symbol_vector);

	std::cout << "Press enter to quit" << std::endl;
	std::cin.get();
}