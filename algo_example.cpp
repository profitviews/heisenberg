#include "cpp_crypto_algosConfig.h"
#include <Bitmex.h>
#include "Poco/ConsoleChannel.h"
#include "Poco/Thread.h"
#include "SIOClient.h"
#include "DataStreamer.h"
#include "TalibMeanReversion.h"
#include "Poco/URI.h"
#include "Poco/JSON/Array.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Any.h"

#include <iostream>

using Poco::Thread;
using Poco::URI;
using Poco::Dynamic::Var;
using Poco::JSON::Parser;

auto constructSymbolJSON(int number_of_symbols, char** symbols)
{
	std::string symbols_json_string{"[\""};
	symbols_json_string += symbols[0] + std::string{"\""};
	for (int n = 0; n < number_of_symbols; ++n)
		symbols_json_string += ",\"" + std::string{symbols[n]} + "\"";
	symbols_json_string += "]";

	return Parser().parse(symbols_json_string).extract<Poco::JSON::Array::Ptr>();
}

int main(int argc, char *argv[])
{
	const auto 
		name_arg{0}, 
		api_key_arg{1}, api_secret_arg{2}, 
		profitview_api_arg{3}, 
		lookback_arg{4}, reversion_level_arg{5}, base_quantity_arg{6}, 
		symbol_args{8};

	if (argc < symbol_args) {
		// report version
		std::cout 
			<< argv[0] 
			<< " Version " 
			<< cpp_crypto_algos_VERSION_MAJOR << "."
			<< cpp_crypto_algos_VERSION_MINOR << std::endl;
		std::cout 
			<< "Usage: " << argv[name_arg] << "exchange_key exchange_secret api_key data_type:market:symbol [data_type:market:symbol ...]" << std::endl;
		return 1;
	}

	Logger *logger = &(Logger::get("example"));

	logger->setChannel(new Poco::ConsoleChannel());

	Bitmex bitmex{argv[api_key_arg], argv[api_secret_arg]};
	TalibMeanReversion *algo = new TalibMeanReversion(
		bitmex, std::stoi(argv[lookback_arg]), std::stod(argv[reversion_level_arg]), std::stoi(argv[base_quantity_arg]));

	logger->information("Creating URI\n");

	//Establish the socket.io connection
	URI connect_uri{"https://markets.profitview.net"};

	URI::QueryParameters qp {{"api_key", argv[profitview_api_arg]}};

	connect_uri.setQueryParameters(qp);

	logger->information("Connecting to URI and authenticating with API key\n");

	// Establish the socket.io connection to an endpoint
	if(SIOClient *sioUserClient = SIOClient::connect(connect_uri.toString(), qp);
		sioUserClient != nullptr) 
	{
		logger->information("Connected to " + connect_uri.toString() + "\n");
		logger->information("SID: " + sioUserClient->getSid() + "\n");

		logger->information("Adding callback for 'trade'\n");
		sioUserClient->on("trade", algo, callback(&MarketDataAdapter::onTrade));

		logger->information("Socket.io client setup complete\n");

		sioUserClient->emit("subscribe", constructSymbolJSON(argc - symbol_args, argv + symbol_args));

		logger->information("Press ENTER to quit...");
		std::cin.get();

		// disconnecting the default socket with no endpoint will also disconnect all endpoints
		sioUserClient->disconnect();

	} else {
		logger->error("Client null!");
	}

	return 0;
}