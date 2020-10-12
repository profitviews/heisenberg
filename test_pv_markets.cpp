#include "socket_io_cpp_streamerConfig.h"
#include "Poco/ConsoleChannel.h"
#include "Poco/Thread.h"
#include "src/include/SIOClient.h"
#include "DataStreamer.h"
#include "SimpleMeanReversionAlgo.h"
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

int main(int argc, char *argv[])
{
	if (argc < 3) {
		// report version
		std::cout 
			<< argv[0] 
			<< " Version " 
			<< socket_io_cpp_streamer_VERSION_MAJOR << "."
			<< socket_io_cpp_streamer_VERSION_MINOR << std::endl;
		std::cout 
			<< "Usage: " << argv[0] << " api_key data_type:market:symbol [data_type:market:symbol ...]" << std::endl;
		return 1;
	}

	Logger *logger = &(Logger::get("example"));

	logger->setChannel(new Poco::ConsoleChannel());

	// Declaring an adapter for the client
	// DataStreamer *dataStreamer = new DataStreamer();
	SimpleMeanReversionAlgo *algo = new SimpleMeanReversionAlgo("bak", "bs");

	logger->information("Creating URI\n");

	//Establish the socket.io connection
	URI connect_uri{"https://markets.profitview.net"};

	URI::QueryParameters qp {{"api_key", argv[1]}};

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

		// wait for user input to move to next section of code
		// socket receiving occurs in another thread and will not be halted
		logger->information("Press ENTER to continue...");
		std::cin.get();

		std::string names{"[\""};
		names += argv[2] + std::string{"\""};
		for (int n = 3; n < argc; ++n)
			names += ",\"" + std::string{argv[n]} + "\"";
		names += "]";

		logger->information("Names to subscribe to: " + names);

		Parser parser;
		Var xbt {parser.parse(names)};

		using Poco::JSON::Array;
		auto xbtArray{xbt.extract<Array::Ptr>()};

		sioUserClient->emit("subscribe", xbtArray);

		logger->information("Press ENTER to quit...");
		std::cin.get();

		// disconnecting the default socket with no endpoint will also disconnect all endpoints
		sioUserClient->disconnect();

	} else {
		logger->error("Client null!");
	}

	return 0;
}