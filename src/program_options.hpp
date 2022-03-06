#pragma once

#include <boost/program_options.hpp>
#include <fmt/core.h>
#include <concepts>
#include <iostream>
#include <optional>

namespace profitview 
{

template <typename T>
concept CustomProgramOptions = requires(T& t, boost::program_options::options_description& options)
{
	{ t.addOptions(options) } -> std::same_as<void>;
};

std::optional<int> parseProgramOptions(int argc, char *argv[], CustomProgramOptions auto&... options)
{
    namespace po = boost::program_options;
    try {
        po::options_description desc(
            fmt::format("{} Version {}.{}\n\nUsage: ", 
            argv[0], cpp_crypto_algos_VERSION_MAJOR, cpp_crypto_algos_VERSION_MINOR
        ));

        desc.add_options()
            ("help", "produce help message")
        ;
        
        (options.addOptions(desc), ...);

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);    

        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 1;
        }
    }
    catch(po::required_option& e)
    {
        std::cout << e.what() << std::endl;
        return 1;
    }
    return std::nullopt;
}

}