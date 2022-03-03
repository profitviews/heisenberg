#ifndef SIO_Client_INCLUDED
#define SIO_Client_INCLUDED

#include "SIOClientImpl.h"

#include "Poco/JSON/Array.h"

using Poco::JSON::Array;
using Poco::URI;

class SIOClient
{
private:
	~SIOClient();

	SIOClientImpl *_socket;

	std::string _uri;
	std::string _endpoint;

	Poco::NotificationCenter *_nCenter;

	SIOEventRegistry *_registry;
	SIONotificationHandler *_sioHandler;

public:
	SIOClient(std::string uri, std::string endpoint, SIOClientImpl *impl);

	static SIOClient *connect(std::string uri, const URI::QueryParameters& = {}, bool = true, int = -1);
	void disconnect();
	void send(std::string s);
	void emit(std::string eventname, std::string args);
	void emit(std::string eventname, Poco::JSON::Object::Ptr args);
	void emit(std::string eventname, Poco::JSON::Array::Ptr args);
	std::string getUri();
	Poco::NotificationCenter *getNCenter();

	std::string getSid() { return nullptr == _socket ? "" : _socket->getSid(); }

	typedef void (SIOEventTarget::*callback)(const void *, Array::Ptr &);

	void on(const char *name, SIOEventTarget *target, callback c);

	void fireEvent(const char *name, Array::Ptr args);
};

#endif
