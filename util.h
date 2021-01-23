#pragma once

#include "Poco/Logger.h"
#include "Poco/JSON/Parser.h"

namespace util {
    void log_trade(Poco::Logger&, Poco::JSON::Object::Ptr&);
} 
