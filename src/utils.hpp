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
#include <type_traits>
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
auto ema(auto const& s, auto p, auto m = 0) -> auto
{
    auto alpha{2.0f/(p - 1)};
    auto const ema_step{[&alpha](auto a, const auto& price) 
    {
        return price*alpha + a*(1 - alpha);
    }};

    return accumulate(s, m, ema_step);
}

auto stdev(auto const& s, auto m, int p) -> auto
{   // Calculate standard deviation given mean (m)
    auto const variance {[&m, &p](auto a, const auto& v) 
    {
        auto const& v_m{v - m};
        return a + v_m*v_m/(p - 1);
    }};
    return std::sqrt(accumulate(s, 0.0, variance));
}

auto abs_differences(const auto& prices, int e) -> auto
{
    auto b {prices.end() - e};
    assert(prices.size() > e);

    using namespace std::ranges;
    
    subrange 
        lagged  {b - 1, prices.end() - 1},
        aligned {b,     prices.end()    };
    std::vector<typename std::remove_cvref_t<decltype(prices)>::value_type> differences(e);
    transform(lagged, aligned, differences.begin(), [](auto n, auto m) -> auto { return std::abs(n - m);});

    return std::make_tuple(differences, prices.back() - *b);
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