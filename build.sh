#!/bin/bash
# 期货行情PC软件 — 快速构建脚本 (CMake + MinGW)
# 用法: bash build.sh

set -e

MINGW="/c/Dev/Qt/Tools/mingw1310_64/bin"
QT="/c/Dev/Qt/6.11.0/mingw_64"
SRC="C:/Dev/Claude-Code/pc_futuresmarketanalysis"
BUILD="$SRC/build_cmake"

export PATH="$MINGW:$QT/bin:$PATH"

echo "=== 1/3 CMake 配置 ==="
cmake -G "MinGW Makefiles" \
    -DCMAKE_PREFIX_PATH="$QT" \
    -DCMAKE_BUILD_TYPE=Release \
    -B "$BUILD" -S "$SRC"

echo "=== 2/3 编译 ==="
cmake --build "$BUILD"

echo "=== 3/3 部署 Qt DLLs ==="
windeployqt "$BUILD/FuturesMarketPC.exe" --release --no-translations

echo "=== 完成 ==="
ls -lh "$BUILD/FuturesMarketPC.exe"
echo "运行: $BUILD/FuturesMarketPC.exe"
