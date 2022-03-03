#pragma once

#include <OrderExecutor.h>
#include "WSCcTradeStream.h"

#include "profitview_util.h"

#include <iostream>

using namespace ccapi;

class CcSimpleMR : public TradeStream, private CcTradeHandler
{
public:
    CcSimpleMR
		( const std::string trade_stream_name 
        , OrderExecutor* executor
		, int lookback
		, double reversion_level
		, double base_quantity
	) 
    : CcTradeHandler(trade_stream_name)
	, lookback_        {lookback       }
	, reversion_level_ {reversion_level}
	, base_quantity_   {base_quantity  }
	, executor_        {executor       }
    {
    }

    ~CcSimpleMR()
    {
    }

    void onStreamedTrade(const TradeData& trade_data) override
    {
        std::cout << "Price: " << trade_data.price << std::endl;
        std::cout << "Side: " << (trade_data.side == TradeData::Side::Buy ? "Buy" : "Sell") << std::endl;
        std::cout << "Size: " << trade_data.size << std::endl;
        std::cout << "Source: " << trade_data.source << std::endl;
        std::cout << "Symbol: " << trade_data.symbol << std::endl;
        std::cout << "Time: " << std::string{std::asctime(std::localtime(&trade_data.time))} << std::endl;
            auto& [elements, prices] { counted_prices_[trade_data.symbol]};

        prices.emplace_back(trade_data.price);

        using Side = OrderExecutor::Side;
        using OrderType = OrderExecutor::OrderType;
        using namespace profitview::util;

        if(elements + 1 < lookback_) {
            ++elements; // Accumulate up to lookback_ prices
        } else {
            // These could be done on the fly but the complexity would distract
            auto mean_value { mean(prices, lookback_)};
            double std_reversion { reversion_level_*stdev(prices, mean_value, lookback_)};

            prices.pop_front(); // Now we have lookback_ prices already, remove the oldest

            if(trade_data.price > mean_value + std_reversion) { // Well greater than the normal volatility
                // so sell, expecting a reversion to the mean
                executor_->new_order(trade_data.symbol, Side::sell, base_quantity_, OrderType::limit, trade_data.price);
            }
            else if(trade_data.price < mean_value - std_reversion) { // Well less than the normal volatility
                // so buy, expecting a reversion to the mean
                executor_->new_order(trade_data.symbol, Side::buy, base_quantity_, OrderType::limit, trade_data.price);
            }
        }
	}

    void subscribe(const std::string& market, const std::vector<std::string>& symbol_list)
    {
        CcTradeHandler::subscribe(market, symbol_list);
    }
private:

	const int lookback_;

    const double reversion_level_; // Multiple of stdev
    double base_quantity_;

    std::map<std::string, std::pair<int, std::deque<double>>> counted_prices_;

    OrderExecutor* executor_;
};