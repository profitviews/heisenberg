#pragma once

#include <SIOEventTarget.h>
#include <SIOClient.h>

class MarketDataAdapter : public SIOEventTarget
{
public:
	MarketDataAdapter();
	virtual ~MarketDataAdapter();

	virtual void onTrade(const void *pSender, Array::Ptr &arg) = 0;
};