#pragma once

#include "enum.hpp"
#include "order_executor.hpp"
#include "utils.hpp"
#include "wscc_trade_stream.hpp"

#include <csv2/writer.hpp>

#include <fmt/core.h>

#include <boost/log/trivial.hpp>

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
class CcKaufman
    : public TradeStream
    , private ccapi::CcTradeHandler
{
public:
    CcKaufman(
        const std::string trade_stream_name,
        OrderExecutor* executor,
        Int lookback,
        Float base_quantity,
        Int er_period,
        Int fast_sc,
        Int slow_sc,
        Int kama_trend,
        const std::string& csv_name = "Kaufman.csv")
        : ccapi::CcTradeHandler(trade_stream_name)
        , lookback_{lookback}
        , base_quantity_{base_quantity}
        , er_period_{er_period}
        , fast_sc_{fast_sc}
        , slow_sc_{slow_sc}
        , kama_trend_{kama_trend}
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

        auto& [prices, mean_reached, initial_mean, kama, kamas]{price_structure_[trade_data.symbol]};

        prices.emplace_back(trade_data.price);

        auto sc_factor{2.0 / (fast_sc_ + 1) - 2.0 / (slow_sc_ + 1)};
        auto sc_sum{2.0 / (slow_sc_ + 1)};

        if (not mean_reached && prices.size() + 1 == lookback_)
        {
            kama = initial_mean = util::ma(prices);
            BOOST_LOG_TRIVIAL(info) << "Initial mean: " << initial_mean << std::endl << std::endl;
            mean_reached = true;
        }
        else if (mean_reached)
        {
            auto [er_vols, change]{util::abs_differences(prices, er_period_)};
            auto er_vol{util::accumulate(er_vols, 0.0)};
            // Occasionally, the sequence will be constant:
            auto er{er_vol > 0 ? change / er_vol : 0.0};    // leading to er_vol of zero
            auto root_sc{er * sc_factor + sc_sum};
            auto sc{root_sc * root_sc};

            BOOST_LOG_TRIVIAL(info) << "ER: " << er << std::endl;
            BOOST_LOG_TRIVIAL(info) << "SC: " << sc << std::endl;

            // These could be done on the fly but the complexity would distract
            auto mean{util::ma(prices)};

            prices.pop_front();    // Now we have lookback_ prices already, remove the
                                   // oldest

            kamas.emplace_back(kama = kama + sc * (trade_data.price - kama));

            if (kamas.size() > kama_trend_)
            {
                auto [monotonic, up]{util::is_monotonic(std::ranges::subrange{kamas.end() - kama_trend_, kamas.end()})};
                if (not monotonic)    // Signal
                {
                    // @todo This will keep buying/selling when the market is not
                    // directional
                    //       It should have more refined behaviour
                    // @todo FTX market orders are not properly implemented so this puts
                    // limits at the
                    //       current price.  This will leave open orders sometimes.
                    executor_->new_order(
                        trade_data.symbol, up ? Side::Buy : Side::Sell, base_quantity_, OrderType::Market);
                }

                csv_writer_.write_strings(
                    trade_data.symbol,
                    trade_data.price,
                    toString(trade_data.side).data(),
                    trade_data.size,
                    trade_data.source,
                    trade_data.time,
                    kama,
                    monotonic ? (up ? "Up" : "Down") : "Not monotonic");

                kamas.pop_front();    // Remove oldest KAMA price
            }
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
        Float initial_mean, kama;
        std::deque<Float> kamas;
    };

private:
    const Int lookback_;

    double base_quantity_;
    Int er_period_, fast_sc_, slow_sc_, kama_trend_;

    std::map<std::string, Data> price_structure_;

    OrderExecutor* executor_;

    std::ofstream csv_;
    util::CsvWriter csv_writer_;
};

}    // namespace profitview