#include "UserAdapter.h"
#include "Poco/ErrorHandler.h"
#include "Poco/Logger.h"
#include "Poco/Net/NetException.h"
#include "src/include/SIOClient.h"

using Poco::ErrorHandler;
using Poco::Logger;
using Poco::Net::NetException;

UserAdapter::UserAdapter()
{
}

UserAdapter::~UserAdapter()
{
}

void UserAdapter::onNotification(const void *pSender, Array::Ptr &arg)
{
	std::string msg = "";
	for (int i = 0; i < arg->size(); ++i)
		msg += arg->get(i).toString() + "";
	Logger::get("example").information("Notification Event: %s", msg);
}

void UserAdapter::onSendUserProfile(const void *pSender, Array::Ptr &arg)
{
	std::string msg = "";
	for (int i = 0; i < arg->size(); ++i)
		msg += arg->get(i).toString() + "";
	Logger::get("example").information("Send User profile Event: %s", msg);

	// Cast sender instance to real instance
	SIOClient *sioClientInstance = (SIOClient *)pSender;

	// Emit the register event with pc informations	// Emit information to server
	sioClientInstance->emit("receive-profile-info", "[{\"firstName\":\"myname\",\"lastName\":\"mylastname\"}]");
}

void UserAdapter::onMessage(const void *pSender, Array::Ptr &arg)
{
	std::string msg = "";
	for (int i = 0; i < arg->size(); ++i)
		msg += arg->get(i).toString() + "";
	Logger::get("example").information("onMessage: %s", msg);
}
