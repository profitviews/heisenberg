#pragma once

#include <numeric>
#include <cmath>

namespace profitview::util
{

double mean(auto const& sequence, int lookback)
{
    return std::accumulate(sequence.begin(), sequence.end(), 0.0)/lookback;
}

template<typename Sequence>
double stdev(const Sequence& sequence, double m, int lookback)
{   // Calculate standard deviation given mean (m)
    auto const variance {[&m, &lookback](auto accumulator, const auto& val) {
        return accumulator + (val - m)*(val - m) / (lookback - 1);
    }};

    return std::sqrt(std::accumulate(sequence.begin(), sequence.end(), 0.0, variance));
}

}