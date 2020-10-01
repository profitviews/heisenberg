#include "MarketDataAdapter.h"
#include "Poco/ErrorHandler.h"
#include "Poco/Logger.h"
#include "Poco/Net/NetException.h"
#include "src/include/SIOClient.h"

using Poco::ErrorHandler;
using Poco::Logger;
using Poco::Net::NetException;

MarketDataAdapter::MarketDataAdapter()
{
}

MarketDataAdapter::~MarketDataAdapter()
{
}

void MarketDataAdapter::onConnected(const void *pSender, Array::Ptr &arg)
{
	std::string msg = "";
	for (int i = 0; i < arg->size(); ++i)
		msg += arg->get(i).toString() + "";
	Logger::get("example").information("Notification Event: %s", msg);
}

void MarketDataAdapter::onTrade(const void *pSender, Array::Ptr &arg)
{
	std::string msg = "";
	for (int i = 0; i < arg->size(); ++i)
		msg += arg->get(i).toString() + "";
	Logger::get("example").information(msg);
}
