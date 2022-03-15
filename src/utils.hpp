#pragma once

#include <cmath>
#include <numeric>

namespace profitview::util
{

double ma(auto const& sequence, int lookback)
{
    return std::accumulate(sequence.begin(), sequence.end(), 0.0)/lookback;
}

double stdev(auto const& sequence, double m, int lookback)
{   // Calculate standard deviation given mean (m)
    auto const variance {[&m, &lookback](auto accumulator, const auto& val) {
        return accumulator + (val - m)*(val - m) / (lookback - 1);
    }};

    return std::sqrt(std::accumulate(sequence.begin(), sequence.end(), 0.0, variance));
}

}