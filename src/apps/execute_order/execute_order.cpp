#include "ccex_order_executor.hpp"
#include "enum.hpp"
#include "exchange_names.hpp"
#include "program_options.hpp"
#include "sandbox_order_executor.hpp"

#include <boost/json.hpp>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>

#include <iostream>
#include <string>

namespace profitview
{

struct ProgramArgs
{
    bool sandbox = false;
    std::string exchange;
    std::string symbol;
    std::string api_key;
    std::string api_secret;
    std::string api_phrase;
    std::string sub_account;
    Side side = Side::Buy;
    double size = 0.0;
    OrderType type = OrderType::Limit;
    double price = 0.0;

    void addOptions(boost::program_options::options_description& options)
    {
        namespace po = boost::program_options;
        // clang-format off
        options.add_options()
            ("sandbox",
             po::bool_switch(&sandbox),
             "Use in-process SandboxOrderExecutor (no API keys or live ccapi; records orders only).")
            ("exchange", po::value(&exchange), "Crypto Exchange to execute on (required for live execution).")
            ("symbol", po::value(&symbol)->required(), "Symbol for cypto assets to trade.")
            ("api_key", po::value(&api_key), "API key for Cypto exchange (required for live execution).")
            ("api_secret", po::value(&api_secret), "API secret for Cypto exchange (required for live execution).")
            ("api_phrase", po::value(&api_phrase), "API phrase for Cypto exchange.")
            ("sub_account", po::value(&sub_account), "Subaccount on Cypto exchange.")
            ("side", po::value(&side)->required(), "The side of the trade <buy|sell>.")
            ("size", po::value(&size)->required(), "Size to trade.")
            ("type", po::value(&type)->required(), "The type of order <limit|market>.")
            ("price", po::value(&price), "Price to trade at.");
        // clang-format on
    }
};

}    // namespace profitview
auto main(int argc, char* argv[]) -> int
{
    using namespace profitview;
    ProgramArgs options;
    auto const result = parseProgramOptions(argc, argv, options);
    if (result)
        return result.value();

    if (!options.sandbox)
    {
        if (options.exchange.empty() || options.api_key.empty() || options.api_secret.empty())
        {
            BOOST_LOG_TRIVIAL(error)
                << "exchange, api_key, and api_secret are required for live execution (use --sandbox to skip)."
                << std::endl;
            return 1;
        }
    }

    if (options.sandbox)
    {
        BOOST_LOG_TRIVIAL(info) << "Sandbox mode: recording order locally (no ccapi session)." << std::endl;
        SandboxOrderExecutor executor{};
        executor.new_order(options.symbol, options.side, options.size, options.type, options.price);
        for (auto const& rec : executor.recorded_orders())
        {
            BOOST_LOG_TRIVIAL(info) << "recorded: symbol=" << rec.symbol << " side=" << toString(rec.side)
                                    << " qty=" << rec.order_qty << " type=" << toString(rec.type)
                                    << " price=" << rec.price << std::endl;
        }
        return 0;
    }

    BOOST_LOG_TRIVIAL(info) << "Running Ccex test.";

    CcexOrderExecutor executor{
        ccapi_exchange_from_cli(options.exchange), options.api_key, options.api_secret, options.api_phrase, options.sub_account};

    BOOST_LOG_TRIVIAL(info) << options.symbol << "Running: " << std::endl;
    executor.new_order(options.symbol, options.side, options.size, options.type, options.price);

    enum
    {
        OrderId,
        Symbol,
        OrderSide,
        Size,
        Price,
        Time,
        Status
    };
    for (const auto& [cid, details] : executor.get_open_orders())
        BOOST_LOG_TRIVIAL(info) << "cid: " << cid << ", symbol: " << std::get<OrderId>(details)
                  << ", status: " << std::get<Status>(details) << std::endl;
    return 0;
}