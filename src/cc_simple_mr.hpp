#pragma once

#include "enum.hpp"
#include "order_executor.hpp"
#include "utils.hpp"
#include "wscc_trade_stream.hpp"

#include <fmt/core.h>

namespace profitview
{

template<std::floating_point Float = double, std::integral Int = int>
class CcSimpleMR
    : public TradeStream
    , private ccapi::CcTradeHandler
{
public:
    CcSimpleMR(
        const std::string trade_stream_name,
        OrderExecutor* executor,
        Int lookback,
        Float reversion_level,
        Float base_quantity,
        const std::string& csv_name = "SimpleMR.csv")
        : ccapi::CcTradeHandler(trade_stream_name)
        , lookback_{lookback}
        , reversion_level_{reversion_level}
        , base_quantity_{base_quantity}
        , executor_{executor}
        , csv_{csv_name}
        , csv_writer_{csv_}
    {}

    void onStreamedTrade(TradeData const& trade_data) override
    {
        fmt::print("Price: {}, ", trade_data.price);
        fmt::print("Side: {}, ", toString(trade_data.side));
        fmt::print("Size: {}, ", trade_data.size);
        fmt::print("Source: {}, ", trade_data.source);
        fmt::print("Symbol: {}, ", trade_data.symbol);
        fmt::print("Time: {}", std::asctime(std::localtime(&trade_data.time)));

        auto& [elements, prices]{counted_prices_[trade_data.symbol]};

        prices.emplace_back(trade_data.price);

        if (elements + 1 < lookback_)
        {
            ++elements;    // Accumulate up to lookback_ prices
        }
        else
        {
            // These could be done on the fly but the complexity would distract
            auto mean{util::ma(prices)};
            auto std_reversion{reversion_level_ * util::stdev(prices, mean, lookback_)};

            prices.pop_front();    // Now we have lookback_ prices already, remove the
                                   // oldest
            
            bool 
                sell_signal{trade_data.price > mean + std_reversion},
                buy_signal {trade_data.price < mean - std_reversion};
            if (sell_signal)
            {    // Well greater than the normal volatility
                // so sell, expecting a reversion to the mean
                executor_->new_order(trade_data.symbol, Side::Sell, base_quantity_, OrderType::Market);
            }
            else if (buy_signal)
            {    // Well less than the normal volatility
                // so buy, expecting a reversion to the mean
                executor_->new_order(trade_data.symbol, Side::Buy, base_quantity_, OrderType::Market);
            }
            csv_writer_.write_strings(
                trade_data.symbol,
                trade_data.price,
                toString(trade_data.side).data(),
                trade_data.size,
                trade_data.source,
                trade_data.time,
                mean,
                std_reversion,
                buy_signal ? "Buy" : (sell_signal ? "Sell" : "No trade"));
        }
    }

    void subscribe(const std::string& market, const std::vector<std::string>& symbol_list)
    {
        CcTradeHandler::subscribe(market, symbol_list);
    }

private:
    const Int lookback_;

    const Float reversion_level_;    // Multiple of stdev
    Float base_quantity_;

    std::map<std::string, std::pair<Int, std::deque<Float>>> counted_prices_;

    OrderExecutor* executor_;

    std::ofstream csv_;
    util::CsvWriter csv_writer_;
};
}    // namespace profitview