#pragma once
#include "src/include/SIOEventTarget.h"
#include "src/include/SIOClient.h"

class MarketDataAdapter : public SIOEventTarget
{
public:
	MarketDataAdapter();
	~MarketDataAdapter();

	void onTrade(const void *pSender, Array::Ptr &arg);
	void onConnected(const void *pSender, Array::Ptr &arg);
};