# C++ Crypto Algos

This is a set (currently of size 1!) of Cryptocurrency trading algos:
* `TalibMeanReversion`: a minimal Mean Reversion algo using the TA-Lib STDDEV implementation.

## Prerequisites

1. Compilers
   `sudo apt install build-essential`
   
1. Other software packages
   ```bash
   sudo apt install libssl-dev
   sudo apt install zlib1g-dev
   ```
1. TA-Lib
   Download the latest TA-Lib **source** tar ball from [ta-lib.org](https://www.ta-lib.org/hdr_dw.html) - currently that is `ta-lib-0.4.0-src.tar.gz`
   
   In some appropriate directory do
   
   ```bash
   tar xf ta-lib-0.4.0-src.tar.gz
   cd ta-lib
   ./configure
   make
   sudo make install
   ```
2. [`bitmex_executor`](https://github.com/profitviews/bitmex_executor) and [`profitview_socketio_cpp`](https://github.com/profitviews/profitview_socketio_cpp)

Extract and build [`bitmex_executor`](https://github.com/profitviews/bitmex_executor) and [`profitview_socketio_cpp`](https://github.com/profitviews/profitview_socketio_cpp) into the directory above `cpp_crypto_algos` (so that the 3 directories are peers).

The prerequisites of these two will satisfy the further prequisites of `cpp_crypto_algos`.

## Build

```bash
git clone https://github.com/profitviews/cpp_crypto_algos.git
cd cpp_crypto_algos/build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

This will create (with debugging symbols):

* `build/cpp_crypto_algos` which will run a simple Mean Reversion algo around prices streamed from https://markets.profitview.net
