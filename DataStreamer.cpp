#include "DataStreamer.h"
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

DataStreamer::DataStreamer()
{
}

DataStreamer::~DataStreamer()
{
}

void DataStreamer::onTrade(const void *, Array::Ptr &arg)
{
	auto& l{Logger::get("example")};
	auto result{arg->getElement<std::string>(0)};

	Parser parser;
	Var result_json{parser.parse(result)};

	using Poco::JSON::Object;

	auto resultObj{result_json.extract<Object::Ptr>()};

    auto price{resultObj->get("price").convert<double>()};
	l.information("Price: " + std::to_string(price));
    
    std::string side{resultObj->get("side").toString() == "S" ? "Sell" : "Buy"};
    l.information("Side: " + side);

    int size{resultObj->get("size").convert<int>()};
	l.information("Size: " + std::to_string(size));

	l.information("Source: " + resultObj->get("src").toString());
	l.information("Symbol: " + resultObj->get("sym").toString());

	time_t date_time{resultObj->get("time").convert<time_t>()};
	l.information("Time: " + std::string{std::asctime(std::localtime(&date_time))});
}

