# **Socket.IO C++ for ProfitView**

This project has been forked from: [isaelblais/socket.io-cpp](https://github.com/isaelblais/socket.io-cpp) where it was forked from [himynameschris/socket.io-poco](https://github.com/himynameschris/socket.io-poco).
The forked version (of [Isaël Blais](https://github.com/isaelblais)) seemed to function in some contexts, but not with https://markets.profitview.net.

Some significant changes were required to get it working.  It now appears robust.

## Prerequisites

### Linux

1. Compilers
   `sudo apt install build-essential`
   
1. Other software packages
   ```bash
   sudo apt install libssl-dev
   sudo apt install zlib1g-dev
   sudo apt-get install libiodbc2 libiodbc2-dev
   sudo snap install --classic cmake
   ```

### Building Poco

   ```shell
   git clone -b master https://github.com/pocoproject/poco.git
   cd poco
   ./configure --omit=Data/SQLite,Data/ODBC,Data/MySQL,Data/PostgreSQL
   make -k
   sudo make install
   ```
   **Note: the Poco build may take tens of minutes**
   
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
