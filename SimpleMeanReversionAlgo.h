#pragma once
#include "MarketDataAdapter.h"

#include <vector>

class SimpleMeanReversionAlgo : public MarketDataAdapter
{
public:
	SimpleMeanReversionAlgo() = delete;
	SimpleMeanReversionAlgo(const std::string&, const std::string&, int = 50);
	~SimpleMeanReversionAlgo();

	void onTrade(const void *pSender, Array::Ptr &arg);
private:

	double mean() { return mean_; }

	std::string bitmex_api_key_;
	std::string bitmex_secret_;
	int lookback_;

	int count_;
	double mean_;
	double d_squared_;
};