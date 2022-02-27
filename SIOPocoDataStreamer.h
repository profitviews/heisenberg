#pragma once

#include <SIOEventTarget.h>

class SIOPocoDataStreamer : public SIOEventTarget
{
public:
	SIOPocoDataStreamer();
	~SIOPocoDataStreamer();

	void onTrade(const void *pSender, Array::Ptr &arg);
    
};