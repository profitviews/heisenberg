#pragma once
#include <MarketDataAdapter.h>
#include <OrderExecutor.h>
#include <boost/json.hpp>
#include <deque>
#include <vector>
#include <map>

class TalibMeanReversion : public MarketDataAdapter
{
public:
	TalibMeanReversion() = delete;
	TalibMeanReversion(OrderExecutor&, int = 50, double = 3.0f, double = 0.0f);
	~TalibMeanReversion();

	void onTrade(const void *, Array::Ptr &arg);

private:

	const int lookback_;

    const double reversion_level_; // Multiple of stdev
    double base_quantity_;

    std::map<std::string, std::pair<int, std::deque<double>>> counted_prices_;

    OrderExecutor& exchange_;

    boost::json::object result_;

    template<typename Sequence>
    double stdev(const Sequence& sequence) const;

};