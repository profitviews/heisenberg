// main.cpp : Defines the entry point for the console application.
#ifdef _WIN64
   //define something for Windows (64-bit)
	#include "Poco/WindowsConsoleChannel.h"
#elif _WIN32
   //define something for Windows (32-bit)
	#include "Poco/WindowsConsoleChannel.h"
#elif __APPLE__
    #if TARGET_IPHONE_SIMULATOR
         // iOS Simulator
    #elif TARGET_OS_IPHONE
        // iOS device
    #elif TARGET_OS_MAC
        // Other kinds of Mac OS
    #else
        // Unsupported platform
    #endif
#elif __linux
    // linux
	#include "Poco/ConsoleChannel.h"
#elif __unix // all unices not caught above
    // Unix
	#include "Poco/ConsoleChannel.h"
#elif __posix
    // POSIX
	#include "Poco/ConsoleChannel.h"
#endif


#include "Poco/Thread.h"

#include "src/include/SIOClient.h"
#include "UserAdapter.h"

#include <iostream>

using Poco::Thread;

int main(int argc, char* argv[])
{
	//create a c++ Poco logger to use and set its channel to the windows console
	//this is the same logger instance that the library will hook into
	Logger *logger = &(Logger::get("example"));

#ifdef _WIN64
   //define something for Windows (64-bit)
	logger->setChannel(new Poco::WindowsConsoleChannel());
#elif _WIN32
   //define something for Windows (32-bit)
	logger->setChannel(new Poco::WindowsConsoleChannel());
#elif __APPLE__
    #if TARGET_IPHONE_SIMULATOR
         // iOS Simulator
    #elif TARGET_OS_IPHONE
        // iOS device
    #elif TARGET_OS_MAC
        // Other kinds of Mac OS
    #else
        // Unsupported platform
    #endif
#elif __linux
    // linux
	logger->setChannel(new Poco::ConsoleChannel());
#elif __unix // all unices not caught above
    // Unix
	logger->setChannel(new Poco::ConsoleChannel());
#elif __posix
    // POSIX
	logger->setChannel(new Poco::ConsoleChannel());
#endif

	// Declaring an adapter for the client
	UserAdapter* userAdapter = new UserAdapter();

	//Establish the socket.io connection
	//JS: var socket = io.connect("localhost:3000")
	//SIOClient *sio = SIOClient::connect("http://localhost:3000");

	// Establish the socket.io connection to an endpoint
	SIOClient* sioUserClient = SIOClient::connect("http://localhost:3000/user");

	//Create a target and register object its method onUpdate for the Update event
	//JS: socket.on("Update", function(data) {...});

	sioUserClient->on("message", userAdapter, callback(&UserAdapter::onMessage));
	sioUserClient->on("notification", userAdapter, callback(&UserAdapter::onNotification));
	sioUserClient->on("send-user-profile", userAdapter, callback(&UserAdapter::onSendUserProfile));
	
	// Send data to server
	sioUserClient->send("Send to server");
	// Emit data to server
	sioUserClient->emit("Emit to server", "data");

	//setup is now complete, messages and events can be send and received
	logger->information("Socket.io client setup complete\n");

	//wait for user input to move to next section of code
	//socket receiving occurs in another thread and will not be halted
	logger->information("Press ENTER to continue...");
	std::cin.get();

	//disconnecting the default socket with no endpoint will also disconnect all endpoints
	sioUserClient->disconnect();

	logger->information("Press ENTER to quit...");
	std::cin.get();

	return 0;
}

