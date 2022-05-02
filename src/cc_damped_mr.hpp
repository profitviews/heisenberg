#pragma once

#include "enum.hpp"
#include "cc_trade_stream.hpp"
#include "utils.hpp"
#include "trade_data.hpp"

#include <boost/log/trivial.hpp>

#include <concepts>
#include <map>

namespace profitview
{

template<std::floating_point Float = double, std::integral Int = int>
class CcDamped : public CcTradeStream<Float, Int>
{
public:
    CcDamped(
        const std::string trade_stream_name,
        OrderExecutor* executor,
        Int lookback,
        Float reversion_level,
        Float base_quantity,
        Float damping,
        const std::string& csv_name = "Damped.csv")
        : CcTradeStream<Float, Int>(trade_stream_name, executor, csv_name)
        , lookback_{lookback}
        , reversion_level_{reversion_level}
        , base_quantity_{base_quantity}
        , damping_{damping}
    {}

    void onStreamedTrade(TradeData const& trade_data) override
    {
        trade_data.print();

        auto& [prices, mean_reached, initial_mean, initial_stdev]{price_structure_[trade_data.symbol]};

        prices.emplace_back(trade_data.price);

        if (not mean_reached and prices.size() > lookback_)
        {
            initial_mean = util::ma(prices);
            initial_stdev = util::stdev(prices, initial_mean, lookback_);
            BOOST_LOG_TRIVIAL(info) << "Initial mean: " << initial_mean << std::endl << std::endl;
            mean_reached = true;
        }
        else if (mean_reached)
        {
            auto mean{util::ma(prices)};
            BOOST_LOG_TRIVIAL(info) << "MA: " << mean << std::endl << std::endl;

            auto const& damping_factor{damping_ * initial_stdev};

            // Version 1: chopping the tops/bottoms off outliers
            auto const& cut_damped{prices | std::ranges::views::transform([&mean, &damping_factor](auto price) -> auto {
                                       return std::abs(price - mean) > damping_factor
                                                ? boost::math::sign(price - mean) * damping_factor + mean
                                                : price;
                                   })};

            // Version 2: excluding outliers
            auto excluded_damped{prices | std::ranges::views::filter([&mean, &damping_factor](auto price) -> auto {
                                     return std::abs(price - mean) < damping_factor;
                                 })};

            // Using Version 2 this time:
            auto std_reversion{reversion_level_ * util::stdev(excluded_damped, mean, lookback_)};
            BOOST_LOG_TRIVIAL(info) << "Std Reversion: " << std_reversion << std::endl << std::endl;

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

    struct Data
    {
        std::deque<Float> prices;
        bool mean_reached;
        Float initial_mean, initial_stdev;
    };

private:
    const Int lookback_;
    const Float reversion_level_;
    const Float base_quantity_;
    const Float damping_;

    std::map<std::string, Data> price_structure_;
};

}    // namespace profitview