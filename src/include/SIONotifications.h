#pragma once
#include "Poco/Notification.h"
#include "SIOPacket.h"

using Poco::Notification;

class SIOClient;

class SIOEvent : public Notification
{
public:
	SIOEvent(SIOClient *client, SocketIOPacket *data) : client(client), data(data) {}

	SIOClient *client;
	SocketIOPacket *data;

protected:
	~SIOEvent() { delete data; };
};
