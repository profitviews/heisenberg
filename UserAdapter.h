#pragma once
#include "src/include/SIOEventTarget.h"

class UserAdapter : public SIOEventTarget
{
public:
	UserAdapter();
	~UserAdapter();

	void onMessage(const void* pSender, Array::Ptr& arg);
	void onEvent(const void* pSender, Array::Ptr& arg);
};

