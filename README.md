# **Socket.IO C++ for ProfitView**

# **#info** #

This project has been taken from: https://github.com/himynameschris/socket.io-poco

## Dependencies: ##

- POCO C++ Foundation, Net, NetSSL and JSON libraries
	- Download at: https://pocoproject.org/download.html
- OpenSSL
	- this is required by the Poco NetSSL library and HTTPS support 

## Windows: ##

**Some more work required**

1. Msys
   To build on Windows you need [Msys2](https://www.msys2.org/).  Follow the instructions there.

2. Msys will give you `pacman` package manager, `g++` and other essential programs.
   Use `pacman` to get:
   `cmake`
   `mingw-w64-x86_64-boost`
   `mingw-w64-x86_64-poco`

## Linux & Mac: ##

### Building Poco

   ```shell
   git clone -b master https://github.com/pocoproject/poco.git`
   cd poco
   make
   sudo make install

## Windows

3. Download the latest TA-Lib **source** tar ball from [ta-lib.org](https://www.ta-lib.org/hdr_dw.html) - currently that is `ta-lib-0.4.0-src.tar.gz`
   In some appropriate directory do
   ```bash
   tar xf ta-lib-0.4.0-src.tar.gz 
   cd ta-lib/
   ./configure
   make
   make install
   
## Linux

1. Compilers
   `sudo apt install build-essential`
   
1. Other software
   `sudo apt install libssl-dev`
   `sudo apt install zlib1g-dev`
