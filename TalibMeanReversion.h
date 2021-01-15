#pragma once
#include "MarketDataAdapter.h"
#include <Exchange.h>
#include <deque>
#include <vector>
#include <map>

class TalibMeanReversion : public MarketDataAdapter
{
public:
	TalibMeanReversion() = delete;
	TalibMeanReversion(Exchange&, int = 50, double = 3.0f, int = 20);
	~TalibMeanReversion();

	void onTrade(const void *pSender, Array::Ptr &arg);

private:

	const int lookback_;

    const double reversion_level_; // Multiple of stdev
    int base_quantity_;

    std::map<std::string, std::pair<int, std::deque<double>>> counted_prices_;

    Exchange& exchange_;

    template<typename Sequence>
    double stdev(const Sequence& sequence) const;

};