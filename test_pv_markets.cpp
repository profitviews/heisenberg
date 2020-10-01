// main.cpp : Defines the entry point for the console application.
#include "Poco/ConsoleChannel.h"
#include "Poco/Thread.h"
#include "src/include/SIOClient.h"
#include "MarketDataAdapter.h"
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
	Logger *logger = &(Logger::get("example"));

	logger->setChannel(new Poco::ConsoleChannel());

	// Declaring an adapter for the client
	MarketDataAdapter *marketDataAdapter = new MarketDataAdapter();

	logger->information("Creating URI\n");

	//Establish the socket.io connection
	URI connect_uri{"https://markets.profitview.net"};

	URI::QueryParameters qp {{"api_key", "223834ffcc42d292c3a5fdc156c244ad2b020465"}};

	connect_uri.setQueryParameters(qp);

	logger->information("Connecting to URI and authenticating with API key\n");

	// Establish the socket.io connection to an endpoint
	if(SIOClient *sioUserClient = SIOClient::connect(
		connect_uri.toString(), 
		qp, 
		true,
		-1);
		sioUserClient != nullptr) 
	{
		logger->information("Connected to " + connect_uri.toString() + "\n");
		logger->information("SID: " + sioUserClient->getSid() + "\n");

		// No "connected" callback necessary
		// sioUserClient->on("connected", marketDataAdapter, callback(&MarketDataAdapter::onConnected));

		logger->information("Adding callback for 'trade'\n");
		sioUserClient->on("trade", marketDataAdapter, callback(&MarketDataAdapter::onTrade));

		logger->information("Socket.io client setup complete\n");

		// wait for user input to move to next section of code
		// socket receiving occurs in another thread and will not be halted
		logger->information("Press ENTER to continue...");
		std::cin.get();

		Parser parser;
		Var xbt {parser.parse("[\"trade:bitmex:XBTUSD\"]")};
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