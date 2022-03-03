# C++ Crypto Algos

## Prerequisites

1. Compilers
   `sudo apt install build-essential`
   
1. Other software packages
   ```bash
   sudo apt install libssl-dev
   sudo apt install zlib1g-dev
   ```
   *There are a few more now - to be added*
   
## Build

```bash
git clone https://github.com/profitviews/cpp_crypto_algos.git
cd cpp_crypto_algos/build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

This will create (with debugging symbols):

* `build/cpp_crypto_algos` which will run a simple Mean Reversion algo on FTX or Coinbase
