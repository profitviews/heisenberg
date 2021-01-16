#include "TalibMeanReversion.h"
#include "Poco/ErrorHandler.h"
#include "Poco/Logger.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Net/NetException.h"
#include "src/include/SIOClient.h"
#include "Poco/Any.h"
#include <ta_libc.h>
#include <numeric>
#include <string>
#include <vector>
#include <memory>

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

TalibMeanReversion::~TalibMeanReversion()
{
}

template<typename Sequence>
double TalibMeanReversion::stdev(const Sequence& sequence) const {
    std::vector<double> prices{ sequence.begin(), sequence.end()};

    // See: https://www.ta-lib.org/d_api/d_api.html#Output%20Size
    std::unique_ptr <TA_Real []> out{ new TA_Real[lookback_ - TA_STDDEV_Lookback(TA_INTEGER_DEFAULT, TA_REAL_DEFAULT)]};

    TA_Integer outBeg;
    TA_Integer outNbElement;
    auto code{ TA_STDDEV(0, lookback_ - 1, prices.data(), TA_INTEGER_DEFAULT, TA_REAL_DEFAULT, &outBeg, &outNbElement, &out[0])};
    return out[0];
}

void TalibMeanReversion::onTrade(const void *pSender, Array::Ptr &arg)
{
	auto& l{ Logger::get("example")};
	auto result{ arg->getElement<std::string>(0)};

	Parser parser;
	Var result_json{ parser.parse(result)};

	using Poco::JSON::Object;

	auto resultObj{ result_json.extract<Object::Ptr>()};
	auto price{ resultObj->get("price").convert<double>()};

    auto symbol{ resultObj->get("sym").toString()};

	l.information("Price: " + std::to_string(price));
	l.information("Side: " + resultObj->get("side").toString());
	l.information("Size: " + resultObj->get("size").toString());
	l.information("Source: " + resultObj->get("src").toString());
	l.information("Symbol: " + resultObj->get("sym").toString());
	time_t date_time{ resultObj->get("time").convert<time_t>()};
	l.information("Time: " + std::string{std::asctime(std::localtime(&date_time))});

    auto& [elements, prices] { counted_prices_[symbol]};
    prices.emplace_back(price);
    if(elements + 1 < lookback_) {
        ++elements;
    } else {
        auto mean { std::accumulate(prices.begin(), prices.end(), 0.0)/lookback_};
        double std_reversion{ reversion_level_*stdev(prices)};
        prices.pop_front();
		l.information("Mean: " + std::to_string(mean));
		l.information("Standard reversion: " + std::to_string(std_reversion));
        if(price > mean + std_reversion) {
			exchange_.new_order(symbol, Side::sell, base_quantity_, OrderType::market);
        }
        else if(price < mean - std_reversion) {
            exchange_.new_order(symbol, Side::buy, base_quantity_, OrderType::market);
        }
    }
}
