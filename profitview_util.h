#pragma once

#include <Poco/Logger.h>
#include <Poco/ConsoleChannel.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Array.h>

#include <numeric>

namespace profitview {
namespace util {
struct Logger {
    Logger() : logger_{Poco::Logger::get("example")}
    {
        logger_.setChannel(new Poco::ConsoleChannel());
    }
    void log_trade(Poco::JSON::Object::Ptr&);
    void info(const std::string& i) { logger_.information(i); }
    void error(const std::string& e) { logger_.error(e); }
    Poco::Logger& logger_;
};

Poco::JSON::Array::Ptr constructSymbolJSON(int number_of_symbols, char** symbols);

template<typename Sequence>
double mean(const Sequence& sequence, int lookback)
{
    return std::accumulate(sequence.begin(), sequence.end(), 0.0)/lookback;
}

template<typename Sequence>
double stdev(const Sequence& sequence, double m, int lookback)
{
    auto variance {[&m, &lookback](auto accumulator, const auto& val) {
        return accumulator + (val - m)*(val - m) / (lookback - 1);
    }};

    return std::sqrt(std::accumulate(sequence.begin(), sequence.end(), 0.0, variance));
}
}}
