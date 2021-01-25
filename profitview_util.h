#pragma once

#include "Poco/Logger.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Array.h"

namespace profitview {
namespace util {
    void log_trade(Poco::Logger&, Poco::JSON::Object::Ptr&);
    Poco::JSON::Array::Ptr constructSymbolJSON(int number_of_symbols, char** symbols);
}}
