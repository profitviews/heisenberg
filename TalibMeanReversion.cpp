#include "TalibMeanReversion.h"
#include "util.h"
#include "Poco/ErrorHandler.h"
#include "Poco/Logger.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Net/NetException.h"
#include <boost/json.hpp>
#include "src/include/SIOClient.h"
#include "Poco/Any.h"
#include <ta_libc.h>
#include <numeric>
#include <string>
#include <vector>
#include <memory>
#include <ctime>

using Poco::ErrorHandler;
using Poco::Logger;
using Poco::Net::NetException;
using Poco::JSON::Parser;
using Poco::Dynamic::Var;

TalibMeanReversion::TalibMeanReversion(
	Exchange& exchange, 
	int lookback,
	double reversion_level,
	int base_quantity)
: lookback_        {lookback       }
, reversion_level_ {reversion_level}
, base_quantity_   {base_quantity  }
, exchange_        {exchange       }
{
}

TalibMeanReversion::~TalibMeanReversion() {}

template<typename Sequence>
double TalibMeanReversion::stdev(const Sequence& sequence) const 
{
    std::vector<double> prices{ sequence.begin(), sequence.end()};

    // See: https://www.ta-lib.org/d_api/d_api.html#Output%20Size
    std::unique_ptr <TA_Real []> out{ new TA_Real[lookback_ - TA_STDDEV_Lookback(TA_INTEGER_DEFAULT, TA_REAL_DEFAULT)]};

    TA_Integer outBeg;
    TA_Integer outNbElement;
    auto code{ TA_STDDEV(0, lookback_ - 1, prices.data(), TA_INTEGER_DEFAULT, TA_REAL_DEFAULT, &outBeg, &outNbElement, &out[0])};
    return out[0];
}

void TalibMeanReversion::onTrade(const void *p, Array::Ptr &market_data)
{
	auto& logger{ Logger::get("example")};
	auto result{ market_data->getElement<std::string>(0)};

	Parser parser;
	Var result_json{ parser.parse(result)};

	using Poco::JSON::Object;

	auto result_object{ result_json.extract<Object::Ptr>()};
	auto price{ result_object->get("price").convert<double>()};

    auto symbol{ result_object->get("sym").toString()};

    util::log_trade(logger, result_object);

	time_t date_time{ result_object->get("time").convert<time_t>()};
	logger.information("Time: " + std::string{std::asctime(std::localtime(&date_time))});

    auto& [elements, prices] { counted_prices_[symbol]};

    prices.emplace_back(price);

    if(elements + 1 < lookback_) {
        ++elements; // Accumulate up to lookback_ prices
    } else {
        // These could be done on the fly but the complexity would distract
        auto mean { std::accumulate(prices.begin(), prices.end(), 0.0)/lookback_};
        double std_reversion { reversion_level_*stdev(prices)};

        prices.pop_front(); // Now we have lookback_ prices already, remove the oldest

		logger.information("Mean: " + std::to_string(mean));
		logger.information("Standard reversion: " + std::to_string(std_reversion));

        if(boost::json::value* headers{ result_.if_contains("headers")}; 
           result_.empty() || 
           (headers && 
            std::stol(headers->as_object()["x-ratelimit-reset"].as_string().c_str()) < std::time(nullptr))) {
            if(price > mean + std_reversion) { // Well greater than the normal volatility
                // so sell, expecting a reversion to the mean
                result_ = exchange_.new_order(symbol, Side::sell, base_quantity_, OrderType::market);
            }
            else if(price < mean - std_reversion) { // Well lest than the normal volatility
                // so buy, expecting a reversion to the mean
                result_ = exchange_.new_order(symbol, Side::buy, base_quantity_, OrderType::market);
            }
        }
    }
}
