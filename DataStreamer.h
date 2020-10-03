#include "MarketDataAdapter.h"

class DataStreamer : public MarketDataAdapter
{
public:
	DataStreamer();
	~DataStreamer();

	void onTrade(const void *pSender, Array::Ptr &arg);
    
};