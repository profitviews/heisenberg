#pragma once
#include "MarketDataAdapter.h"

#include <Bitmex.h>

#include <vector>
#include <map>

class SimpleMeanReversionAlgo : public MarketDataAdapter
{
public:
    class RunningStats
    {
    public:
        RunningStats()
        : count_     { 0    }
        , mean_      { 0.0f }
        , d_squared_ { 0.0f }
        {}
        RunningStats(const RunningStats& r)
        : count_     { r.count_     }
        , mean_      { r.mean_      }
        , d_squared_ { r.d_squared_ }
        {}

        void update_state(double price);
        double mean() { return mean_; }
        double d_squared() { return d_squared_; }
        double variance() { return count_ > 1 ? d_squared_/(count_ - 1) : 0; }
        double stdev() { return std::sqrt(variance()); }

    private:
        int count_;
        double mean_;
        double d_squared_;
    };
public:
	SimpleMeanReversionAlgo() = delete;
	SimpleMeanReversionAlgo(const std::string&, const std::string&, int = 50);
	~SimpleMeanReversionAlgo();

	void onTrade(const void *pSender, Array::Ptr &arg);
private:

    void update_state(double, const std::string&);

	int lookback_;

    std::map<std::string, RunningStats> symbol_stats_;

    Bitmex exchange_;

};