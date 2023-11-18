# Heisenberg Algo Trading

<img src="/assets/images/heisenberg_photo.jpg" style="width:500px"> 

## C++ Crypto Algos

### Build steps

Linux or MacOS. See [here](./windows.md) for Windows.

* Clone project
   ```bash
   git clone https://github.com/profitviews/heisenberg.git heisenberg
   cd heisenberg
   ```

* Install and Configure Conan and Ninja
   ```bash
   python3 -m venv .venv              # Create a Python virtual env
   source ./.venv/bin/activate        # Activate the virtual env
   pip install -r ./requirements.txt  # Install Conan and Ninja
   conan profile detect --force       # Generate a default configuration with the local machine settings
   conan config install ./.conan      # Install supported build profiles from ./.conan to ./conan2
   ```

* Standard Build  
   Installing Conan dependencies and configuring CMake presets.
   The sample build below is choosing the `Release` configuration:
   ```bash
   mkdir build
   conan install ./ -pr:h .conan2/profiles/gcc/12/x64-libstdc++11-release -pr:b .conan2/profiles/gcc/12/x64-libstdc++11-release -of ./build --build missing
   source build/conanbuild.sh
   cmake --preset conan-release
   cmake --build --preset conan-release
   ```
   
* Ninja Multi-Config Build

   Multi-config builds allow you to create a build folder containing sub-folders for different build configurations and build them side-by-side.
   To generate all the configurations we run the `conan-default` preset which configures CMake for these configurations `Release` and `Debug`. The sample build below is choosing the `Release` configuration:
   ```bash
   conan install ./ -pr:h .conan2/profiles/gcc/12/x64-libstdc++11-release -pr:b .conan2/profiles/gcc/12/x64-libstdc++11-release -of ./build --build missing -c tools.cmake.cmaketoolchain:generator="Ninja Multi-Config"
   source build/conanbuild.sh
   cmake --preset conan-default # The configure stage for multi-config builds is conan-default
   cmake --build --preset conan-release # The build stage for multi-config builds is the conan-<configuration>
   ```

This will create:

* `build/bin/algo` which will run a simple Mean Reversion algo on FTX or Coinbase
* For example
```bash
cd bin
./algo --exchange=coinbase --algo=SimpleMR --api_key=$COINBASE_API_KEY --api_secret=$COINBASE_API_SECRET --api_phrase=$COINBASE_API_PHRASE --lookback=50 --reversion_level=2 --base_quantity=0.0025 --symbol=ETH-BTC
```
