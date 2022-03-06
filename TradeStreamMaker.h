#pragma once

#include "side.hpp"

#include <map>
#include <memory>
#include <string>
#include <stdexcept>
#include <vector>

namespace profitview 
{

struct TradeData
{
    double price;
    Side side;
    double size;
    std::string source;
    std::string symbol;
    time_t time;
};

class TradeStream 
{
public:
    TradeStream() = default;
    virtual ~TradeStream() = default;
	virtual void onStreamedTrade(TradeData const& trade_data) = 0;
    virtual void subscribe(std::string const& market, std::vector<std::string> const& symbol_list) = 0;
};

struct TradeStreamException : public std::runtime_error 
{
    TradeStreamException(std::string const& message)
    :   std::runtime_error(message) 
    {}
};

struct TradeStreamMaker
{
public:
    template<typename TradeStreamT, typename... Args>
    static void register_stream(std::string const& name, Args&&... args)
    {
        made[name] = std::make_shared<TradeStreamT>(name, std::forward<Args>(args)...);
    }

    static TradeStream& get(std::string const& name)
    {
        return *made.at(name);
    }

private:
    inline static std::map<std::string, std::shared_ptr<TradeStream>> made;
};

}
