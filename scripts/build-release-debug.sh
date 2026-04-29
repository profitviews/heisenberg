#!/usr/bin/env bash
# Build both Release and Debug in an existing multi-config tree (after cmake --preset conan-default).
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build}"

if [[ ! -f "$BUILD_DIR/CMakeCache.txt" ]]; then
  echo "Expected an existing CMake build dir at $BUILD_DIR (run cmake --preset conan-default first)." >&2
  exit 1
fi

cmake --build "$BUILD_DIR" --config Release --parallel
cmake --build "$BUILD_DIR" --config Debug --parallel
