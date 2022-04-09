#pragma once

#include "enum.hpp"
#include "order_executor.hpp"
#include "utils.hpp"
#include "wscc_trade_stream.hpp"

#include <csv2/writer.hpp>

#include <fmt/core.h>

#include <boost/log/trivial.hpp>

#include <concepts>
#include <fstream>
#include <map>
#include <numeric>
#include <ranges>
#include <string>
#include <tuple>
#include <vector>

namespace profitview
{

template<std::floating_point Float = double, std::integral Int = int>
class CcDamped
    : public TradeStream
    , private ccapi::CcTradeHandler
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
        : ccapi::CcTradeHandler(trade_stream_name)
        , lookback_{lookback}
        , reversion_level_{reversion_level}
        , base_quantity_{base_quantity}
        , damping_{damping}
        , executor_{executor}
        , csv_{csv_name}
        , csv_writer_{csv_}
    {}

    void onStreamedTrade(TradeData const& trade_data) override
    {
        util::print_trade_data(trade_data);

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
            // These could be done on the fly but the complexity would distract
            auto mean{util::ma(prices)};

            auto const& damping_factor{damping_ * initial_stdev};

            // Version 1: chopping the tops/bottoms off extremes
            auto const& cut_damped{prices | std::views::transform([&mean, &damping_factor](auto price) -> auto {
                                       return std::abs(price - mean) > damping_factor
                                                ? boost::math::sign(price - mean) * damping_factor
                                                : price;
                                   })};

            // Version 2: excluding extreme prices
            auto excluded_damped{prices | std::views::filter([&mean, &damping_factor](auto price) -> auto {
                                     return std::abs(price - mean) < damping_factor;
                                 })};

            // Using Version 2 this time:
            auto std_reversion{reversion_level_ * util::stdev(excluded_damped, mean, lookback_)};

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

    OrderExecutor* executor_;

    std::ofstream csv_;
    util::CsvWriter csv_writer_;
};

}    // namespace profitview