# C++ Crypto Algos

## Build steps

Linux or MacOS.  See [here](https://github.com/profitviews/cpp_crypto_algos/blob/896f7bf3a1c54c4f2597a2abfd4de70bb8ed32e1/windows.md) for Windows.

1. Clone project
   ```
   git clone https://github.com/profitviews/cpp_crypto_algos.git cpp_crypto_algos
   cd cpp_crypto_algos
   ```

2. Install Conan
   ```
   python3 -m venv .venv         # Create a Python virtual env
   source ./.venv/bin/activate   # Activate the virtual env
   pip install conan             # Install conan
   ```

3. Install Conan Package & Configure CMake 
   ```
   mkdir build
   cd build
   conan install ../ --build missing
   source ./activate.sh
   cmake -DCMAKE_BUILD_TYPE=Debug ..
   ```
   
4. Build
   ```bash
   cmake --build .
   ```

This will create (with debugging symbols):

* `build/cpp_crypto_algos` which will run a simple Mean Reversion algo on FTX or Coinbase
* For example
```bash
cd bin
./algo --exchange=coinbase --api_key=$COINBASE_API_KEY --api_secret=$COINBASE_API_SECRET --api_phrase=$COINBASE_API_PHRASE --lookback=50 --reversion_level=2 --base_quantity=0.0025 --symbol=ETH-BTC
```

