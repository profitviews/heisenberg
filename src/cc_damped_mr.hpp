#pragma once

#include "enum.hpp"
#include "order_executor.hpp"
#include "wscc_trade_stream.hpp"
#include "utils.hpp"

#include <csv2/writer.hpp>

#include <fmt/core.h>

#include <concepts>
#include <string>
#include <vector>
#include <fstream>
#include <ranges>
#include <map>
#include <tuple>
#include <numeric>

namespace profitview 
{

template<std::floating_point Float = double, std::integral Int = int>
class CcDamped : public TradeStream, private ccapi::CcTradeHandler
{
public:
    CcDamped
		( const std::string trade_stream_name 
        , OrderExecutor* executor
		, Int lookback
        , Float reversion_level
		, Float base_quantity
        , Float damping
        , const std::string& csv_name = "Damped.csv"
	) 
    : ccapi::CcTradeHandler(trade_stream_name)
	, lookback_        {lookback       }
    , reversion_level_ {reversion_level}
	, base_quantity_   {base_quantity  }
    , damping_         {damping        }
	, executor_        {executor       }
    , csv_             {csv_name       }
    , csv_writer_      {csv_           }
    {}

    void onStreamedTrade(TradeData const& trade_data) override
    {
        fmt::print("Price: {}, ", trade_data.price);
        fmt::print("Side: {}, ", toString(trade_data.side));
        fmt::print("Size: {}, ", trade_data.size);
        fmt::print("Source: {}, ", trade_data.source);
        fmt::print("Symbol: {}, ", trade_data.symbol);
        fmt::print("Time: {}", std::asctime(std::localtime(&trade_data.time)));

        auto& [prices, mean_reached, initial_mean, initial_stdev] { price_structure_[trade_data.symbol]};

        prices.emplace_back(trade_data.price);

        if (not mean_reached and prices.size() + 1 == lookback_) {
            initial_mean = util::ma(prices);
            initial_stdev = util::stdev(prices, initial_mean, lookback_);
            std::cout << "Initial mean: " << initial_mean << std::endl << std::endl;
            mean_reached = true;
        } else if (mean_reached) {
            // These could be done on the fly but the complexity would distract
            auto mean { util::ma(prices)};

            auto const& damping_factor{damping_*initial_stdev};
            auto const& damped {prices | std::views::transform(
                [&mean, &damping_factor](auto v) -> auto
                {   
                    auto const& v_m{v - mean};
                    return std::abs(v_m) > damping_factor 
                        ? boost::math::sign(v_m)*damping_factor 
                        : v;
                })};
            auto std_reversion { reversion_level_*util::stdev(damped, mean, lookback_)};

            prices.pop_front(); // Now we have lookback_ prices already, remove the oldest
            bool 
                sell_condition {trade_data.price > mean + std_reversion},
                buy_condition {trade_data.price < mean - std_reversion};
            if(sell_condition) { // Well greater than the normal volatility
                // so sell, expecting a reversion to the mean
                executor_->new_order(trade_data.symbol, Side::Sell, base_quantity_, OrderType::Market);
            } else if(buy_condition) { // Well less than the normal volatility
                // so buy, expecting a reversion to the mean
                executor_->new_order(trade_data.symbol, Side::Buy, base_quantity_, OrderType::Market);
            }

            csv_writer_.write_strings
                ( trade_data.symbol
                , trade_data.price
                , toString(trade_data.side).data()
                , trade_data.size
                , trade_data.source
                , trade_data.time
                , mean
                , std_reversion
                , buy_condition ? "Buy" : (sell_condition ? "Sell" : "No trade")
                );
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

    std::map< std::string, Data> price_structure_;

    OrderExecutor* executor_;

    std::ofstream csv_;
    util::CsvWriter csv_writer_;
};

}