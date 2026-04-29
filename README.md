# Heisenberg Algo Trading

<img src="/assets/images/heisenberg_photo.jpg" style="width:500px"> 

## C++ Crypto Algos

### Build steps

Linux or macOS. See [windows.md](./windows.md) for Windows.

The Conan recipe sets `Ninja Multi-Config` as the CMake generator, so one configure step produces a multi-config tree where both `Release` and `Debug` are valid (`CMakeLists.txt` limits configurations to those two). Pick Conan profiles that match how you want dependencies built (`build_type` is usually `Release` or `Debug` in the profile paths below).

#### Clone the project

```bash
git clone https://github.com/profitviews/heisenberg.git heisenberg
cd heisenberg
```

#### Install and configure Conan and Ninja

```bash
python3 -m venv .venv              # Create a Python virtual env
source ./.venv/bin/activate        # Activate the virtual env
pip install -r ./requirements.txt  # Install Conan and Ninja
conan profile detect --force         # Generate a default configuration with the local machine settings
conan config install ./.conan       # Merge repo Conan config into your Conan home (see docs)
```

#### Configure (GCC example)

Adjust `.conan/profiles/gcc/12/...` if you use Clang or another toolchain.

```bash
mkdir -p build
conan install ./ \
  -pr:h .conan/profiles/gcc/12/x64-libstdc++11-release \
  -pr:b .conan/profiles/gcc/12/x64-libstdc++11-release \
  -of ./build --build missing
conan install ./ \
  -pr:h .conan/profiles/gcc/12/x64-libstdc++11-debug \
  -pr:b .conan/profiles/gcc/12/x64-libstdc++11-debug \
  -of ./build --build missing
source build/conanbuild.sh
cmake --preset conan-default
```

Use the **release** profile command alone if you only build **Release**. For **Ninja Multi-Config**, Conan must generate CMakeDeps for **both** `Release` and `Debug` (two installs into the same `-of ./build`) or **`cmake --build build --config Debug`** will fail with missing Boost headers and similar errors—the compiler command will lack Conan `-isystem` paths.

Do not pass **`-DCMAKE_TOOLCHAIN_FILE=…`** together with **`cmake --preset conan-default`**. Conan’s preset already sets **`toolchainFile`** (`build/conan_toolchain.cmake`); adding **`-D`** makes CMake warn that **`CMAKE_TOOLCHAIN_FILE`** was unused. Configure with **`source build/conanbuild.sh && cmake --preset conan-default`** only.

Do not pass `-c tools.cmake.cmaketoolchain:generator=...` manually; it is already set in `conanfile.py`.

#### Build Release and Debug

From the repo root, after configure:

```bash
cmake --build build --config Release --parallel
cmake --build build --config Debug --parallel
```

Or:

```bash
./scripts/build-release-debug.sh
```

`BUILD_DIR` defaults to `./build`; override if needed: `BUILD_DIR=/path/to/build ./scripts/build-release-debug.sh`.

If you change Conan inputs (profiles, `conanfile.py`), re-run **`conan install`** (both release and debug installs when you build both configs), then **`cmake --preset conan-default`** again.

#### Unit tests (Catch2)

With **`cmake --preset conan-default`** applied and **`BUILD_TESTING`** left at its default (**ON**), build and run tests from the repo root:

```bash
cmake --build build --config Debug --target heisenberg_tests --parallel
cd build && ctest --output-on-failure -C Debug
```

For **Release**, use **`--config Release`** in both commands. With Ninja Multi-Config as configured here, the executable is **`build/tests/Debug/heisenberg_tests`** or **`build/tests/Release/heisenberg_tests`**.

Configure with **`BUILD_TESTING=OFF`** if you need to omit the test target (smaller configure when FetchContent for Catch2 is undesirable).

**Sandbox executor:** [`SandboxOrderExecutor`](src/sandbox_order_executor.hpp) records [`OrderExecutor::new_order`](src/order_executor.hpp) calls in memory (no ccapi/network). Coverage lives under **`[e2e][sandbox]`** in **`heisenberg_tests`**. To smoke the CLI without API keys:

```bash
cmake --build build --config Debug --target execute_order --parallel
./build/bin/Debug/execute_order --sandbox --symbol BTC-USD --side buy --size 1 --type limit --price 100
```

#### C++ IDE notes (Cursor vs VS Code)

IntelliSense and bundled extensions differ between **Cursor** and **VS Code**. This repo includes **`.vscode/settings.json`** with **`clangd.arguments`** (**`--compile-commands-dir`** toward **`build/compile_commands.json`**, **`--background-index`**, and on Linux **`--query-driver`** so **clangd** matches **GCC/libstdc++** paths used by **`/usr/bin/c++`** in **`compile_commands.json`**). See **[docs/cpp-ide-cursor-vscode.md](./docs/cpp-ide-cursor-vscode.md)**.

#### Debugging in Cursor / VS Code (Linux)

1. Configure Conan once (`conan install` … `cmake --preset conan-default`), then build **Debug**: Run Build Task (`Ctrl+Shift+B`) → **Build Debug (Conan)**, or `cmake --build build --config Debug`.
2. For **breakpoints / GDB**, `.vscode/launch.json` uses **`cppdbg`** — install whatever provides it in your editor (VS Code usually uses Microsoft **C/C++** for the debugger adapter; Cursor users should follow [docs/cpp-ide-cursor-vscode.md](./docs/cpp-ide-cursor-vscode.md) for IntelliSense vs debugging).
3. **Run and Debug** (`Ctrl+Shift+D`), pick a **`(gdb) …`** configuration, then **F5**. Use the **Debug** CMake configuration (`-g`). Optional: `"preLaunchTask": "Build Debug (Conan)"` on a launch config.

#### Artifacts and example

Executables end up under `build/bin/` (sometimes with `Release` or `Debug` subdirectories, depending on platform and generator).

**Exchanges.** `--exchange` accepts short names that map to [ccapi](https://github.com/crypto-chronic/cryptochronic-api) exchange identifiers: `coinbase`, `ftx`, `bitmex`, `kraken` (Kraken **spot**). You can also pass the ccapi name directly (e.g. `kraken`). Credentials should match the strings ccapi expects (e.g. `KRAKEN_API_KEY`, `KRAKEN_API_SECRET` for Kraken). Symbol strings must match each venue—Kraken pair names differ from Coinbase product IDs.

```bash
cd build/bin
./algo --exchange=coinbase --algo=SimpleMR --api_key=$COINBASE_API_KEY --api_secret=$COINBASE_API_SECRET --api_phrase=$COINBASE_API_PHRASE --lookback=50 --reversion_level=2 --base_quantity=0.0025 --symbols ETH-BTC
```

Kraken spot example (pair names are Kraken/ccapi-specific, not Coinbase-style):

```bash
./algo --exchange=kraken --algo=SimpleMR --api_key=$KRAKEN_API_KEY --api_secret=$KRAKEN_API_SECRET --lookback=50 --reversion_level=2 --base_quantity=0.0025 --symbols XBT/USD
```
