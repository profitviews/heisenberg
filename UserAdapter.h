#pragma once
#include "src/include/SIOEventTarget.h"

class UserAdapter : public SIOEventTarget
{
public:
	UserAdapter();
	~UserAdapter();

	void onNotification(const void *pSender, Array::Ptr &arg);
	void onSendUserProfile(const void *pSender, Array::Ptr &arg);
	void onMessage(const void *pSender, Array::Ptr &arg);
};