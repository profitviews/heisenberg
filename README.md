# **Socket.IO CPP with Poco libraries** #
*A C++ Socket.IO Client using the libraries at [pocoproject.org](http://pocoproject.org/)*

# **#info** #

This project has been taken from: https://github.com/himynameschris/socket.io-poco

#### changes:

- Removed cmake files
- Removed everything related to v09x (old version)
- Replaced version of v10x for v2x which represent actual version of socket.io
- Add better demo with endpoint

#### fixes:

- Fix emit to endpoint 
- Fix send to endpoint
- Fix receive from endpoint
- Null ptr init

#### todos:

- Add socket io client init settings
- Add reconnection attempts setting 
- Provide instructions to build on linux & mac with g++

# **#include** #

## Dependencies: ##

- POCO C++ Foundation, Net, NetSSL and JSON libraries
	- Download at: https://pocoproject.org/download.html
- OpenSSL
	- this is required by the Poco NetSSL library and HTTPS support 

# **#building** #

## Windows: ##

#### Building Poco with vcpkg
```
vcpkg install POCO
```
ref: https://pocoproject.org/docs/00200-GettingStarted.html

#### Building socket.io-cpp with visual studio

Add ref. of Poco include files in "Include Directories" of the project

```
<path_to_vcpkg_folder>\vcpkg\packages\poco_x86-windows\include
<path_to_vcpkg_folder>\vcpkg\packages\openssl-windows_x86-windows\include
```

Add ref. of Poco libraries files in "Library Directories" of the project

```
<path_to_vcpkg_folder>\vcpkg\packages\poco_x86-windows\lib
<path_to_vcpkg_folder>\vcpkg\packages\openssl-windows_x86-windows\lib
```

## Linux & Mac: ##

#### Building Poco
```
./configure
make
```
ref: https://pocoproject.org/docs/00200-GettingStarted.html


#### Building socket.io-cpp
```
Comming soon...
```

# **#using** #

**To create a client object and connect:**

`SIOClient *sio = SIOClient::connect("http://localhost:3000");`

**To use messaging:**

Simply use the send method and pass the message string

`sio->send("Hello Socket.IO");`

**To use events:**

1) Create your own subclass of SIOEventTarget 

2) Create methods to handle the callbacks, an event callback function signature must match this pattern:

`void MyClass::onUpdate(const void* pSender, Object::Ptr& arg)`

3) Event callbacks can then be registered through the "on" method, similar to the javascript API, providing a reference to the target object and the function address of the callback method wrapped in the "callback()" typedef like this:

`sio->on("Update", *pMyClass, callback(&MyClass::onUpdate));`

Note: callback() is a custom typedef used by the socket.io-poco library, defined as:

`typedef void (SIOEventTarget::*callback)(const void*, Object::Ptr&);`

4) Lastly, to fire an event use the emit method, passing the event name and data both as strings:

`testpoint->emit("testevent", "[{\"name\":\"myname\",\"type\":\"mytype\"}]");`

**To use endpoints, AKA namespaces:**

To connect to the endpoint 'testpoint':

`SIOClient *testpoint = SIOClient::connect("http://localhost:3000/testpoint");`

This will first check for a socket already connected to the base URI localhost:3000. If it exists, it will be used and the namespace will be connected. If it does not exist, the connection to the base URI will first be established and then the connection to the endpoint will be established.

# **#license** #

MIT License

Copyright (c) 2013 Chris Hannon / channon.us

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
