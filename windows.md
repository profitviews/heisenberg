# Heisenberg - Windows

## Build steps

Download the latest version of [Visual Studio Community 2022](https://visualstudio.microsoft.com/vs/community/).

Start the Visual Studio 2022 Developer Command Prompt.

* Install and Configure Conan and Ninja
   ```powershell
   python -m venv .venv              # Create a Python virtual env
   .venv\Scripts\activate.bat        # Activate the virtual env
   pip install -r ./requirements.txt # Install Conan and Ninja
   conan profile detect --force      # Generate a default configuration with the local machine settings
   conan config install ./.conan     # Install supported build profiles from ./.conan to ./conan2
   ```
  
* Ninja Multi-Config Build
   
   Multi-config builds allow you to create a build folder containing sub-folders for different build configurations and build them side-by-side.
   To generate all the configurations we run the `conan-default` preset which configures CMake for these configurations `Release` and `Debug`. The sample build below is choosing the `Release` configuration:
   ```powershell
   conan install ./ -pr:h .conan2/profiles/msvc/193/x64-release -pr:b .conan2/profiles/msvc/193/x64-release --build missing -c tools.cmake.cmaketoolchain:generator="Ninja Multi-Config"
   build\generators\conanbuild.bat
   cmake --preset conan-default         # The configure stage for multi-config builds is conan-default
   cmake --build --preset conan-release # The build stage for multi-config builds is the conan-<configuration>
   ```

This will create:

* `heisenberg\build\bin\Release\algo.exe` which will run a simple Mean Reversion algo on FTX or Coinbase
* For example
```powershell
cd bin\Release
.\algo --exchange=coinbase --api_key=$Env:COINBASE_API_KEY --api_secret=$Env:COINBASE_API_SECRET --api_phrase=$Env:COINBASE_API_PHRASE --lookback=50 --reversion_level=2 --base_quantity=0.0025 --symbol=ETH-BTC
```
