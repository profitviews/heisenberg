#pragma once

#include "enum.hpp"
#include "order_executor.hpp"
#include "utils.hpp"
#include "wscc_trade_stream.hpp"

#include <csv2/writer.hpp>

#include <concepts>
#include <fstream>
#include <string>

namespace profitview
{

template<std::floating_point Float = double, std::integral Int = int>
class CcTradeStream
    : public TradeStream
    , private ccapi::CcTradeHandler
{
public:
    CcTradeStream(
        const std::string trade_stream_name,
        OrderExecutor* executor,
        const std::string& csv_name)
        : ccapi::CcTradeHandler(trade_stream_name)
        , executor_{executor}
        , csv_{csv_name}
        , csv_writer_{csv_}
    {}

    void subscribe(const std::string& market, const std::vector<std::string>& symbol_list)
    {
        CcTradeHandler::set_stream(this);
        CcTradeHandler::subscribe(market, symbol_list);
    }

    void writeCsv(auto&&... args)
    {
        csv_writer_.write(std::forward<decltype(args)>(args)...);
    }

protected:

    auto new_order(std::string const& symbol, Side side, double orderQty, OrderType type, double price = 0.0) -> auto
    {
        return executor_->new_order(symbol, side, orderQty, type, price);
    }

private:
    OrderExecutor* executor_;
    std::ofstream csv_;
    util::CsvWriter csv_writer_;
};

}    // namespace profitview