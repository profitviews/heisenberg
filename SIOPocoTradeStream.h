#include "TradeStreamMaker.h"

#include "profitview_poco.h"

#include <SIOClient.h>
#include <Poco/URI.h>
#include <SIOEventTarget.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Net/NetException.h>
#include <Poco/Any.h>

#include <iterator>

using Poco::Net::NetException;
using Poco::JSON::Parser;
using Poco::Dynamic::Var;

class SIOPocoStream : public SIOEventTarget
{
public:
    SIOPocoStream(const std::string& profitview_api_key, const std::string& trade_stream_name) 
    : sio_user_client_ {nullptr}
    , trade_stream_name_ {trade_stream_name}
    {
        // Establish the socket.io connection
        Poco::URI connect_uri{"https://markets.profitview.net"};
        Poco::URI::QueryParameters qp {{"api_key", profitview_api_key}};

        connect_uri.setQueryParameters(qp);
        sio_user_client_ = SIOClient::connect(connect_uri.toString(), qp);

        if(sio_user_client_ == nullptr) throw TradeStreamException("Couldn't connect to URI");

        sio_user_client_->on("trade", this, callback(&SIOPocoStream::onTrade));
    }

    void subscribe(const std::string& market, const std::vector<std::string>& symbol_list)
    {
        sio_user_client_->emit("subscribe", construct_symbol_json_string(market, symbol_list));
    }

    void onTrade(const void *pSender, Array::Ptr &arg)
    {
        profitview::poco::Logger logger;
        auto result{arg->getElement<std::string>(0)};

        Parser parser;
        Var result_json{parser.parse(result)};

        using Poco::JSON::Object;

        auto result_object{result_json.extract<Object::Ptr>()};

        auto price{result_object->get("price").convert<double>()};
        auto side{result_object->get("side").toString()};
        auto size{result_object->get("size").convert<double>()};
        auto source{result_object->get("src")};
        auto symbol{result_object->get("sym")};

        logger.log_trade(result_object);

        time_t date_time{result_object->get("time").convert<time_t>()};
        logger.info("Time: " + std::string{std::asctime(std::localtime(&date_time))});
        std::cout << "trade_stream_name_: " << trade_stream_name_ << std::endl;

        TradeStreamMaker::make.at(trade_stream_name_)->onStreamedTrade(
            { price
            , side == "S" ? TradeData::Side::Sell : TradeData::Side::Buy
            , size
            , source
            , symbol
            , date_time
            });
    }

    ~SIOPocoStream()
    {
        sio_user_client_->disconnect();
    }

private:
    SIOClient* sio_user_client_;
    const std::string trade_stream_name_; // @note Can't be a reference (why? not understood)
    Poco::JSON::Array::Ptr construct_symbol_json_string(const std::string& market, const std::vector<std::string>& symbol_list)
    {
        std::string symbols_json_string{"[\""};
        const std::string symbol_prefix{"trade:" + market + ":"};
        symbols_json_string += symbol_prefix + symbol_list.front() + std::string{"\""};
        
        for(auto symbol{symbol_list.begin() + 1}; symbol != symbol_list.end(); ++symbol)
            symbols_json_string += ",\"" + symbol_prefix + *symbol;
        symbols_json_string += "]";

        using namespace Poco::JSON;
        return Parser().parse(symbols_json_string).extract<Array::Ptr>();
    }
};

class SIOPocoTradeStream : public TradeStream, private SIOPocoStream
{
public:
    SIOPocoTradeStream(
        const std::string& trade_stream_name, const std::string& profitview_api_key) 
    : SIOPocoStream(profitview_api_key, trade_stream_name)
    {
    }

    ~SIOPocoTradeStream()
    {
    }

    void onStreamedTrade(const TradeData& trade_data) override
    {
        std::cout << "Price: " << trade_data.price << std::endl;
        std::cout << "Side: " << (trade_data.side == TradeData::Side::Buy ? "Buy" : "Sell") << std::endl;
        std::cout << "Size: " << trade_data.size << std::endl;
        std::cout << "Source: " << trade_data.source << std::endl;
        std::cout << "Symbol: " << trade_data.symbol << std::endl;
        std::cout << "Time: " << std::string{std::asctime(std::localtime(&trade_data.time))} << std::endl;
    }

    void subscribe(const std::string& market, const std::vector<std::string>& symbol_list)
    {
        SIOPocoStream::subscribe(market, symbol_list);
    }

};