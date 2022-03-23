#pragma once

#include "enum.hpp"
#include "order_executor.hpp"
#include "wscc_trade_stream.hpp"
#include "utils.hpp"

#include <csv2/writer.hpp>

#include <fmt/core.h>

#include <string>
#include <vector>
#include <fstream>
#include <ranges>
#include <map>
#include <tuple>
#include <numeric>

namespace profitview 
{

class CCKaufman : public TradeStream, private ccapi::CcTradeHandler
{
public:
    CCKaufman
		( const std::string trade_stream_name 
        , OrderExecutor* executor
		, int lookback
		, double base_quantity
        , int er_period
        , int fast_sc
        , int slow_sc
        , int kama_trend
        , const std::string& csv_name = "Kaufman.csv"
	) 
    : ccapi::CcTradeHandler(trade_stream_name)
	, lookback_        {lookback       }
	, base_quantity_   {base_quantity  }
    , er_period_       {er_period      }
    , fast_sc_         {fast_sc        }
    , slow_sc_         {slow_sc        }
    , kama_trend_      {kama_trend     }
	, executor_        {executor       }
    , csv_             {csv_name       }
    , csv_writer_      {csv_           }
    {}

    void onStreamedTrade(profitview::TradeData const& trade_data) override
    {
        fmt::print("Price: {}, ", trade_data.price);
        fmt::print("Side: {}, ", toString(trade_data.side));
        fmt::print("Size: {}, ", trade_data.size);
        fmt::print("Source: {}, ", trade_data.source);
        fmt::print("Symbol: {}, ", trade_data.symbol);
        fmt::print("Time: {}", std::asctime(std::localtime(&trade_data.time)));

        auto& [prices, mean_reached, initial_mean, kama, kamas] { price_structure_[trade_data.symbol]};

        prices.emplace_back(trade_data.price);

        using price_t = decltype(prices)::value_type;

        price_t sc_factor {2.0/(fast_sc_+1) - 2.0/(slow_sc_+1)};
        price_t sc_sum    {2.0/(slow_sc_+1)};

        if (not mean_reached && prices.size() + 1 == lookback_) {
            kama = initial_mean = util::ma(prices, lookback_);
            std::cout << "Initial mean: " << initial_mean << std::endl << std::endl;
            mean_reached = true;
        } else if (mean_reached) {
            auto [er_vols, change]{util::abs_differences(prices, er_period_)};
            auto er_vol{util::accumulate(er_vols, 0.0)};
            // Occasionally, the sequence will be constant:
            auto er{er_vol > 0 ? change/er_vol : 0.0};  // leading to er_vol of zero
            auto root_sc{er*sc_factor + sc_sum};
            auto sc{root_sc*root_sc};

            std::cout << "ER: " << er << std::endl;
            std::cout << "SC: " << sc << std::endl;
            
            // These could be done on the fly but the complexity would distract
            auto mean_value { util::ma(prices, lookback_)};

            prices.pop_front(); // Now we have lookback_ prices already, remove the oldest

            kamas.emplace_back(kama = kama + sc*(trade_data.price - kama));

            if(kamas.size() > kama_trend_)
            {
                auto [monotonic, up] {util::is_monotonic(std::ranges::subrange {kamas.end() - kama_trend_, kamas.end()})};
                if(not monotonic) // Signal
                {
                    // @todo This will keep buying/selling when the market is not directional
                    //       It should have more refined behaviour
                    // @todo FTX market orders are not properly implemented so this puts limits at the 
                    //       current price.  This will leave open orders sometimes.
                    executor_->new_order(trade_data.symbol, up ? Side::Buy : Side::Sell, base_quantity_, OrderType::Limit, trade_data.price);
                }

                csv_writer_.write_strings
                    ( trade_data.symbol
                    , std::to_string(trade_data.price)
                    , toString(trade_data.side).data()
                    , std::to_string(trade_data.size)
                    , trade_data.source
                    , std::to_string(kama)
                    , monotonic ? (up ? "Up" : "Down") : "Not monotonic"
                    );

                kamas.pop_front(); // Remove oldest KAMA price
            }
        }
	}

    void subscribe(const std::string& market, const std::vector<std::string>& symbol_list)
    {
        CcTradeHandler::subscribe(market, symbol_list);
    }

private:

	const int lookback_;

    double base_quantity_;
    int er_period_, fast_sc_, slow_sc_, kama_trend_;

    std::map<std::string, 
             std::tuple<
                std::deque<double>, 
                bool, 
                double, 
                double,
                std::deque<double>
                >
            > price_structure_;

    OrderExecutor* executor_;

    std::ofstream csv_;
    util::CsvWriter csv_writer_;
};

}