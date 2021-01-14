#pragma once
#include "MarketDataAdapter.h"
#include <Exchange.h>
#include <vector>
#include <map>

class SimpleMeanReversionAlgo : public MarketDataAdapter
{
public:
    class RunningStats
    {
    public:
        class NotPrepared {};

        RunningStats()
        : count_     { 0     }
        , mean_      { 0.0f  }
        , d_squared_ { 0.0f  }
        , prepared_  { false }
        {}
        RunningStats(const RunningStats& r)
        : count_     { r.count_     }
        , mean_      { r.mean_      }
        , d_squared_ { r.d_squared_ }
        , prepared_  { r.prepared_  }
        {}

        void prepare_state(double price);
        void update_state(double price);
        double mean() const { return mean_; }
        double d_squared() const { return d_squared_; }
        double variance() const;
        double stdev() const { return std::sqrt(variance()); }
        int count() const { return count_; }
        bool prepared() const { return prepared_; }

    private:
        int count_;
        int lookback_;
        double mean_;
        double d_squared_;
        bool prepared_;
    };
public:
	SimpleMeanReversionAlgo() = delete;
	SimpleMeanReversionAlgo(Exchange&, int = 50, double = 3.0f, int = 20);
	~SimpleMeanReversionAlgo();

	void onTrade(const void *pSender, Array::Ptr &arg);

    friend class RunningStats;
private:

    void update_state(double, const std::string&);

	int lookback_;

    double reversion_level_; // Multiple of stdev
    int base_quantity_;

    std::map<std::string, RunningStats> symbol_stats_;

    Exchange& exchange_;

};