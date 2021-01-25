#pragma once
#include "src/include/SIOEventTarget.h"
#include "src/include/SIOClient.h"

#include <vector>

class MarketDataAdapter : public SIOEventTarget
{
public:
	MarketDataAdapter();
	virtual ~MarketDataAdapter();

	virtual void onTrade(const void *pSender, Array::Ptr &arg) = 0;
};