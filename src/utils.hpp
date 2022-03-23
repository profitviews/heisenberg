#pragma once

#include <boost/range/numeric.hpp>
#include <boost/math/special_functions/sign.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>

#include <csv2/writer.hpp>

#include <iostream>
#include <numeric>
#include <ranges>
#include <tuple>
#include <limits>
#include <cmath>
#include <cassert>

namespace profitview::util
{

// boost::accumulate should do this, but there's a compile fail
inline auto accumulate(const auto& s, auto i) -> auto
{
    return std::accumulate(std::begin(s), std::end(s), i);
}

inline auto accumulate(const auto& s, auto i, auto op) -> auto
{
    return std::accumulate(std::begin(s), std::end(s), i, op);
}

auto ma(auto const& s, int p) -> auto
{
    return accumulate(s, 0.0)/p;
}

// Exponential Moving Average (ema) difference formula from https://en.wikipedia.org/wiki/Moving_average:
double ema(auto const& s, int p, double m = 0.0f)
{
    auto alpha{2.0f/(p -1)};
    auto const ema_step{[&alpha](auto a, const auto& price) 
    {
        return price*alpha + a*(1 - alpha);
    }};

    return accumulate(s, m, ema_step);
}

double stdev(auto const& s, double m, int p, double d = std::numeric_limits<double>::max())
{   // Calculate standard deviation given mean (m)
    auto const variance {[&m, &p](auto a, const auto& v) 
    {
        return a + (v - m)*(v - m) / (p - 1);
    }};
    auto const& t {s | std::views::transform([&m, &d](auto v) -> auto
    {
        auto const& v_m{v - m};
        return std::abs(v_m) > d ? boost::math::sign(v_m)*d : v_m;
    })};
    return std::sqrt(accumulate(t, 0.0, variance));
}

template <typename Prices> // Couldn't get this to work with (const auto& prices,...)
auto abs_differences(const Prices& prices, int e) 
    -> std::tuple<std::vector<typename Prices::value_type>, typename Prices::value_type>
{
    auto b {prices.end() - e};
    assert(prices.size() > e);

    using namespace std::ranges;

    subrange 
        lagged  {b - 1, prices.end() - 1},
        aligned {b,     prices.end()    };
    std::vector<typename Prices::value_type> differences(e);
    transform(lagged, aligned, differences.begin(), [](auto n, auto m) -> auto { return std::abs(n - m);});

    return {differences, std::abs(prices.back() - *b)};
}

auto is_monotonic(auto const& s) -> std::tuple<bool, bool> // { monotonic, up }
{ // If monotonic     ascending      { true,  true  }
  // If monotonic     descending     { true,  false }
  // If non-monotonic now ascending  { false, true  }
  // If non-monotonic now descending { false, false }
    auto sgn = [](int a, int b) {
        return a < b ? 1 : (a == b) ? 0 : -1;
    };
    int prev = 0;
    for (int i = 0; i < s.size() - 1; ++ i) {
        int c = sgn(s[i], s[i + 1]);
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
{ // Provides shorthand `write_strings()`
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