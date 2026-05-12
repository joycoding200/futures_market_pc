#!/bin/bash
# 期货行情PC软件 — 快速构建脚本
# 用法: bash build.sh

set -e
export PATH="/c/Dev/Qt/Tools/mingw1310_64/bin:$PATH"

QT="C:/Dev/Qt/6.11.0/mingw_64"
SRC="C:/Dev/Claude-Code/pc_futuresmarketanalysis"
BIN="$SRC/bin"
MOC="$QT/bin/moc"

cd "$SRC"
rm -rf "$BIN"
mkdir -p "$BIN/moc"

echo "=== 1/3 运行 MOC (Qt 元对象编译器) ==="
for h in \
  src/app/Application.h src/app/MainWindow.h \
  src/core/EventBus.h \
  src/data/MarketDataProvider.h src/data/SimDataProvider.h \
  src/data/CTPDataProvider.h src/data/MarketDataBuffer.h src/data/KLineAggregator.h \
  src/chart/KLineChart.h src/chart/TimeSharingChart.h src/chart/QuotePanel.h \
  src/chart/ContractList.h src/chart/TradeRecord.h \
  src/db/Database.h \
  src/system/SettingsDialog.h src/system/ContractManager.h src/system/AboutDialog.h \
  src/stats/StatsCalculator.h src/stats/StatsPanel.h; do
  out="$BIN/moc/moc_$(basename $h .h).cpp"
  "$MOC" -o "$out" "$h"
done
echo "Generated $(ls $BIN/moc/*.cpp | wc -l) moc files"

echo "=== 2/3 编译 + 链接 ==="
g++ -std=c++17 -O2 -DSPDLOG_HEADER_ONLY \
  -Isrc -Ithird_party/spdlog/include -I"$BIN/moc" \
  -I"$QT/include" -I"$QT/include/QtCore" -I"$QT/include/QtGui" -I"$QT/include/QtWidgets" -I"$QT/include/QtSql" \
  src/main.cpp \
  src/app/Application.cpp src/app/MainWindow.cpp \
  src/core/EventBus.cpp src/core/Config.cpp \
  src/data/SimDataProvider.cpp "src/data/CTPDataProvider.cpp" \
  src/data/MarketDataBuffer.cpp src/data/KLineAggregator.cpp \
  src/chart/KLineChart.cpp src/chart/ChartAxis.cpp src/chart/ChartPainter.cpp \
  src/chart/Crosshair.cpp src/chart/TimeSharingChart.cpp src/chart/QuotePanel.cpp \
  src/chart/ContractList.cpp src/chart/TradeRecord.cpp \
  src/indicator/MA.cpp src/indicator/MACD.cpp src/indicator/KDJ.cpp src/indicator/RSI.cpp \
  src/db/Database.cpp \
  "src/system/SettingsDialog.cpp" "src/system/ContractManager.cpp" "src/system/AboutDialog.cpp" \
  src/stats/StatsCalculator.cpp src/stats/StatsPanel.cpp \
  $BIN/moc/moc_*.cpp \
  -o "$BIN/FuturesMarketPC.exe" \
  -L"$QT/lib" -lQt6Core -lQt6Gui -lQt6Widgets -lQt6Sql \
  -mwindows

echo "=== 3/3 完成 ==="
ls -lh "$BIN/FuturesMarketPC.exe"
echo "运行: $BIN/FuturesMarketPC.exe"
