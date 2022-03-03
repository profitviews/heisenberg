#ifndef SIO_ClientImpl_DEFINED
#define SIO_ClientImpl_DEFINED

#include <string>

#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Logger.h"
#include "Poco/Timer.h"
#include "Poco/NotificationCenter.h"
#include "Poco/Thread.h"
#include "Poco/ThreadTarget.h"
#include "Poco/RunnableAdapter.h"
#include "Poco/URI.h"

#include "Poco/JSON/Parser.h"

#include "SIONotificationHandler.h"
#include "SIOEventRegistry.h"
#include "SIOEventTarget.h"
#include "SIOPacket.h"

using Poco::Logger;
using Poco::NotificationCenter;
using Poco::Thread;
using Poco::ThreadTarget;
using Poco::Timer;
using Poco::TimerCallback;
using Poco::Net::HTTPClientSession;
using Poco::Net::WebSocket;
using Poco::URI;

class SIOClientImpl : public Poco::Runnable
{
public:
	bool handshake();
	bool openSocket();
	bool init();

	void release();
	void addref();

	static SIOClientImpl *connect(
		Poco::URI uri, 
		const URI::QueryParameters& = {}, 
		bool = true,
		int = -1);
	void disconnect(std::string endpoint);
	void connectToEndpoint(std::string endpoint);
	void monitor();
	virtual void run();
	void heartbeat(Poco::Timer &timer);
	bool receive();
	void send(std::string endpoint, std::string s);
	void send(SocketIOPacket *packet);
	void emit(std::string endpoint, std::string eventname, std::string args);
	void emit(std::string endpoint, std::string eventname, Poco::JSON::Object::Ptr args);
	void emit(std::string endpoint, std::string eventname, Poco::JSON::Array::Ptr args);

	void setHandshakeParameters(const URI::QueryParameters& handshakeParameters) {
		_handshake_parameters = handshakeParameters;
	}

	std::string getUri();

	std::string getSid() { return _sid; }

private:
	SIOClientImpl();
	SIOClientImpl(
		const Poco::URI& uri, 
		const URI::QueryParameters& = {}, 
		bool = true,
		int = -1);
	~SIOClientImpl(void);

	std::string _sid;
	int _heartbeat_timeout;
	int _timeout;
	std::string _host;
	int _port;
	Poco::URI _uri;
	bool _connected;
	SocketIOPacket::SocketIOVersion _version;

	HTTPClientSession *_session;
	WebSocket *_ws;
	Timer *_heartbeatTimer;
	int _heartbeat_start_interval;
	Logger *_logger;
	Thread _thread;

	int _refCount;
	char *_buffer;
	std::size_t _buffer_size;

	// For authentication data or similar - such as an API key
	URI::QueryParameters _handshake_parameters;

	// Some implementations don't assume a masked payload - such as socket.io
	bool _mask_payload;
};

#endif
