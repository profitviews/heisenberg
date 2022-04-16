#pragma once

#include "side.hpp"
#include "trade_data.hpp"
#include "trade_stream.hpp"
#include "trade_stream_exception.hpp"

#include <memory>
#include <string>
#include <unordered_map>

namespace profitview
{

struct TradeStreamMaker
{
public:
    template<typename TradeStreamT, typename... Args>
    static auto register_stream(std::string const& name, Args&&... args) -> auto
    {
        return made[name] = std::make_shared<TradeStreamT>(name, std::forward<Args>(args)...);
    }

    static TradeStream& get(std::string const& name) { return *made.at(name); }

private:
    inline static std::unordered_map<std::string, std::shared_ptr<TradeStream>> made;
};

}    // namespace profitview
