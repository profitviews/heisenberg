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
   
   Conan is configured for Ninja Multi-Config (`conanfile.py`). Configure once, then build Release and Debug:
   ```powershell
   conan install ./ -pr:h .conan2/profiles/msvc/194/x64-release -pr:b .conan2/profiles/msvc/194/x64-release --build missing -of ./build
   build\conanbuild.bat
   cmake --preset conan-default
   cmake --build build --config Release
   cmake --build build --config Debug
   ```

This will create:

* `heisenberg\build\bin\Release\algo.exe` which will run a simple Mean Reversion algo on FTX or Coinbase
* For example
```powershell
cd bin\Release
.\algo --exchange=coinbase --api_key=$Env:COINBASE_API_KEY --api_secret=$Env:COINBASE_API_SECRET --api_phrase=$Env:COINBASE_API_PHRASE --lookback=50 --reversion_level=2 --base_quantity=0.0025 --symbol=ETH-BTC
```

## C++ editing (clangd)

CMake emits **`compile_commands.json`** under **`build\`** for tooling. See **[docs/cpp-ide-cursor-vscode.md](../docs/cpp-ide-cursor-vscode.md)** for VS Code/Cursor, **clangd**, and **`.vscode/settings.json`**. The Linux **`--query-driver`** globs checked into that file normally **do not** match MSVC (**`cl.exe`**); clangd still infers headers from **`compile_commands.json`**. If your **`compile_commands.json`** lists MinGW/MSYS **`g++.exe`**, add a matching **`--query-driver`** glob (often in **User** settings).
