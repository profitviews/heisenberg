#pragma once

#include <ccapi_cpp/ccapi_macro.h>

#include <string>
#include <unordered_map>

namespace profitview
{

/** Map CLI shortcuts (e.g. kraken, coinbase) to ccapi exchange names; pass-through if already canonical. */
inline std::string ccapi_exchange_from_cli(std::string const& exchange_cli)
{
    static const std::unordered_map<std::string, std::string> aliases{
        {"ftx",      CCAPI_EXCHANGE_NAME_FTX     },
        {"coinbase", CCAPI_EXCHANGE_NAME_COINBASE},
        {"bitmex",   CCAPI_EXCHANGE_NAME_BITMEX  },
        {"kraken",   CCAPI_EXCHANGE_NAME_KRAKEN  },
    };
    auto const it = aliases.find(exchange_cli);
    return it != aliases.end() ? it->second : exchange_cli;
}

}    // namespace profitview
