# C++ Crypto Algos

This is a set of Cryptocurrency trading algos

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
2. Extract and build [`bitmex_executor`](https://github.com/profitviews/bitmex_executor) and [`profitview_socketio_cpp`](https://github.com/profitviews/profitview_socketio_cpp) into the directory above `cpp_crypto_algos` (so that the 3 directories are peers).
