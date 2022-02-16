#include "cpp_crypto_algosConfig.h"
#include "TalibMeanReversion.h"
#include <profitview_util.h>
#include <Ccex.h>
#include <SIOClient.h>

#include <iostream>

int main(int argc, char *argv[])
{
	enum{ name_arg
		, api_key_arg
		, api_secret_arg
		, profitview_api_arg
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
			<< "Usage: " << argv[name_arg] << " exchange_key exchange_secret api_key lookback reversion_multiple base_quantity data_type:market:symbol [data_type:market:symbol ...]" << std::endl;
		return 1;
	}

	Logger *logger {&(Logger::get("example"))};

	Ccex ftx{"ftx", argv[api_key_arg], argv[api_secret_arg]};
	TalibMeanReversion *algo { new TalibMeanReversion(
		ftx, std::stoi(argv[lookback_arg]), std::stod(argv[reversion_level_arg]), std::stoi(argv[base_quantity_arg]))};

	logger->information("Creating URI\n");

	using Poco::URI;
	//Establish the socket.io connection
	URI connect_uri{"https://markets.profitview.net"};

	URI::QueryParameters qp {{"api_key", argv[profitview_api_arg]}};

	connect_uri.setQueryParameters(qp);

	logger->information("Connecting to URI and authenticating with API key\n");

	// Establish the socket.io connection to an endpoint
	if(SIOClient *sioUserClient { SIOClient::connect(connect_uri.toString(), qp)}; sioUserClient != nullptr) 
	{
		logger->information("Connected to " + connect_uri.toString() + "\n");
		logger->information("SID: " + sioUserClient->getSid() + "\n");

		logger->information("Adding callback for 'trade'\n");
		sioUserClient->on("trade", algo, callback(&MarketDataAdapter::onTrade));

		logger->information("Socket.io client setup complete\n");

		sioUserClient->emit("subscribe", profitview::util::constructSymbolJSON(argc - symbol_args, argv + symbol_args));

		logger->information("Press ENTER to quit...");
		std::cin.get();

		// disconnecting the default socket with no endpoint will also disconnect all endpoints
		sioUserClient->disconnect();

	} else {
		logger->error("Client null!");
	}

	return 0;
}