#include "include/SIONotificationHandler.h"
#include "Poco/Observer.h"
#include "include/SIONotifications.h"
#include "Poco/NotificationCenter.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/ParseHandler.h"

#include "include/SIOEventRegistry.h"
#include "include/SIOClient.h"

using Poco::Observer;
using Poco::Dynamic::Var;
using Poco::JSON::Array;
using Poco::JSON::Object;
using Poco::JSON::ParseHandler;
using Poco::JSON::Parser;

SIONotificationHandler::SIONotificationHandler(void)
{
}

SIONotificationHandler::SIONotificationHandler(NotificationCenter *nc)
{
	_nCenter = nc;
	registerCallbacks(_nCenter);

	_logger = &(Logger::get("SIOClientLog"));
}

SIONotificationHandler::~SIONotificationHandler(void)
{
	_nCenter->removeObserver(
		Observer<SIONotificationHandler, SIOEvent>(*this, &SIONotificationHandler::handleEvent));
}

void SIONotificationHandler::handleEvent(SIOEvent *pNf)
{
	_logger->information("handling Event");
	_logger->information("data: %s", pNf->data->toString());

	Poco::JSON::Array::Ptr arr = new Poco::JSON::Array(pNf->data->getDatas());
	pNf->client->fireEvent(pNf->data->getEvent().c_str(), arr);
	pNf->release();
}

void SIONotificationHandler::registerCallbacks(NotificationCenter *nc)
{
	_nCenter = nc;
	_nCenter->addObserver(
		Observer<SIONotificationHandler, SIOEvent>(*this, &SIONotificationHandler::handleEvent));
}

void SIONotificationHandler::setNCenter(NotificationCenter *nc)
{
	_nCenter = nc;
	registerCallbacks(_nCenter);
}
