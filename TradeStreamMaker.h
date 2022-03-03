#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>

struct TradeData
{
    enum class Side { Buy, Sell };

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
    TradeStream() {}
    virtual ~TradeStream() {}
	virtual void onStreamedTrade(const TradeData& trade_data) = 0;
    virtual void subscribe(const std::string& market, const std::vector<std::string>& symbol_list) = 0;

};

struct TradeStreamException : public std::runtime_error 
{
    TradeStreamException(const std::string message) : std::runtime_error(message){}
    ~TradeStreamException() {}
};

struct TradeStreamMaker
{
    inline static std::map<std::string, std::shared_ptr<TradeStream>> make;

    template<typename TradeStreamT, typename... Args>
    static void register_stream(const std::string& name, Args... args)
    {
        make[name] = std::make_shared<TradeStreamT>(name, args...);
    }
};