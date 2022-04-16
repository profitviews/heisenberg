#pragma once

#include "enum.hpp"
#include "cc_trade_stream.hpp"

namespace profitview
{

template<std::floating_point Float = double, std::integral Int = int>
class CcSimpleMR : public CcTradeStream<Float, Int>
{
public:
    CcSimpleMR(
        const std::string trade_stream_name,
        OrderExecutor* executor,
        Int lookback,
        Float reversion_level,
        Float base_quantity,
        const std::string& csv_name = "SimpleMR.csv")
        : CcTradeStream<Float, Int>(trade_stream_name, executor, csv_name)
        , lookback_{lookback}
        , reversion_level_{reversion_level}
        , base_quantity_{base_quantity}
    {}

    void onStreamedTrade(TradeData const& trade_data) override
    {
        trade_data.print();

        auto& [elements, prices]{counted_prices_[trade_data.symbol]};

        prices.emplace_back(trade_data.price);

        if (prices.size() > lookback_)
        {
            auto mean{util::ma(prices)};

            auto std_reversion{reversion_level_ * util::stdev(prices, mean, lookback_)};

            prices.pop_front();
            
            bool 
                sell_signal{trade_data.price > mean + std_reversion},
                buy_signal {trade_data.price < mean - std_reversion};

            if (sell_signal)
            {    
                this->new_order(trade_data.symbol, Side::Sell, base_quantity_, OrderType::Market);
            }
            else if (buy_signal)
            {    
                this->new_order(trade_data.symbol, Side::Buy, base_quantity_, OrderType::Market);
            }

            this->writeCsv(
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

private:
    const Int lookback_;

    const Float reversion_level_;    // Multiple of stdev
    Float base_quantity_;

    std::map<std::string, std::pair<Int, std::deque<Float>>> counted_prices_;
};

}    // namespace profitview