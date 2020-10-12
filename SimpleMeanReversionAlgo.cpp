#include "SimpleMeanReversionAlgo.h"
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

SimpleMeanReversionAlgo::SimpleMeanReversionAlgo(
	const std::string& bitmex_api_key, const std::string& bitmex_secret, int lookback)
: lookback_      {lookback      }
, symbol_stats_  {              }
, exchange_      {bitmex_api_key, bitmex_secret}
{
}

SimpleMeanReversionAlgo::~SimpleMeanReversionAlgo()
{
}

void SimpleMeanReversionAlgo::RunningStats::update_state(double price)
{
	count_++;
	const double mean_differential = (price - mean_) / count_;
	const double new_mean = mean_ + mean_differential;
	const double d_squared_increment = 
			(price - new_mean) * (price - mean_);
	const double new_d_squared = d_squared_ + d_squared_increment;
	mean_ = new_mean;
	d_squared_ = new_d_squared;
}

void SimpleMeanReversionAlgo::update_state(double price, const std::string& symbol)
{
    symbol_stats_[symbol].update_state(price);
}

void SimpleMeanReversionAlgo::onTrade(const void *pSender, Array::Ptr &arg)
{
	auto& l{Logger::get("example")};
	auto result{arg->getElement<std::string>(0)};

	Parser parser;
	Var result_json{parser.parse(result)};

	using Poco::JSON::Object;

	auto resultObj{result_json.extract<Object::Ptr>()};
	auto price{resultObj->get("price").convert<double>()};

    auto symbol{resultObj->get("sym").toString()};
    update_state(price, symbol);

	l.information("Price: " + std::to_string(price));
    l.information("Mean: " + std::to_string(symbol_stats_[symbol].mean()));
    l.information("Standard deviation: " + std::to_string(symbol_stats_[symbol].stdev()));

	l.information("Side: " + resultObj->get("side").toString());
	l.information("Size: " + resultObj->get("size").toString());
	l.information("Source: " + resultObj->get("src").toString());
	l.information("Symbol: " + resultObj->get("sym").toString());
	time_t date_time{resultObj->get("time").convert<time_t>()};
	l.information("Time: " + std::string{std::asctime(std::localtime(&date_time))});
}
