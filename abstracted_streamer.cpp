#include "WSCcTradeStream.h"
#include "TradeStreamMaker.h"

#include <iostream>
#include <cstdlib>

int main(int argc, char *argv[])
{
	enum { name_arg
		 , market_arg
		 , symbol_args
	};

	if (argc < symbol_args) {
		std::cout 
			<< "Usage: " << argv[name_arg] << " market symbol [symbol...]" << std::endl;
		return 1;
	}

	const std::string market{argv[market_arg]};

	std::vector<std::string> symbol_vector;
	for (int i = symbol_args; i < argc; ++i)
		symbol_vector.emplace_back(argv[i]);	
	
	TradeStreamMaker::register_stream<WSCcTradeStream>("WSCcStream");
	TradeStreamMaker::get("WSCcStream").subscribe(market, symbol_vector);

	std::cout << "Press enter to quit" << std::endl;
	std::cin.get();
}