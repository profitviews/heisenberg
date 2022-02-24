#pragma once
#include <MarketDataAdapter.h>
#include <OrderExecutor.h>
#include <profitview_util.h>
#include <SIOClient.h>
#include <Poco/Logger.h>
#include <Poco/JSON/Parser.h>
#include <boost/json.hpp>
#include <Poco/Any.h>
#include <boost/json.hpp>
#include <functional>
#include <deque>
#include <vector>
#include <map>
#include <numeric>
#include <string>
#include <vector>
#include <memory>
#include <ctime>

class SimpleMR : public MarketDataAdapter
{
public:
	SimpleMR() = delete;
	SimpleMR(OrderExecutor& executor, int lookback, double reversion_level, double base_quantity)
    : lookback_ {lookback}, reversion_level_{reversion_level}, base_quantity_{base_quantity}, executor_{executor}{}
	~SimpleMR(){}

	void onTrade(const void *p, Array::Ptr &market_data)
    {
        auto& logger{ Poco::Logger::get("example")};
        auto result{ market_data->getElement<std::string>(0)};

        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result_json{ parser.parse(result)};

        auto result_object{ result_json.extract<Poco::JSON::Object::Ptr>()};
        auto price{ result_object->get("price").convert<double>()};

        auto symbol{ result_object->get("sym").toString()};

        profitview::util::log_trade(logger, result_object);

        time_t date_time{ result_object->get("time").convert<time_t>()};
        logger.information("Time: " + std::string{std::asctime(std::localtime(&date_time))});

        auto& [elements, prices] { counted_prices_[symbol]};

        prices.emplace_back(price);

        using Side = OrderExecutor::Side;
        using OrderType = OrderExecutor::OrderType;

        if(elements + 1 < lookback_) {
            ++elements; // Accumulate up to lookback_ prices
        } else {
            // These could be done on the fly but the complexity would distract
            auto mean_value { mean(prices)};
            double std_reversion { reversion_level_*stdev(prices, mean_value)};

            prices.pop_front(); // Now we have lookback_ prices already, remove the oldest

            logger.information("Mean: " + std::to_string(mean_value));
            logger.information("Standard reversion: " + std::to_string(std_reversion));

            if(price > mean_value + std_reversion) { // Well greater than the normal volatility
                // so sell, expecting a reversion to the mean
                executor_.new_order(symbol, Side::sell, base_quantity_, OrderType::limit, price);
            }
            else if(price < mean_value - std_reversion) { // Well less than the normal volatility
                // so buy, expecting a reversion to the mean
                executor_.new_order(symbol, Side::buy, base_quantity_, OrderType::limit, price);
            }
        }
    }
private:

	const int lookback_;

    const double reversion_level_; // Multiple of stdev
    double base_quantity_;

    std::map<std::string, std::pair<int, std::deque<double>>> counted_prices_;

    OrderExecutor& executor_;

    boost::json::object result_;

    template<typename Sequence>
    double mean(const Sequence& sequence) const
    {
        return std::accumulate(sequence.begin(), sequence.end(), 0.0)/lookback_;
    }

    template<typename Sequence>
    double stdev(const Sequence& sequence, double m) const
    {
        auto variance_func {[&m, this](auto accumulator, const auto& val) {
            return accumulator + ((val - m)*(val - m) / (lookback_ - 1));
        }};

        return std::sqrt(std::accumulate(sequence.begin(), sequence.end(), 0.0, variance_func));
    }
};