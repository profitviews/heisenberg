#include "program_options.hpp"

#include <utils.hpp>

#include <boost/json.hpp>
#include <boost/program_options.hpp>
#include <boost/log/trivial.hpp>

#include <ranges>

#include <iostream>

namespace profitview
{

struct ProgramArgs
{
    std::string name;
    int er_period;
    // std::string symbol;
    // std::string api_key;
    // std::string api_secret;
    // std::string api_phrase;
    // double size = 0.0;
    // double price = 0.0;

    void addOptions(boost::program_options::options_description& options)
    {
        namespace po = boost::program_options;
        // clang-format off
        options.add_options()
            ("name", po::value(&name)->required(), "Name of experiment")
            ("er_period", po::value(&er_period)->required(), "Efficiency ratio lookback period");
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

    BOOST_LOG_TRIVIAL(info) << "Running experiments" << std::endl;

    BOOST_LOG_TRIVIAL(info) << " Running: " << options.name << std::endl;

    std::vector<double> a{5, 6, 7, 10, 11, 12, 5, 6, 7, 10, 11, 12, 5, 6, 7, 10, 11, 12, 5, 6, 7, 10, 11, 12};
    std::ofstream exp{"experiment.csv"};
    profitview::util::CsvWriter cw{exp};

    auto [d, c]{profitview::util::abs_differences(a, options.er_period)};

    cw.write(a[0], d[0], c, 5.5);

    cw.write("Hello", "there", std::to_string(20.5), a[0], d[0], c, 5.5);

    using namespace std::ranges;

    copy(d, std::ostream_iterator<decltype(d)::value_type>(std::cout, " "));
    BOOST_LOG_TRIVIAL(info) << std::endl << "Change: " << c << std::endl;

    subrange s1{a.begin(), a.begin() + 5};
    subrange s2{a.begin() + 3, a.begin() + 7};
    subrange s3{a.begin(), a.begin() + 10};
    subrange s4{a.begin() + 5, a.begin() + 6};

    auto [s1m, s1d]{util::is_monotonic(s1)};
    auto [s2m, s2d]{util::is_monotonic(s2)};
    auto [s3m, s3d]{util::is_monotonic(s3)};
    auto [s4m, s4d]{util::is_monotonic(s4)};

    std::cout << std::boolalpha << s1m << " " << s1d << std::endl
              << s2m << " " << s2d << std::endl
              << s3m << " " << s3d << std::endl
              << s4m << " " << s4d << std::endl
              << std::endl;

    return 0;
}