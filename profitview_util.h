#pragma once

#include <Poco/Logger.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Array.h>

namespace profitview {
namespace util {
struct Logger {
    Logger() : logger_{Poco::Logger::get("example")}{}
    void log_trade(Poco::JSON::Object::Ptr&);
    void info(const std::string& i) { logger_.information(i); }
    void error(const std::string& e) { logger_.error(e); }
    Poco::Logger& logger_;
};

Poco::JSON::Array::Ptr constructSymbolJSON(int number_of_symbols, char** symbols);
}}
