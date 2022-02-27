#include "cpp_crypto_algosConfig.h"
#include "SimpleMR.h"
#include <TradeStreamMaker.h>
#include <SIOPocoTradeStream.h>
#include <profitview_util.h>
#include <CcexOrderExecutor.h>
#include <SIOClient.h>

#include <iostream>

class SIOPocoSimpleMR : public TradeStream, private SIOPocoStream
{
public:
    SIOPocoSimpleMR
		( const std::string& profitview_api_key
		, OrderExecutor* executor
		, int lookback
		, double reversion_level
		, double base_quantity
		, const std::string trade_stream_name
	) 
    : SIOPocoStream(profitview_api_key, trade_stream_name)
	, lookback_        {lookback       }
	, reversion_level_ {reversion_level}
	, base_quantity_   {base_quantity  }
	, executor_        {executor       }
    {
    }

    ~SIOPocoSimpleMR()
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
            auto& [elements, prices] { counted_prices_[trade_data.symbol]};

        prices.emplace_back(trade_data.price);

        using Side = OrderExecutor::Side;
        using OrderType = OrderExecutor::OrderType;

        if(elements + 1 < lookback_) {
            ++elements; // Accumulate up to lookback_ prices
        } else {
            // These could be done on the fly but the complexity would distract
            auto mean_value { mean(prices)};
            double std_reversion { reversion_level_*stdev(prices, mean_value)};

            prices.pop_front(); // Now we have lookback_ prices already, remove the oldest

            logger_.info("Mean: " + std::to_string(mean_value));
            logger_.info("Standard reversion: " + std::to_string(std_reversion));

            if(trade_data.price > mean_value + std_reversion) { // Well greater than the normal volatility
                // so sell, expecting a reversion to the mean
                executor_->new_order(trade_data.symbol, Side::sell, base_quantity_, OrderType::limit, trade_data.price);
            }
            else if(trade_data.price < mean_value - std_reversion) { // Well less than the normal volatility
                // so buy, expecting a reversion to the mean
                executor_->new_order(trade_data.symbol, Side::buy, base_quantity_, OrderType::limit, trade_data.price);
            }
        }
	}

    void subscribe(const std::string& market, const std::vector<std::string>& symbol_list)
    {
        SIOPocoStream::subscribe(market, symbol_list);
    }
private:

	const int lookback_;

    const double reversion_level_; // Multiple of stdev
    double base_quantity_;

    std::map<std::string, std::pair<int, std::deque<double>>> counted_prices_;

    OrderExecutor* executor_;

    profitview::util::Logger logger_;

    template<typename Sequence>
    double mean(const Sequence& sequence) const
    {
        return std::accumulate(sequence.begin(), sequence.end(), 0.0)/lookback_;
    }

    template<typename Sequence>
    double stdev(const Sequence& sequence, double m) const
    {
        auto variance {[&m, this](auto accumulator, const auto& val) {
            return accumulator + (val - m)*(val - m) / (lookback_ - 1);
        }};

        return std::sqrt(std::accumulate(sequence.begin(), sequence.end(), 0.0, variance));
    }
};

int main(int argc, char *argv[])
{
	enum{ name_arg
		, exchange_arg
		, api_key_arg
		, api_secret_arg
		, profitview_api_arg
		, lookback_arg
		, reversion_level_arg
		, base_quantity_arg
		, symbol_args 
		};

	if (argc < symbol_args) {
		// report version
		std::cout 
			<< argv[0]  
			<< " Version " 
			<< cpp_crypto_algos_VERSION_MAJOR << "."
			<< cpp_crypto_algos_VERSION_MINOR << std::endl;
		std::cout 
			<< "Usage: " << argv[name_arg] << " exchange_key exchange_secret api_key lookback reversion_multiple base_quantity data_type:market:symbol [data_type:market:symbol ...]" << std::endl;
		return 1;
	}
    profitview::util::Logger logger;

	CcexOrderExecutor executor{argv[exchange_arg], 5, argv[api_key_arg], argv[api_secret_arg]};

	logger.info("Creating URI\n");

	std::string symbols{argv[symbol_args]};
	auto first_colon = symbols.find(':');
	auto second_colon = symbols.find(':', first_colon + 1);
	auto market{symbols.substr(first_colon + 1, second_colon - first_colon - 1)};
	auto symbol{symbols.substr(second_colon + 1)};

	std::vector<std::string> symbol_vector{symbol};
	TradeStreamMaker::register_stream<SIOPocoSimpleMR>("SIOPocoSimpleMR", 
		argv[profitview_api_arg], 
		&executor, 
		std::stoi(argv[lookback_arg]),
		std::stod(argv[reversion_level_arg]), 
		std::stod(argv[base_quantity_arg]),
		"SIOPocoSimpleMR");

	TradeStreamMaker::make["SIOPocoSimpleMR"]->subscribe(market, symbol_vector);

	std::cout << "Press enter to quit" << std::endl;
	std::cin.get();

	return 0;
}