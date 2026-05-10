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
    src/db/Database.cpp
)

set(HEADERS
    src/app/Application.h
    src/core/EventBus.h
    src/core/Config.h
    src/data/TickData.h
    src/data/KLineData.h
    src/data/MarketDataProvider.h
    src/data/SimDataProvider.h
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
    src/db/Database.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql
    spdlog::spdlog
    SQLite::SQLite3
    qcustomplot
)
