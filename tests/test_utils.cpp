#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <utils.hpp>

#include <cmath>
#include <vector>

namespace pv = profitview::util;

TEST_CASE("util::ma mean of doubles", "[utils]")
{
    std::vector<double> const v{1.0, 2.0, 3.0, 4.0};
    REQUIRE(pv::ma(v) == Catch::Approx(2.5));
    REQUIRE(pv::ma(v, 4) == Catch::Approx(2.5));
}

TEST_CASE("util::stdev sample std dev", "[utils]")
{
    std::vector<double> v{1.0, 2.0, 3.0, 4.0};
    double const m = 2.5;
    int const p = 4;
    // Sum of squared deviations = 5.0; divide by (p-1)=3 -> variance 5/3
    REQUIRE(pv::stdev(v, m, p) == Catch::Approx(std::sqrt(5.0 / 3.0)));
}

TEST_CASE("util::ema exponential moving aggregate", "[utils]")
{
    std::vector<double> const s{10.0, 20.0};
    double const p = 5.0;
    // alpha = 2/(p-1) = 0.5; accumulate from m=0: first 10 -> 5; second 20 -> 10 + 2.5 = 12.5
    REQUIRE(pv::ema(s, p, 0.0) == Catch::Approx(12.5));
}

TEST_CASE("util::abs_differences window", "[utils]")
{
    std::vector<double> const prices{1.0, 2.0, 3.0, 4.0, 5.0};
    int const e = 2;
    auto const [diffs, span] = pv::abs_differences(prices, e);
    REQUIRE(diffs.size() == static_cast<size_t>(e));
    REQUIRE(diffs[0] == Catch::Approx(1.0));
    REQUIRE(diffs[1] == Catch::Approx(1.0));
    REQUIRE(span == Catch::Approx(1.0));
}

TEST_CASE("util::is_monotonic ascending", "[utils]")
{
    std::vector<int> const up{1, 2, 3};
    auto const [mono, ascending] = pv::is_monotonic(up);
    REQUIRE(mono == true);
    REQUIRE(ascending == true);
}

TEST_CASE("util::is_monotonic descending", "[utils]")
{
    std::vector<int> const down{3, 2, 1};
    auto const [mono, ascending] = pv::is_monotonic(down);
    REQUIRE(mono == true);
    REQUIRE(ascending == false);
}

TEST_CASE("util::is_monotonic non-monotonic", "[utils]")
{
    std::vector<int> const zig{1, 3, 2};
    auto const [mono, ascending] = pv::is_monotonic(zig);
    REQUIRE(mono == false);
    REQUIRE(ascending == false);
}
