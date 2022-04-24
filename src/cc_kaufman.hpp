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
class CcKaufman : public CcTradeStream<Float, Int>
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
        : CcTradeStream<Float, Int>(trade_stream_name, executor, csv_name)
        , lookback_{lookback}
        , base_quantity_{base_quantity}
        , er_period_{er_period}
        , fast_sc_{fast_sc}
        , slow_sc_{slow_sc}
        , kama_trend_{kama_trend}
    {}

    void onStreamedTrade(TradeData const& trade_data) override
    {
        trade_data.print();

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
                    this->new_order(trade_data.symbol, up ? Side::Buy : Side::Sell, base_quantity_, OrderType::Market);
                }

                this->writeCsv(
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

};

}    // namespace profitview