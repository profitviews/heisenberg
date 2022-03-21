#pragma once

#include <boost/range/numeric.hpp>

#include <csv2/writer.hpp>

#include <iostream>
#include <numeric>
#include <ranges>
#include <tuple>
#include <cmath>
#include <cassert>

namespace profitview::util
{

double ma(auto const& sequence, int lookback)
{
    return std::accumulate(sequence.begin(), sequence.end(), 0.0)/lookback;
}

// Exponential Moving Average (ema) difference formula from https://en.wikipedia.org/wiki/Moving_average:
double ema(auto const& sequence, int lookback, double m = 0.0f)
{
    auto alpha{2.0f/(lookback -1)};
    auto const ema_step{[&alpha](auto accumulator, const auto& price) 
    {
        return price*alpha + accumulator*(1 - alpha);
    }};

    return boost::accumulate(sequence, m, ema_step);
}

double stdev(auto const& sequence, double m, int lookback)
{   // Calculate standard deviation given mean (m)
    auto const variance {[&m, &lookback](auto accumulator, const auto& val) 
    {
        return accumulator + (val - m)*(val - m) / (lookback - 1);
    }};

    return std::sqrt(boost::accumulate(sequence, 0.0, variance));
}

template <typename Prices> // Couldn't get this to work with (const auto& prices,...)
auto abs_differences(const Prices& prices, int extent) 
    -> std::tuple<std::vector<typename Prices::value_type>, typename Prices::value_type>
{
    auto period_begin {prices.end() - extent};
    assert(prices.size() > extent);

    std::ranges::subrange lagged  {period_begin - 1, prices.end() - 1};
    std::ranges::subrange aligned {period_begin,     prices.end()    };
    std::vector<typename Prices::value_type> differences(extent);
    std::ranges::transform(lagged, aligned, differences.begin(), [](auto n, auto m) -> auto { return std::abs(n - m);});

    return {differences, std::abs(prices.back() - *period_begin)};
}

auto is_monotonic(auto const& sequence) -> std::tuple<bool, bool> { // { monotonic, up }
    auto sgn = [](int a, int b) {
        return a < b ? 1 : (a == b) ? 0 : -1;
    };
    int prev = 0;
    for (int i = 0; i < sequence.size() - 1; ++ i) {
        int c = sgn(sequence[i], sequence[i + 1]);
        if (c != 0) {
            if (c != prev && prev != 0) {
                return {false, c == 1}; 
            }
            prev = c;
        }
    }
    return {true, prev == 1};
}

class CsvWriter : public csv2::Writer<csv2::delimiter<','>>
{
public:
    template <typename Stream>
    CsvWriter(Stream&& stream) : csv2::Writer<csv2::delimiter<','>>(stream) {}

    template <typename... Args>
    void write_strings(Args... args)
    {
        this->write_row(std::vector<std::string>({args...}));
    }
};
}