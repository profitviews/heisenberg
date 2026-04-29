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
source build/conanbuild.sh
cmake --preset conan-default
```

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

Use debug Conan profiles for `conan install` when you need Conan-provided libraries built as Debug (same profile for `-pr:h` and `-pr:b`). Release profiles match Release dependencies.

#### Artifacts and example

Executables end up under `build/bin/` (sometimes with `Release` or `Debug` subdirectories, depending on platform and generator).

```bash
cd build/bin
./algo --exchange=coinbase --algo=SimpleMR --api_key=$COINBASE_API_KEY --api_secret=$COINBASE_API_SECRET --api_phrase=$COINBASE_API_PHRASE --lookback=50 --reversion_level=2 --base_quantity=0.0025 --symbol=ETH-BTC
```
