#include "profitview_socketio_cppConfig.h"
#include "SIOPocoDataStreamer.h"
#include "profitview_util.h"
#include <SIOClient.h>
#include <Poco/ConsoleChannel.h>
#include <Poco/URI.h>

#include <iostream>
#include <cstdlib>

int main(int argc, char *argv[])
{
	const auto 
		name_arg{0},
		profitview_api_arg{1}, 
		symbol_args{2};

	if (argc < symbol_args) {
		// report version
		std::cout 
			<< argv[0] 
			<< " Version " 
			<< profitview_socketio_cpp_VERSION_MAJOR << "."
			<< profitview_socketio_cpp_VERSION_MINOR << std::endl;
		std::cout 
			<< "Usage: " << argv[name_arg] << " api_key_name data_type:market:symbol [data_type:market:symbol ...]" << std::endl;
		return 1;
	}

	Logger *logger{&(Logger::get("example"))};

	logger->setChannel(new Poco::ConsoleChannel());

	SIOPocoDataStreamer *streamer { new SIOPocoDataStreamer()};

	logger->information("Creating URI\n");

	//Establish the socket.io connection
	Poco::URI connect_uri{"https://markets.profitview.net"};
	Poco::URI::QueryParameters qp {{"api_key", argv[profitview_api_arg]}};

	connect_uri.setQueryParameters(qp);

	logger->information("Connecting to URI and authenticating with API key\n");

	// Connect the socket.io to an endpoint
	if(SIOClient *sioUserClient = SIOClient::connect(connect_uri.toString(), qp); sioUserClient != nullptr) 
	{
		logger->information("Connected to " + connect_uri.toString() + "\n");
		logger->information("SID: " + sioUserClient->getSid() + "\n");
		logger->information("Adding callback for 'trade'\n");

		sioUserClient->on("trade", streamer, callback(&SIOPocoDataStreamer::onTrade));

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