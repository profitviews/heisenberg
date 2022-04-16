#include "program_options.hpp"
#include "trade_stream_maker.hpp"
#include "wscc_trade_stream.hpp"

#include <boost/program_options.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

namespace profitview
{

struct ProgramArgs
{
    std::string exchange;
    std::vector<std::string> symbols;

    void addOptions(boost::program_options::options_description& options)
    {
        namespace po = boost::program_options;
        // clang-format off 
        options.add_options()
            ("exchange", po::value(&exchange)->required(), "Crypto Exchange to execute on.")
            ("symbols", po::value(&symbols)->multitoken()->required(), "Symbols for cypto assets to trade.");
        // clang-format on
    }
};

}    // namespace profitview

int main(int argc, char* argv[])
{
    using namespace profitview;
    ProgramArgs options;
    auto const result = profitview::parseProgramOptions(argc, argv, options);
    if (result)
        return result.value();

    auto stream {TradeStreamMaker::register_stream<WSCcTradeStream>("WSCcStream")};
    stream->subscribe(options.exchange, options.symbols);

    std::cout << "Press enter to quit" << std::endl;
    std::cin.get();
}