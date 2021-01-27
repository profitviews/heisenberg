# **Socket.IO C++ for ProfitView**

This project has been forked from: https://github.com/isaelblais/socket.io-cpp where it was forked from https://github.com/himynameschris/socket.io-poco.
The originally forked version (of [Isaël Blais](https://github.com/isaelblais)) seemed to function in some contexts, but not with https://markets.profitview.net.

Some significant changes were required.

## Prerequisits ##

### Linux

1. Compilers
   `sudo apt install build-essential`
   
1. Other software
   `sudo apt install libssl-dev`
   `sudo apt install zlib1g-dev`

### Windows

**Some more work required**

With some work a manual build with Msys2 was possible, however there were runtime problems.  It is probably not difficult to complete this work.

1. Msys
   To build on Windows you need [Msys2](https://www.msys2.org/).  Follow the instructions there.

2. Msys will give you `pacman` package manager, `g++` and other essential programs.
   Use `pacman` to get:
   `cmake`
   `mingw-w64-x86_64-boost`
   `mingw-w64-x86_64-poco`

### Building Poco

   ```shell
   git clone -b master https://github.com/pocoproject/poco.git`
   cd poco
   make
   sudo make install
   ```
   
## Build

```bash
git clone https://github.com/profitviews/profitview_socketio_cpp.git
cd profitview_socketio_cpp/build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

This will create (with debugging symbols):
* `build/profitview_socketio_cpp` which will print streamed prices from https://markets.profitview.net
* `build/libprofitview_socketio.a` a static library containing classes allowing this streaming.
