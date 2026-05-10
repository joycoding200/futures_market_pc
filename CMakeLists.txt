cmake_minimum_required(VERSION 3.22)
project(FuturesMarketPC VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql)
find_package(spdlog REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(qcustomplot REQUIRED)

set(SOURCES
    src/main.cpp
    src/app/Application.cpp
    src/core/EventBus.cpp
    src/core/Config.cpp
    src/data/SimDataProvider.cpp
    src/data/CTPDataProvider.cpp
    src/data/MarketDataBuffer.cpp
    src/data/KLineAggregator.cpp
    src/chart/KLineChart.cpp
    src/chart/ChartAxis.cpp
    src/chart/ChartPainter.cpp
    src/chart/Crosshair.cpp
    src/chart/TimeSharingChart.cpp
    src/chart/QuotePanel.cpp
    src/chart/ContractList.cpp
    src/chart/TradeRecord.cpp
    src/indicator/MA.cpp
    src/indicator/MACD.cpp
    src/indicator/KDJ.cpp
    src/indicator/RSI.cpp
    src/db/Database.cpp
    src/system/SettingsDialog.cpp
    src/system/ContractManager.cpp
    src/system/AboutDialog.cpp
)

set(HEADERS
    src/app/Application.h
    src/core/EventBus.h
    src/core/Config.h
    src/data/TickData.h
    src/data/KLineData.h
    src/data/MarketDataProvider.h
    src/data/SimDataProvider.h
    src/data/CTPDataProvider.h
    src/data/MarketDataBuffer.h
    src/data/KLineAggregator.h
    src/chart/KLineChart.h
    src/chart/ChartAxis.h
    src/chart/ChartPainter.h
    src/chart/Crosshair.h
    src/chart/TimeSharingChart.h
    src/chart/QuotePanel.h
    src/chart/ContractList.h
    src/chart/TradeRecord.h
    src/indicator/IndicatorBase.h
    src/indicator/MA.h
    src/indicator/MACD.h
    src/indicator/KDJ.h
    src/indicator/RSI.h
    src/db/Database.h
    src/system/SettingsDialog.h
    src/system/ContractManager.h
    src/system/AboutDialog.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql
    spdlog::spdlog
    SQLite::SQLite3
    qcustomplot
)

option(WITH_CTP "Build with CTP support" ON)
if(WITH_CTP)
    set(CTP_ROOT "${CMAKE_SOURCE_DIR}/third_party/ctp")
    target_include_directories(${PROJECT_NAME} PRIVATE "${CTP_ROOT}")
    target_link_libraries(${PROJECT_NAME} PRIVATE "${CTP_ROOT}/thostmduserapi_se")
endif()
