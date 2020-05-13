#include "UserAdapter.h"
#include "Poco/ErrorHandler.h"
#include "Poco/Logger.h"
#include "Poco/Net/NetException.h"

using Poco::Net::NetException;
using Poco::ErrorHandler;
using Poco::Logger;


UserAdapter::UserAdapter()
{
}

UserAdapter::~UserAdapter()
{
}

void UserAdapter::onMessage(const void* pSender, Array::Ptr& arg)
{
	std::string msg = "";
	for (int i = 0; i < arg->size(); ++i)
		msg += arg->get(i).toString() + "";
	Logger::get("example").information("onMessage: %s", msg);
}

void UserAdapter::onEvent(const void* pSender, Array::Ptr & arg)
{
	std::string msg = "";
	for (int i = 0; i < arg->size(); ++i)
		msg += arg->get(i).toString() + " ";
	Logger::get("example").information("onEvent!");
}
