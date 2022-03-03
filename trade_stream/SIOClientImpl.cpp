#include "include/SIOClientImpl.h"

#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPMessage.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/NetException.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/StreamCopier.h"
#include "Poco/Format.h"
#include <iostream>
#include <sstream>
#include <limits>
#include "Poco/StringTokenizer.h"
#include "Poco/String.h"
#include "Poco/Timer.h"
#include "Poco/RunnableAdapter.h"
#include "Poco/URI.h"

#include "include/SIONotifications.h"
#include "include/SIOClientRegistry.h"
#include "include/SIOClient.h"

using Poco::Dynamic::Var;
using Poco::JSON::Array;
using Poco::JSON::Object;
using Poco::JSON::ParseHandler;
using Poco::JSON::Parser;

using Poco::cat;
using Poco::StreamCopier;
using Poco::StringTokenizer;
using Poco::Timer;
using Poco::TimerCallback;
using Poco::UInt16;
using Poco::URI;
using Poco::Dynamic::Var;
using Poco::Net::Context;
using Poco::Net::HTTPClientSession;
using Poco::Net::HTTPMessage;
using Poco::Net::HTTPRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPSClientSession;
using Poco::Net::NetException;
using Poco::Net::SocketAddress;
using Poco::Net::WebSocket;

SIOClientImpl::SIOClientImpl() : _mask_payload{true}
{
	SIOClientImpl(URI("http://localhost:8080"));
}

SIOClientImpl::SIOClientImpl(
	const Poco::URI& uri,
	const URI::QueryParameters& handshake_parameters, 
	bool mask_payload, 
	int heartbeat_start_interval) 
: _host                    {uri.getHost()}
, _port                    {uri.getPort()}
, _uri                     {uri}
, _connected               {false}
, _version                 {SocketIOPacket::V2x}
, _session                 {nullptr}
, _ws                      {nullptr}
, _heartbeatTimer          {nullptr}
, _heartbeat_start_interval{heartbeat_start_interval}
, _logger                  {nullptr}
, _thread                  {}
, _refCount                {0}
, _buffer                  {nullptr}
, _buffer_size             {0}
, _handshake_parameters    {handshake_parameters}
, _mask_payload            {mask_payload}
{
}

SIOClientImpl::~SIOClientImpl(void)
{

	_thread.join();
	disconnect("");

	_ws->shutdown();
	delete (_ws);

	delete (_heartbeatTimer);
	delete (_session);
	if (_buffer)
	{
		delete[] _buffer;
		_buffer = NULL;
		_buffer_size = 0;
	}

	std::stringstream ss;
	ss << _uri.getHost() << ":" << _uri.getPort() << _uri.getPath();
	std::string uri = ss.str();
	SIOClientRegistry::instance()->removeSocket(uri);
}

bool SIOClientImpl::init()
{
	_logger = &(Logger::get("SIOClientLog"));

	if (handshake())
	{
		if (openSocket())
			return true;
	}

	return false;
}

namespace {
class HandshakeException { // Subclass from a runtime_error
};

std::pair<int, int> get_json_length(const std::string& handshake_response, int start = 0)
{
	if(handshake_response[start] != '\000') throw HandshakeException{};
	// Search for a FF
	auto length_end {handshake_response.find('\377', start + 1)};
	if(length_end == std::string::npos)
		throw HandshakeException{};
	
	std::string length_string;
	for (int i = start + 1; i < length_end; ++i)
		length_string.push_back(static_cast<char>('0' + handshake_response[i]));
	auto length {std::stoi(length_string)};
	return {length, length_end + 1};
}

std::pair<Object::Ptr, int> extract_sid(const std::string& handshake_response)
{
	auto [sid_length, sid_length_end] = get_json_length(handshake_response);

	if(handshake_response[sid_length_end] != '0') // '0' is "Open" packet type
		throw HandshakeException{};

	auto sid_string{handshake_response.substr(sid_length_end + 1, sid_length)};

	sid_string[sid_string.size() - 1] = '\377'; // add an "EOF" otherwise the parser errors out

	ParseHandler::Ptr pHandler = new ParseHandler(false);
	Parser parser(pHandler);
	Var sid_result = parser.parse(sid_string);
	Object::Ptr sid_msg = sid_result.extract<Object::Ptr>();
	return {sid_msg, sid_length_end + sid_length};
}

Array::Ptr extract_api_response(const std::string& handshake_response, int sid_length)
{
	auto [api_length, api_length_end] = get_json_length(handshake_response, sid_length);

	if(handshake_response[api_length_end] != '4' &&   // '4' is "Message" packet type
	   handshake_response[api_length_end + 1] != '2') // '2' is PING or "heartbeat" message type
		throw HandshakeException{};

	auto api_string{handshake_response.substr(api_length_end + 2, api_length - 1)};

	api_string[api_string.size() - 1] = '\377'; // add an "EOF" otherwise the parser errors out

	ParseHandler::Ptr pHandler = new ParseHandler(false);
	Parser parser(pHandler);
	Var api_result = parser.parse(api_string);
	Array::Ptr api_msg = api_result.extract<Array::Ptr>();
	return api_msg;
}

};

bool SIOClientImpl::handshake()
{
	UInt16 aport = _port;
	if (_uri.getScheme() == "https")
	{
		const Context::Ptr context(new Context(Context::CLIENT_USE, "", "", "", Context::VERIFY_NONE));
		_session = new HTTPSClientSession(_host, aport, context);
	}
	else
	{
		_session = new HTTPClientSession(_host, aport);
	}
	_session->setKeepAlive(false);
	
	// URI handshake_uri{"/socket.io/1/"}; // Not clear why the 1/ is added
	URI handshake_uri{"/socket.io/"};
	handshake_uri.addQueryParameter("EIO", "3");
	handshake_uri.addQueryParameter("transport", "polling");
	for(auto [name, value]: _handshake_parameters)
		handshake_uri.addQueryParameter(name, value);

	HTTPRequest req(HTTPRequest::HTTP_GET, handshake_uri.toString(), HTTPMessage::HTTP_1_1);
	req.set("Accept", "*/*");
	req.setContentType("text/plain");
	req.setHost(_host);

	_logger->information("Send Handshake Post request...:");
	HTTPResponse res;
	std::string handshake_response;

	try
	{
		_session->sendRequest(req);
		std::istream &rs = _session->receiveResponse(res);
		_logger->information("Receive Handshake Post request...");
		StreamCopier::copyToString(rs, handshake_response);
		if (res.getStatus() != Poco::Net::HTTPResponse::HTTP_OK)
		{
			_logger->error("%s %s", res.getStatus(), res.getReason());
			_logger->error("response: %s\n", handshake_response);
			return false;
		}
	}
	catch (std::exception &e)
	{
		return false;
	}

	_logger->information("%s %s", res.getStatus(), res.getReason());
	_logger->information("response: %s\n", handshake_response);

	_version = SocketIOPacket::V2x;

	auto [sid_message, sid_length] = extract_sid(handshake_response);

	_logger->information("session: %s", sid_message->get("sid").toString());
	_logger->information("heartbeat: %s", sid_message->get("pingInterval").toString());
	_logger->information("timeout: %s", sid_message->get("pingTimeout").toString());

	_sid = sid_message->get("sid").toString();
	_heartbeat_timeout = atoi(sid_message->get("pingInterval").toString().c_str()) / 1000;
	_timeout = atoi(sid_message->get("pingTimeout").toString().c_str()) / 1000;

	auto api_message = extract_api_response(handshake_response, sid_length);

	return api_message->get(1).extract<Object::Ptr>()->get("success").toString() == "true";
}

bool SIOClientImpl::openSocket()
{
	UInt16 aport = _port;
	HTTPResponse res;
	HTTPRequest req;
	req.setMethod(HTTPRequest::HTTP_GET);
	req.setVersion(HTTPMessage::HTTP_1_1);
	switch (_version)
	{
	case SocketIOPacket::V2x:
	{
		req.setURI("/socket.io/?EIO=3&transport=websocket&sid=" + _sid);
	}
	break;
	}

	_logger->information("WebSocket To Create for %s", _sid);
	Poco::Timestamp now;
	now.update();
	do
	{
		try
		{
			_ws = new WebSocket(*_session, req, res);
		}
		catch (NetException &ne)
		{

			_logger->warning("Exception when creating websocket %s : %s - %s", ne.displayText(), ne.code(), ne.what());
			if (_ws)
			{
				delete _ws;
				_ws = NULL;
			}
			Poco::Thread::sleep(100);
		}
	} while (_ws == NULL && now.elapsed() < 1000000);
	if (_ws == NULL)
	{
		_logger->error("Impossible to create websocket");
		return _connected;
	}

	if (_version == SocketIOPacket::V2x)
	{
		// std::string s = "5"; //That's a ping https://github.com/Automattic/engine.io-parser/blob/1b8e077b2218f4947a69f5ad18be2a512ed54e93/lib/index.js#L21
		std::string s = "2probe"; // I believe that a PING should be "2".
								  // In the successful Python version, "probe" is also sent
		_ws->sendFrame(s.data(), s.size());
		_ws->sendFrame("5", 1);   // "5" is upgrade
	}

	_logger->information("WebSocket Created and initialised");

	_connected = true; //FIXME on 1.0.x the server acknowledge the connection

	auto convert_timeout { [](int timeout) -> int { return static_cast<int>(timeout * 750); } };

	int hbInterval = convert_timeout(this->_heartbeat_timeout);
	_heartbeatTimer = new Timer(
		_heartbeat_start_interval >= 0 ? convert_timeout(_heartbeat_start_interval) : hbInterval, hbInterval);
	TimerCallback<SIOClientImpl> heartbeat(*this, &SIOClientImpl::heartbeat);
	_heartbeatTimer->start(heartbeat);

	_thread.start(*this);

	return _connected;
}

SIOClientImpl *SIOClientImpl::connect(
	URI uri, 
	const URI::QueryParameters& hanshakeParameters, 
	bool mask_payload, 
	int heartbeat_start_interval)
{
	SIOClientImpl *s = new SIOClientImpl(uri, hanshakeParameters, mask_payload, heartbeat_start_interval);

	if (s && s->init())
	{
		return s;
	}

	return NULL;
}

void SIOClientImpl::disconnect(std::string endpoint)
{
	std::string s;
	if (_version == SocketIOPacket::V2x)
		s = "41" + endpoint;
	_ws->sendFrame(s.data(), s.size());
	if (endpoint == "")
	{
		_logger->information("Disconnect");
		_heartbeatTimer->stop();
		_connected = false;
	}

	if (_version == SocketIOPacket::V2x)
		_ws->shutdown();
}

void SIOClientImpl::connectToEndpoint(std::string endpoint)
{
	_logger->information("heartbeat called");
	SocketIOPacket *packet = SocketIOPacket::createPacketWithType("connect", _version);
	packet->setEndpoint(endpoint);
	this->send(packet);
}

void SIOClientImpl::heartbeat(Poco::Timer &timer)
{
	_logger->information("heartbeat called");
	SocketIOPacket *packet = SocketIOPacket::createPacketWithType("heartbeat", _version);
	this->send(packet);
}

void SIOClientImpl::run()
{

	monitor();
}

void SIOClientImpl::monitor()
{
	do
	{
		receive();
	} while (_connected);
}

void SIOClientImpl::send(std::string endpoint, std::string s)
{
	switch (_version)
	{
	case SocketIOPacket::V2x:
		this->emit(endpoint, "message", s);
		break;
	}
}

void SIOClientImpl::emit(std::string endpoint, std::string eventname, Poco::JSON::Object::Ptr args)
{
	_logger->information("Emitting event \"%s\"", eventname);
	SocketIOPacket *packet = SocketIOPacket::createPacketWithType("event", _version);
	packet->setEndpoint(endpoint);
	packet->setEvent(eventname);
	packet->addData(args);
	this->send(packet);

} 

void SIOClientImpl::emit(std::string endpoint, std::string eventname, Poco::JSON::Array::Ptr args)
{
	_logger->information("Emitting event \"%s\"", eventname);
	SocketIOPacket *packet = SocketIOPacket::createPacketWithType("event", _version);
	packet->setEndpoint(endpoint);
	packet->setEvent(eventname);
	packet->addData(args);
	this->send(packet);
}

void SIOClientImpl::emit(std::string endpoint, std::string eventname, std::string args)
{
	_logger->information("Emitting event \"%s\"", eventname);
	SocketIOPacket *packet = SocketIOPacket::createPacketWithType("event", _version);
	packet->setEndpoint(endpoint);
	packet->setEvent(eventname);
	packet->addData(args);
	this->send(packet);
}

void SIOClientImpl::send(SocketIOPacket *packet)
{
	std::string req = packet->toString();
	if (_connected)
	{
		_logger->information("-->SEND:%s", req);
		_ws->sendFrame(req.data(), req.size());
	}
	else
		_logger->warning("Cant send the message (%s) because disconnected", req);
}

bool SIOClientImpl::receive()
{
	if (!_buffer)
	{
		int rcv_size = _ws->getReceiveBufferSize();
		_buffer = new char[rcv_size];
		_buffer_size = rcv_size;
	}
	int flags;
	int n;

	n = _ws->receiveFrame(_buffer, _buffer_size, flags);
	_logger->information("I received something...bytes received: %d ", n);

	SocketIOPacket *packetOut;

	std::stringstream s;
	for (int i = 0; i < n; i++)
	{
		s << _buffer[i];
	}
	SIOClient *c;
	std::stringstream suri;
	suri << _uri.getHost() << ":" << _uri.getPort() << _uri.getPath();
	std::string uri = suri.str();

	switch (_version)
	{
	case SocketIOPacket::V2x:
	{
		const char first = s.str().at(0);
		std::string data = s.str().substr(1);
		// int control = atoi(&first); RH: this gives "42" as 42 not 4 because the address of the first 
		//                                 character is a char array for the whole string.  We just want
		//                                 the first character
		int control = static_cast<int>(first - '0');
		_logger->information("Buffer received: [%s]\tControl code: [%i]", s.str(), control);
		switch (control)
		{
		case 0:
			_logger->information("Not supposed to receive control 0 for websocket");
			_logger->warning("That's not good");
			break;
		case 1:
			_logger->information("Not supposed to receive control 1 for websocket");
			break;
		case 2:
			_logger->information("Ping received, send pong");
			data = "3" + data;
			_ws->sendFrame(data.c_str(), data.size());
			break;
		case 3:
			_logger->information("Pong received");
			if (data == "probe")
			{
				_logger->information("Request Update");
				_ws->sendFrame("5", 1);
			}
			break;
		case 4:
		{
			packetOut = SocketIOPacket::createPacketWithType("event", _version);
			const char second = data.at(0);
			data = data.substr(1);
			int nendpoint = data.find("[");
			std::string endpoint = "";
			if (nendpoint != std::string::npos)
			{
				endpoint += data.substr(0, nendpoint);
				data = data.substr(nendpoint);
				//uri+=endpoint;
			}
			packetOut->setEndpoint(endpoint);
			c = SIOClientRegistry::instance()->getClient(uri);

			// control = atoi(&second);  // RH: As above for first
			control = static_cast<int>(second - '0');
			_logger->information("Message code: [%i]", control);
			switch (control)
			{
			case 0:
				_logger->information("Socket Connected");
				_connected = true;
				break;
			case 1:
				_logger->information("Socket Disconnected");
				//this->disconnect("/");//FIXME the server is telling us it is disconnecting
				break;
			case 2:
			{
				_logger->information("Event Dispatched (%s)", data);
				ParseHandler::Ptr pHandler = new ParseHandler(false);
				Parser parser(pHandler);
				Var result = parser.parse(data);
				Array::Ptr msg = result.extract<Array::Ptr>();
				packetOut->setEvent(msg->get(0));
				for (int i = 1; i < msg->size(); ++i)
					packetOut->addData(msg->get(i).toString());

				c = SIOClientRegistry::instance()->getClient(uri);
				c->getNCenter()->postNotification(new SIOEvent(c, packetOut));
			}
			break;
			case 3:
				_logger->information("Message Ack");
				break;
			case 4:
				_logger->information("Error");
				break;
			case 5:
				_logger->information("Binary Event");
				break;
			case 6:
				_logger->information("Binary Ack");
				break;
			}
		}
		break;
		case 5:
			_logger->information("Upgrade required");
			break;
		case 6:
			_logger->information("Noop");
			break;
		}
	}
	break;
	}

	return true;
}

void SIOClientImpl::addref()
{
	_refCount++;
}

void SIOClientImpl::release()
{
	if (--_refCount == 0)
		delete this;
}
