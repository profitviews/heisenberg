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
	const std::string& bitmex_api_key, const std::string& bitmex_secret, 
	int lookback,
	double reversion_level,
	int base_quantity)
: lookback_        {lookback      }
, symbol_stats_    {              }
, exchange_        {bitmex_api_key, bitmex_secret}
, reversion_level_ {reversion_level}
, base_quantity_   {base_quantity}
{
}

SimpleMeanReversionAlgo::~SimpleMeanReversionAlgo()
{
}

void SimpleMeanReversionAlgo::RunningStats::prepare_state(double price)
{
	count_++;

	mean_ = mean_ + (price - mean_)/count_;
	d_squared_ = d_squared_ + (price - mean_)*(price - mean_);
}

void SimpleMeanReversionAlgo::RunningStats::update_state(double price)
{
	if(!prepared_) {
		lookback_ = count_;
		prepared_ = true;
	}

	++count_;

	mean_ = (lookback_*mean_ + price - mean_)/lookback_;
	d_squared_ = d_squared_ + (price - mean_)*(price - mean_);
}

double SimpleMeanReversionAlgo::RunningStats::variance() const {
	if(!prepared_) throw NotPrepared();
	if (count_ <= 1) return 0.0f;
	return d_squared_/lookback_;
}

void SimpleMeanReversionAlgo::update_state(double price, const std::string& symbol)
{
	if(symbol_stats_[symbol].count() > lookback_)
	    symbol_stats_[symbol].update_state(price);
	else
	    symbol_stats_[symbol].prepare_state(price);
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
	RunningStats& stats {symbol_stats_.at(symbol)};
	if(stats.prepared()) {
		if(price > stats.mean() + stats.stdev()*reversion_level_) {
			exchange_.new_order(symbol, Side::sell, base_quantity_, OrderType::market);
		}
		if(price < stats.mean() - stats.stdev()*reversion_level_) {
			exchange_.new_order(symbol, Side::buy, base_quantity_, OrderType::market);
		}
	}

}
