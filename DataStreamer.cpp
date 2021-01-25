#include "DataStreamer.h"
#include "profitview_util.h"
#include "Poco/ErrorHandler.h"
#include "Poco/Logger.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Net/NetException.h"
#include "src/include/SIOClient.h"
#include "Poco/Any.h"
#include <string>

using Poco::ErrorHandler;
using Poco::Logger;
using Poco::Net::NetException;
using Poco::JSON::Parser;
using Poco::Dynamic::Var;

DataStreamer::DataStreamer() {}
DataStreamer::~DataStreamer() {}

void DataStreamer::onTrade(const void *, Array::Ptr &arg)
{
	auto& logger{Logger::get("example")};
	auto result{arg->getElement<std::string>(0)};

	Parser parser;
	Var result_json{parser.parse(result)};

	using Poco::JSON::Object;

	auto result_object{result_json.extract<Object::Ptr>()};

    auto price{result_object->get("price").convert<double>()};
    std::string side{result_object->get("side").toString() == "S" ? "Sell" : "Buy"};
    int size{result_object->get("size").convert<int>()};

	profitview::util::log_trade(logger, result_object);

	time_t date_time{result_object->get("time").convert<time_t>()};
	logger.information("Time: " + std::string{std::asctime(std::localtime(&date_time))});
}

