# Phase 1: 行情展示 — 实施方案

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 搭建 Qt/C++ 期货行情PC软件的骨架，实现实时行情数据接收（模拟源）、K线图/分时图/盘口/合约列表的完整展示。

**Architecture:** 模块化分层架构 — Core(EventBus/Config) → Data(MarketDataProvider/SimDataProvider/RingBuffer) → Chart(KLineChart/TimeSharingChart/QuotePanel/ContractList) → App(主窗口布局)。数据层通过 EventBus 信号向 UI 层推送行情，UI 组件按需订阅。

**Tech Stack:** Qt 6.x / C++17 / CMake + vcpkg / QPainter自绘 / SQLite

---

## 文件结构概览

```
src/
├── main.cpp                          # 入口
├── app/Application.h/cpp             # 应用壳，主窗口
├── core/EventBus.h/cpp               # 跨模块事件分发
├── core/Config.h/cpp                 # 全局配置管理（单例）
├── data/
│   ├── TickData.h                    # Tick数据结构
│   ├── KLineData.h                   # K线数据结构
│   ├── MarketDataProvider.h          # 抽象接口（纯虚类）
│   ├── SimDataProvider.h/cpp         # 模拟行情数据源
│   ├── MarketDataBuffer.h/cpp        # 实时数据环形缓存
│   └── KLineAggregator.h/cpp         # Tick→K线聚合器
├── chart/
│   ├── KLineChart.h/cpp              # K线图组件（QPainter自绘）
│   ├── ChartAxis.h/cpp               # 坐标轴计算
│   ├── ChartPainter.h/cpp            # K线/成交量/MA绘制
│   ├── Crosshair.h/cpp               # 十字光标
│   ├── TimeSharingChart.h/cpp        # 分时图组件
│   ├── QuotePanel.h/cpp              # 盘口面板
│   ├── ContractList.h/cpp            # 自选合约列表
│   └── TradeRecord.h/cpp             # 逐笔成交记录
├── db/Database.h/cpp                 # SQLite封装
└── resources/
tests/
├── test_data_layer.cpp               # 数据层测试
├── test_kline_chart.cpp              # K线图测试
└── test_main_window.cpp              # 主窗口测试
```

---

### Task 1: 项目骨架与CMake构建系统

**Files:**
- Create: `CMakeLists.txt`
- Create: `vcpkg.json`
- Create: `src/main.cpp`
- Create: `src/app/Application.h`, `src/app/Application.cpp`

- [ ] **Step 1: 编写 vcpkg 依赖清单**

```json
{
  "name": "futures-market-pc",
  "version": "0.1.0",
  "dependencies": [
    "qt6",
    "spdlog",
    "sqlite3",
    "qcustomplot"
  ]
}
```

- [ ] **Step 2: 编写根 CMakeLists.txt**

```cmake
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
```

- [ ] **Step 3: 编写 Application 骨架**

头文件 `src/app/Application.h`:
```cpp
#pragma once
#include <QApplication>
#include <spdlog/spdlog.h>

class Application : public QApplication {
    Q_OBJECT
public:
    Application(int& argc, char** argv);
    ~Application();
    int run();

private:
    void initLogging();
    void initEventBus();
    void initDataLayer();
    void initMainWindow();
};
```

实现 `src/app/Application.cpp`:
```cpp
#include "Application.h"
#include "core/EventBus.h"
#include "core/Config.h"
#include "data/SimDataProvider.h"
#include "data/MarketDataBuffer.h"
#include "data/KLineAggregator.h"

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv) {
    setApplicationName("期货行情PC软件");
    setApplicationVersion("0.1.0");
    initLogging();
}

Application::~Application() {}

void Application::initLogging() {
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("期货行情PC软件启动");
}

void Application::initEventBus() {
    EventBus::instance();
    spdlog::debug("EventBus 初始化完成");
}

void Application::initDataLayer() {
    // 创建模拟数据源、数据缓存、K线聚合器
    auto* simProvider = new SimDataProvider(this);
    auto* buffer = new MarketDataBuffer(this);
    auto* aggregator = new KLineAggregator(buffer, this);

    QObject::connect(simProvider, &SimDataProvider::tickReceived,
                     buffer, &MarketDataBuffer::onTick);
    QObject::connect(buffer, &MarketDataBuffer::tickUpdated,
                     aggregator, &KLineAggregator::onTick);

    simProvider->start();
    spdlog::debug("数据层初始化完成");
}

int Application::run() {
    initEventBus();
    initDataLayer();
    initMainWindow();
    return exec();
}
```

- [ ] **Step 4: 编写 main.cpp 入口**

```cpp
#include "app/Application.h"

int main(int argc, char* argv[]) {
    Application app(argc, argv);
    return app.run();
}
```

- [ ] **Step 5: 构建验证**

```bash
cd C:/Dev/Claude-Code/pc_futuresmarketanalysis
mkdir -p build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE="[vcpkg-root]/scripts/buildsystems/vcpkg.cmake"
cmake --build .
```
验证：编译成功，无错误。

- [ ] **Step 6: Commit**

```bash
git add CMakeLists.txt vcpkg.json src/main.cpp src/app/
git commit -m "feat: add project skeleton with CMake/Qt6 build system"
```

---

### Task 2: 核心基础设施 — EventBus 与 Config

**Files:**
- Create: `src/core/EventBus.h`, `src/core/EventBus.cpp`
- Create: `src/core/Config.h`, `src/core/Config.cpp`

- [ ] **Step 1: 编写 EventBus**

头文件 `src/core/EventBus.h`:
```cpp
#pragma once
#include <QObject>

// 跨模块事件分发中心（单例）
// 各模块通过 EventBus 订阅/发布事件，避免模块间直接依赖
class EventBus : public QObject {
    Q_OBJECT
public:
    static EventBus& instance();
    enum class EventType {
        TickUpdate,
        KLineUpdate,
        ContractSelected,
        PeriodChanged,
        DataSourceChanged
    };

signals:
    void tickUpdated(const QString& contract, double price, double volume);
    void klineUpdated(const QString& contract, const QVariantMap& kline);
    void contractSelected(const QString& contract);
    void periodChanged(const QString& period);

private:
    EventBus() = default;
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;
};
```

实现 `src/core/EventBus.cpp`:
```cpp
#include "EventBus.h"

EventBus& EventBus::instance() {
    static EventBus bus;
    return bus;
}
```

- [ ] **Step 2: 编写 Config 配置管理**

头文件 `src/core/Config.h`:
```cpp
#pragma once
#include <QString>
#include <QSettings>
#include <memory>

// 全局配置管理（单例），基于 QSettings 持久化
class Config {
public:
    static Config& instance();

    // 行情配置
    int refreshIntervalMs() const;        // 默认250ms
    void setRefreshIntervalMs(int ms);

    int maxKLineCount() const;            // 默认2000根
    void setMaxKLineCount(int count);

    QString defaultContracts() const;     // 默认自选合约列表
    void setDefaultContracts(const QString& contracts);

    // 模拟数据配置
    double simBasePrice() const;          // 默认4000
    double simVolatility() const;         // 默认0.001

private:
    Config();
    std::unique_ptr<QSettings> m_settings;
};
```

实现 `src/core/Config.cpp`:
```cpp
#include "Config.h"

Config& Config::instance() {
    static Config c;
    return c;
}

Config::Config()
    : m_settings(std::make_unique<QSettings>("FuturesMarketPC", "config")) {
    // 确保默认值存在
    if (!m_settings->contains("refreshInterval")) m_settings->setValue("refreshInterval", 250);
    if (!m_settings->contains("maxKLineCount")) m_settings->setValue("maxKLineCount", 2000);
    if (!m_settings->contains("simBasePrice")) m_settings->setValue("simBasePrice", 4000.0);
    if (!m_settings->contains("simVolatility")) m_settings->setValue("simVolatility", 0.001);
    m_settings->sync();
}

int Config::refreshIntervalMs() const { return m_settings->value("refreshInterval").toInt(); }
void Config::setRefreshIntervalMs(int ms) { m_settings->setValue("refreshInterval", ms); }

int Config::maxKLineCount() const { return m_settings->value("maxKLineCount").toInt(); }
void Config::setMaxKLineCount(int count) { m_settings->setValue("maxKLineCount", count); }

QString Config::defaultContracts() const { return m_settings->value("defaultContracts").toString(); }
void Config::setDefaultContracts(const QString& cs) { m_settings->setValue("defaultContracts", cs); }

double Config::simBasePrice() const { return m_settings->value("simBasePrice").toDouble(); }
double Config::simVolatility() const { return m_settings->value("simVolatility").toDouble(); }
```

- [ ] **Step 3: 编译验证**

```bash
cd C:/Dev/Claude-Code/pc_futuresmarketanalysis/build
cmake --build .
```
验证：`EventBus.o` 和 `Config.o` 编译成功。

- [ ] **Step 4: Commit**

```bash
git add src/core/
git commit -m "feat: add EventBus singleton and Config manager"
```

---

### Task 3: 数据结构定义

**Files:**
- Create: `src/data/TickData.h`
- Create: `src/data/KLineData.h`

- [ ] **Step 1: 编写 TickData**

```cpp
#pragma once
#include <QString>
#include <cstdint>
#include <array>

// 实时行情Tick数据结构
// 含最新价、成交量、持仓量、五档买卖盘
struct TickData {
    QString   contract;                     // 合约代码，如 "rb2510"
    double    lastPrice    = 0.0;           // 最新成交价
    double    openPrice    = 0.0;           // 开盘价
    double    highPrice    = 0.0;           // 最高价
    double    lowPrice     = 0.0;           // 最低价
    double    volume       = 0.0;           // 成交量
    double    openInterest = 0.0;           // 持仓量
    double    preSettle    = 0.0;           // 昨结算价

    std::array<double, 5> bidPrice  = {};  // 买一~买五
    std::array<double, 5> bidVolume = {};  // 买一~买五量
    std::array<double, 5> askPrice  = {};  // 卖一~卖五
    std::array<double, 5> askVolume = {};  // 卖一~卖五量

    int64_t   timestamp    = 0;            // 毫秒时间戳

    bool isValid() const { return contract.isEmpty() == false && lastPrice > 0; }
};
```

- [ ] **Step 2: 编写 KLineData**

```cpp
#pragma once
#include <QString>
#include <cstdint>

// K线周期的枚举
enum class KLineType {
    M1, M5, M15, M30, H1, D, W, M
};

inline QString klineTypeToString(KLineType t) {
    switch (t) {
        case KLineType::M1:  return "1m";
        case KLineType::M5:  return "5m";
        case KLineType::M15: return "15m";
        case KLineType::M30: return "30m";
        case KLineType::H1:  return "1H";
        case KLineType::D:   return "D";
        case KLineType::W:   return "W";
        case KLineType::M:   return "M";
    }
    return "D";
}

// 单根K线数据
struct KLineData {
    QString   contract;         // 合约代码
    double    open        = 0.0;
    double    high        = 0.0;
    double    low         = 0.0;
    double    close       = 0.0;
    double    volume      = 0.0;
    double    openInterest = 0.0;
    int64_t   timestamp   = 0;  // 该周期起始时间（毫秒）
    KLineType type        = KLineType::D;
    bool      isComplete  = false; // 是否已闭合（历史数据为true）

    // 涨跌判断
    bool isRising() const { return close >= open; }
    // K线的最高最低范围
    double range() const { return high - low; }
};

// 按时间戳排序用的比较函数
inline bool operator<(const KLineData& a, const KLineData& b) {
    return a.timestamp < b.timestamp;
}
```

- [ ] **Step 3: 编译验证**

```bash
cd C:/Dev/Claude-Code/pc_futuresmarketanalysis/build
cmake --build .
```
验证：头文件编译通过（被后续cpp引用时验证）。

- [ ] **Step 4: Commit**

```bash
git add src/data/TickData.h src/data/KLineData.h
git commit -m "feat: define TickData and KLineData structures"
```

---

### Task 4: MarketDataProvider 抽象接口 + SimDataProvider 模拟数据源

**Files:**
- Create: `src/data/MarketDataProvider.h`
- Create: `src/data/SimDataProvider.h`, `src/data/SimDataProvider.cpp`

- [ ] **Step 1: 编写 MarketDataProvider 抽象接口**

```cpp
#pragma once
#include <QObject>
#include <QString>
#include <QVector>
#include "TickData.h"
#include "KLineData.h"

// 行情数据提供者抽象基类
// CTPProvider、HttpProvider、SimProvider 均继承此类
class MarketDataProvider : public QObject {
    Q_OBJECT
public:
    explicit MarketDataProvider(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~MarketDataProvider() = default;

    // 订阅/退订合约行情
    virtual void subscribe(const QString& contract) = 0;
    virtual void unsubscribe(const QString& contract) = 0;
    virtual void subscribeAll(const QVector<QString>& contracts) = 0;

    // 请求历史K线数据
    virtual void requestHistory(const QString& contract, KLineType type,
                                int count) = 0;

    // 启动/停止数据源
    virtual void start() = 0;
    virtual void stop() = 0;

    virtual QString providerName() const = 0;

signals:
    void tickReceived(const TickData& tick);
    void klineReceived(const KLineData& kline);
    void historyLoaded(const QString& contract, const QVector<KLineData>& data);
    void connectionStatus(const QString& status);
    void errorOccurred(const QString& error);
};
```

- [ ] **Step 2: 编写 SimDataProvider 头文件**

```cpp
#pragma once
#include <QTimer>
#include <QRandomGenerator>
#include <QMap>
#include "MarketDataProvider.h"
#include "KLineAggregator.h"

// 模拟行情数据源，用于开发调试
// 基于随机游走生成仿真的期货Tick数据
class SimDataProvider : public MarketDataProvider {
    Q_OBJECT
public:
    explicit SimDataProvider(QObject* parent = nullptr);
    ~SimDataProvider() override;

    void subscribe(const QString& contract) override;
    void unsubscribe(const QString& contract) override;
    void subscribeAll(const QVector<QString>& contracts) override;
    void requestHistory(const QString& contract, KLineType type,
                        int count) override;
    void start() override;
    void stop() override;
    QString providerName() const override { return "Simulated"; }

private slots:
    void onTimerTick();

private:
    TickData generateTick(const QString& contract);
    double randomWalk(double currentPrice, double volatility, double min, double max);

    QTimer* m_timer;
    QMap<QString, double> m_prices;    // 合约当前价格
    QMap<QString, double> m_opens;     // 合约开盘价
    QRandomGenerator m_rng;
    static constexpr int DEFAULT_INTERVAL_MS = 250;
    static constexpr double DEFAULT_BASE_PRICE = 4000.0;
    static constexpr double DEFAULT_VOLATILITY = 2.0; // 单Tick价格波动
};
```

- [ ] **Step 3: 编写 SimDataProvider 实现**

```cpp
#include "SimDataProvider.h"
#include "core/Config.h"
#include <spdlog/spdlog.h>
#include <cmath>

SimDataProvider::SimDataProvider(QObject* parent)
    : MarketDataProvider(parent)
    , m_timer(new QTimer(this))
{
    connect(m_timer, &QTimer::timeout, this, &SimDataProvider::onTimerTick);
}

SimDataProvider::~SimDataProvider() { stop(); }

void SimDataProvider::subscribe(const QString& contract) {
    if (!m_prices.contains(contract)) {
        m_prices[contract] = Config::instance().simBasePrice();
        m_opens[contract] = Config::instance().simBasePrice();
        spdlog::info("SimDataProvider: 订阅合约 {}", contract.toStdString());
    }
}

void SimDataProvider::unsubscribe(const QString& contract) {
    m_prices.remove(contract);
    m_opens.remove(contract);
}

void SimDataProvider::subscribeAll(const QVector<QString>& contracts) {
    for (const auto& c : contracts) subscribe(c);
}

void SimDataProvider::requestHistory(const QString& contract, KLineType type, int count) {
    // Phase 1 暂不实现历史数据，Phase 2 从 SQLite 加载
    Q_UNUSED(contract); Q_UNUSED(type); Q_UNUSED(count);
}

void SimDataProvider::start() {
    m_timer->start(DEFAULT_INTERVAL_MS);
    spdlog::info("SimDataProvider 已启动，刷新间隔 {}ms", DEFAULT_INTERVAL_MS);
}

void SimDataProvider::stop() {
    m_timer->stop();
    spdlog::info("SimDataProvider 已停止");
}

void SimDataProvider::onTimerTick() {
    for (auto it = m_prices.begin(); it != m_prices.end(); ++it) {
        TickData tick = generateTick(it.key());
        it.value() = tick.lastPrice;
        emit tickReceived(tick);
    }
}

TickData SimDataProvider::generateTick(const QString& contract) {
    TickData tick;
    tick.contract = contract;
    tick.timestamp = QDateTime::currentMSecsSinceEpoch();

    double currentPrice = m_prices.value(contract);
    double basePrice = Config::instance().simBasePrice();
    tick.lastPrice = randomWalk(currentPrice, DEFAULT_VOLATILITY,
                                basePrice * 0.85, basePrice * 1.15);
    tick.openPrice = m_opens.value(contract);
    tick.highPrice = std::max(tick.highPrice, tick.lastPrice);
    tick.lowPrice = tick.lowPrice > 0 ? std::min(tick.lowPrice, tick.lastPrice) : tick.lastPrice;
    tick.volume = m_rng.bounded(1, 50);
    tick.openInterest = 100000 + m_rng.bounded(-500, 500);
    tick.preSettle = basePrice;

    // 模拟五档盘口
    double spread = basePrice * 0.0001; // 约1个最小变动价位
    for (int i = 0; i < 5; ++i) {
        tick.askPrice[i] = tick.lastPrice + spread * (i + 1);
        tick.askVolume[i] = m_rng.bounded(100, 5000);
        tick.bidPrice[i] = tick.lastPrice - spread * (i + 1);
        tick.bidVolume[i] = m_rng.bounded(100, 5000);
    }
    return tick;
}

double SimDataProvider::randomWalk(double currentPrice, double volatility, double min, double max) {
    double change = (m_rng.generateDouble() - 0.5) * 2.0 * volatility;
    double newPrice = currentPrice + change;
    return std::clamp(newPrice, min, max);
}
```

- [ ] **Step 4: 编译构建**

```bash
cd C:/Dev/Claude-Code/pc_futuresmarketanalysis/build
cmake --build .
```
验证：编译成功。

- [ ] **Step 5: Commit**

```bash
git add src/data/MarketDataProvider.h src/data/SimDataProvider.h src/data/SimDataProvider.cpp
git commit -m "feat: add MarketDataProvider interface and SimDataProvider implementation"
```

---

### Task 5: MarketDataBuffer 环形缓存 + KLineAggregator K线聚合器

**Files:**
- Create: `src/data/MarketDataBuffer.h`, `src/data/MarketDataBuffer.cpp`
- Create: `src/data/KLineAggregator.h`, `src/data/KLineAggregator.cpp`

- [ ] **Step 1: 编写 MarketDataBuffer 头文件**

```cpp
#pragma once
#include <QObject>
#include <QMap>
#include <QVector>
#include <deque>
#include "TickData.h"
#include "KLineData.h"

// 实时行情数据环形缓存
// 按合约存储最近N个Tick和K线数据，自动淘汰旧数据
class MarketDataBuffer : public QObject {
    Q_OBJECT
public:
    explicit MarketDataBuffer(int maxItemsPerContract = 2000, QObject* parent = nullptr);

    // 接收新数据
    void onTick(const TickData& tick);
    void onKLine(const KLineData& kline);

    // 查询接口
    TickData latestTick(const QString& contract) const;
    QVector<KLineData> klines(const QString& contract, KLineType type, int count = -1) const;
    KLineData latestKLine(const QString& contract, KLineType type) const;

    // 自选合约管理
    QStringList subscribedContracts() const;
    void addContract(const QString& contract);

signals:
    void tickUpdated(const TickData& tick);
    void klineUpdated(const KLineData& kline);

private:
    int m_maxItems;
    QMap<QString, TickData> m_latestTicks;
    QMap<QString, std::deque<KLineData>> m_klines;  // key = "contract:type"
    QStringList m_contracts;

    static QString makeKey(const QString& contract, KLineType type) {
        return contract + ":" + klineTypeToString(type);
    }
};
```

- [ ] **Step 2: 编写 MarketDataBuffer 实现**

```cpp
#include "MarketDataBuffer.h"
#include <spdlog/spdlog.h>

MarketDataBuffer::MarketDataBuffer(int maxItemsPerContract, QObject* parent)
    : QObject(parent), m_maxItems(maxItemsPerContract) {}

void MarketDataBuffer::onTick(const TickData& tick) {
    if (!tick.isValid()) return;
    m_latestTicks[tick.contract] = tick;
    emit tickUpdated(tick);
}

void MarketDataBuffer::onKLine(const KLineData& kline) {
    QString key = makeKey(kline.contract, kline.type);
    auto& deq = m_klines[key];

    // 更新最后一根K线或追加新K线
    if (!deq.empty() && deq.back().timestamp == kline.timestamp) {
        deq.back() = kline;
    } else {
        deq.push_back(kline);
    }

    // 淘汰旧数据
    while (static_cast<int>(deq.size()) > m_maxItems) {
        deq.pop_front();
    }
    emit klineUpdated(kline);
}

TickData MarketDataBuffer::latestTick(const QString& contract) const {
    return m_latestTicks.value(contract);
}

QVector<KLineData> MarketDataBuffer::klines(const QString& contract, KLineType type, int count) const {
    QString key = makeKey(contract, type);
    const auto& deq = m_klines.value(key);
    if (count < 0 || count >= static_cast<int>(deq.size())) {
        return QVector<KLineData>(deq.begin(), deq.end());
    }
    // 返回最近的 count 根K线
    auto start = deq.end() - count;
    return QVector<KLineData>(start, deq.end());
}

KLineData MarketDataBuffer::latestKLine(const QString& contract, KLineType type) const {
    QString key = makeKey(contract, type);
    const auto& deq = m_klines.value(key);
    return deq.empty() ? KLineData() : deq.back();
}

QStringList MarketDataBuffer::subscribedContracts() const { return m_contracts; }
void MarketDataBuffer::addContract(const QString& contract) {
    if (!m_contracts.contains(contract)) m_contracts.append(contract);
}
```

- [ ] **Step 3: 编写 KLineAggregator 头文件**

```cpp
#pragma once
#include <QObject>
#include <QMap>
#include "TickData.h"
#include "KLineData.h"

class MarketDataBuffer;

// Tick→K线聚合器
// 将实时Tick数据聚合成各周期的K线数据
class KLineAggregator : public QObject {
    Q_OBJECT
public:
    explicit KLineAggregator(MarketDataBuffer* buffer, QObject* parent = nullptr);

public slots:
    void onTick(const TickData& tick);

private:
    void aggregateKLine(const TickData& tick, KLineType type);
    int64_t roundToPeriod(int64_t timestamp, KLineType type);
    int periodMinutes(KLineType type);

    MarketDataBuffer* m_buffer;
    // 各周期当前正在构建的K线
    QMap<QString, KLineData> m_currentKlines[8]; // 8种周期
};
```

- [ ] **Step 4: 编写 KLineAggregator 实现**

```cpp
#include "KLineAggregator.h"
#include "MarketDataBuffer.h"
#include <spdlog/spdlog.h>

KLineAggregator::KLineAggregator(MarketDataBuffer* buffer, QObject* parent)
    : QObject(parent), m_buffer(buffer) {}

void KLineAggregator::onTick(const TickData& tick) {
    // 聚合所有8种周期的K线
    static const KLineType types[] = {
        KLineType::M1, KLineType::M5, KLineType::M15, KLineType::M30,
        KLineType::H1, KLineType::D, KLineType::W, KLineType::M
    };
    for (auto type : types) {
        aggregateKLine(tick, type);
    }
}

void KLineAggregator::aggregateKLine(const TickData& tick, KLineType type) {
    QString key = tick.contract + ":" + klineTypeToString(type);
    int64_t periodStart = roundToPeriod(tick.timestamp, type);
    auto& current = m_currentKlines[static_cast<int>(type)][key];

    // 新周期开始
    if (current.timestamp != periodStart) {
        if (current.timestamp > 0) {
            current.isComplete = true;
            m_buffer->onKLine(current);
        }
        current = KLineData{};
        current.contract = tick.contract;
        current.type = type;
        current.timestamp = periodStart;
        current.open = tick.lastPrice;
        current.high = tick.lastPrice;
        current.low = tick.lastPrice;
    }

    // 更新当前K线
    current.close = tick.lastPrice;
    current.high = std::max(current.high, tick.lastPrice);
    current.low = std::min(current.low, tick.lastPrice);
    current.volume += tick.volume;
    current.openInterest = tick.openInterest;
    current.isComplete = false;

    // 推送到 Buffer
    m_buffer->onKLine(current);
}

int64_t KLineAggregator::roundToPeriod(int64_t timestamp, KLineType type) {
    int minutes = periodMinutes(type);
    int64_t msPerMinute = 60000LL;
    int64_t periodMs = static_cast<int64_t>(minutes) * msPerMinute;

    if (type == KLineType::D) {
        // 日K：按天对齐
        QDateTime dt = QDateTime::fromMSecsSinceEpoch(timestamp);
        QDateTime dayStart(dt.date(), QTime(0, 0));
        return dayStart.toMSecsSinceEpoch();
    }
    return (timestamp / periodMs) * periodMs;
}

int KLineAggregator::periodMinutes(KLineType type) {
    switch (type) {
        case KLineType::M1:  return 1;
        case KLineType::M5:  return 5;
        case KLineType::M15: return 15;
        case KLineType::M30: return 30;
        case KLineType::H1:  return 60;
        case KLineType::D:   return 1440;
        default: return 1440;
    }
}
```

- [ ] **Step 5: 编译构建**

```bash
cd C:/Dev/Claude-Code/pc_futuresmarketanalysis/build
cmake --build .
```
验证：编译成功。

- [ ] **Step 6: Commit**

```bash
git add src/data/MarketDataBuffer.h src/data/MarketDataBuffer.cpp \
        src/data/KLineAggregator.h src/data/KLineAggregator.cpp
git commit -m "feat: add MarketDataBuffer ring buffer and KLineAggregator"
```

---

### Task 6: KLineChart 核心图表组件

**Files:**
- Create: `src/chart/ChartAxis.h`, `src/chart/ChartAxis.cpp`
- Create: `src/chart/ChartPainter.h`, `src/chart/ChartPainter.cpp`
- Create: `src/chart/Crosshair.h`, `src/chart/Crosshair.cpp`
- Create: `src/chart/KLineChart.h`, `src/chart/KLineChart.cpp`

- [ ] **Step 1: 编写 ChartAxis 坐标轴**

头文件 `src/chart/ChartAxis.h`:
```cpp
#pragma once
#include <QVector>
#include "data/KLineData.h"

// 计算K线图坐标映射
struct ChartAxis {
    // 数据范围
    double priceMin = 0.0;
    double priceMax = 0.0;
    double volumeMax = 0.0;

    // 可视像素范围
    int chartHeight = 600;
    int chartWidth = 800;

    // 从K线数组计算价格范围
    void calcPriceRange(const QVector<KLineData>& visibleKlines);
    // 价格 → 像素Y坐标（Y轴反转：高价=小像素值）
    int priceToY(double price, double margin = 0.1) const;
    // 音量 → 像素Y坐标
    int volumeToY(double volume) const;
    // K线索引 → 像素X坐标
    double klineIndexToX(int index, int visibleCount) const;
    // 像素X → K线索引
    int pixelXToIndex(double pixelX, int visibleCount, int startIndex) const;

    static constexpr int RIGHT_MARGIN = 60;  // 右侧留白(Y轴标签)
    static constexpr int BOTTOM_MARGIN = 30; // 底部留白(X轴标签)
};
```

实现 `src/chart/ChartAxis.cpp`:
```cpp
#include "ChartAxis.h"
#include <algorithm>
#include <cmath>

void ChartAxis::calcPriceRange(const QVector<KLineData>& visibleKlines) {
    if (visibleKlines.isEmpty()) return;
    priceMax = visibleKlines.first().high;
    priceMin = visibleKlines.first().low;
    volumeMax = visibleKlines.first().volume;
    for (const auto& k : visibleKlines) {
        priceMax = std::max(priceMax, k.high);
        priceMin = std::min(priceMin, k.low);
        volumeMax = std::max(volumeMax, k.volume);
    }
    if (priceMax == priceMin) priceMax += 1.0;
}

int ChartAxis::priceToY(double price, double margin) const {
    double range = priceMax - priceMin;
    double padding = range * margin;
    double adjustedMax = priceMax + padding;
    double adjustedMin = priceMin - padding;
    double ratio = (price - adjustedMin) / (adjustedMax - adjustedMin);
    return static_cast<int>((chartHeight - BOTTOM_MARGIN) * (1.0 - ratio));
}

int ChartAxis::volumeToY(double volume) const {
    if (volumeMax <= 0) return chartHeight - BOTTOM_MARGIN;
    int volumeAreaHeight = chartHeight / 4; // 成交量占底部1/4
    double ratio = volume / volumeMax;
    return chartHeight - BOTTOM_MARGIN - static_cast<int>(ratio * volumeAreaHeight);
}

double ChartAxis::klineIndexToX(int index, int visibleCount) const {
    double candleWidth = static_cast<double>(chartWidth - RIGHT_MARGIN) / visibleCount;
    return index * candleWidth;
}

int ChartAxis::pixelXToIndex(double pixelX, int visibleCount, int startIndex) const {
    double candleWidth = static_cast<double>(chartWidth - RIGHT_MARGIN) / visibleCount;
    return startIndex + static_cast<int>(pixelX / candleWidth);
}
```

- [ ] **Step 2: 编写 ChartPainter 绘制引擎**

头文件 `src/chart/ChartPainter.h`:
```cpp
#pragma once
#include <QPainter>
#include <QVector>
#include "data/KLineData.h"

class ChartAxis;

// QPainter K线绘制引擎
// 负责K线、成交量柱、MA均线的像素级绘制
class ChartPainter {
public:
    explicit ChartPainter(QPainter* painter) : m_painter(painter) {}

    void drawBackground(const QRect& rect, const QColor& bgColor);
    void drawGrid(const ChartAxis& axis, int gridRows, int gridCols);
    void drawCandles(const QVector<KLineData>& klines, const ChartAxis& axis,
                     int startIndex, int visibleCount);
    void drawVolume(const QVector<KLineData>& klines, const ChartAxis& axis,
                    int startIndex, int visibleCount);
    void drawPriceLabels(const ChartAxis& axis, int labelCount = 5);

    QColor upColor()   const { return QColor(239, 68, 68); }    // 红色阳线
    QColor downColor() const { return QColor(34, 197, 94); }    // 绿色阴线

private:
    QPainter* m_painter;
    static constexpr int CANDLE_GAP = 1;  // K线间距
};
```

实现 `src/chart/ChartPainter.cpp`:
```cpp
#include "ChartPainter.h"
#include "ChartAxis.h"

void ChartPainter::drawBackground(const QRect& rect, const QColor& bgColor) {
    m_painter->fillRect(rect, bgColor);
}

void ChartPainter::drawGrid(const ChartAxis& axis, int gridRows, int gridCols) {
    QPen gridPen(QColor(40, 40, 50, 80), 1, Qt::DotLine);
    m_painter->setPen(gridPen);
    for (int i = 0; i <= gridRows; ++i) {
        int y = i * (axis.chartHeight - axis.BOTTOM_MARGIN) / gridRows;
        m_painter->drawLine(0, y, axis.chartWidth - axis.RIGHT_MARGIN, y);
    }
}

void ChartPainter::drawCandles(const QVector<KLineData>& klines, const ChartAxis& axis,
                                int startIndex, int visibleCount) {
    double barWidth = static_cast<double>(axis.chartWidth - axis.RIGHT_MARGIN) / visibleCount - CANDLE_GAP * 2;
    if (barWidth < 1.0) barWidth = 1.0;  // 缩放极限时最小1px

    for (int i = 0; i < visibleCount && (startIndex + i) < klines.size(); ++i) {
        const auto& k = klines[startIndex + i];
        double x = axis.klineIndexToX(i, visibleCount) + CANDLE_GAP;

        int highY = axis.priceToY(k.high);
        int lowY  = axis.priceToY(k.low);
        int openY = axis.priceToY(k.open);
        int closeY = axis.priceToY(k.close);
        int candleTop = std::min(openY, closeY);
        int candleBody = std::abs(closeY - openY);
        if (candleBody == 0) candleBody = 1;

        QColor color = k.isRising() ? upColor() : downColor();
        m_painter->setPen(QPen(color, 1));
        m_painter->setBrush(color);

        // 绘制影线
        m_painter->drawLine(QPointF(x + barWidth / 2, highY),
                            QPointF(x + barWidth / 2, candleTop));
        m_painter->drawLine(QPointF(x + barWidth / 2, candleTop + candleBody),
                            QPointF(x + barWidth / 2, lowY));

        // 绘制实体
        if (k.isRising()) {
            m_painter->setBrush(Qt::NoBrush);  // 空心阳线
            m_painter->drawRect(QRectF(x, candleTop, barWidth, candleBody));
        } else {
            m_painter->setBrush(color);         // 实心阴线
            m_painter->drawRect(QRectF(x, candleTop, barWidth, candleBody));
        }
    }
}

void ChartPainter::drawVolume(const QVector<KLineData>& klines, const ChartAxis& axis,
                               int startIndex, int visibleCount) {
    double barWidth = static_cast<double>(axis.chartWidth - axis.RIGHT_MARGIN) / visibleCount - CANDLE_GAP * 2;
    if (barWidth < 1.0) barWidth = 1.0;

    for (int i = 0; i < visibleCount && (startIndex + i) < klines.size(); ++i) {
        const auto& k = klines[startIndex + i];
        double x = axis.klineIndexToX(i, visibleCount) + CANDLE_GAP;
        int volumeBottom = axis.volumeToY(0);
        int volumeTop = axis.volumeToY(k.volume);
        int volumeHeight = volumeBottom - volumeTop;

        QColor color = k.isRising() ? upColor() : downColor();
        color.setAlpha(120);
        m_painter->setPen(Qt::NoPen);
        m_painter->setBrush(color);
        m_painter->drawRect(QRectF(x, volumeTop, barWidth, std::max(1, volumeHeight)));
    }
}

void ChartPainter::drawPriceLabels(const ChartAxis& axis, int labelCount) {
    m_painter->setPen(QColor(180, 180, 190));
    QFont font("Consolas", 9);
    m_painter->setFont(font);
    for (int i = 0; i <= labelCount; ++i) {
        double price = axis.priceMin + (axis.priceMax - axis.priceMin) * (labelCount - i) / labelCount;
        int y = axis.priceToY(price);
        m_painter->drawText(axis.chartWidth - axis.RIGHT_MARGIN + 5, y + 4,
                            QString::number(price, 'f', 0));
    }
}
```

- [ ] **Step 3: 编写 Crosshair 十字光标**

头文件 `src/chart/Crosshair.h`:
```cpp
#pragma once
#include <QPoint>
#include <QPainter>

// 十字光标交互组件
struct Crosshair {
    bool   visible = false;
    QPoint pos;             // 当前鼠标位置
    QPoint dataCoord;       // 对应的K线数据索引 (index, priceY)

    void draw(QPainter* painter, const QRect& chartRect) const;
    void moveTo(const QPoint& p, const QRect& chartRect);
    void hide() { visible = false; }
};
```

实现 `src/chart/Crosshair.cpp`:
```cpp
#include "Crosshair.h"

void Crosshair::draw(QPainter* painter, const QRect& chartRect) const {
    if (!visible) return;
    painter->save();
    QPen pen(QColor(255, 255, 255, 180), 1, Qt::DotLine);
    painter->setPen(pen);
    // 水平线
    painter->drawLine(chartRect.left(), pos.y(), chartRect.right(), pos.y());
    // 垂直线
    painter->drawLine(pos.x(), chartRect.top(), pos.x(), chartRect.bottom());
    painter->restore();
}

void Crosshair::moveTo(const QPoint& p, const QRect& chartRect) {
    pos = p;
    visible = true;
}
```

- [ ] **Step 4: 编写 KLineChart 主组件**

头文件 `src/chart/KLineChart.h`:
```cpp
#pragma once
#include <QWidget>
#include <QVector>
#include "data/KLineData.h"
#include "data/TickData.h"
#include "ChartAxis.h"
#include "ChartPainter.h"
#include "Crosshair.h"

class MarketDataBuffer;

// K线图主组件，QWidget + QPainter 自绘
// 集成K线、成交量、十字光标、缩放平移等交互
class KLineChart : public QWidget {
    Q_OBJECT
public:
    explicit KLineChart(MarketDataBuffer* buffer, QWidget* parent = nullptr);

    void setPeriod(KLineType type);
    KLineType period() const { return m_period; }
    void setContract(const QString& contract);

signals:
    void crosshairInfo(const QString& info);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

public slots:
    void onTickUpdated(const TickData& tick);
    void onKLineUpdated(const KLineData& kline);

private:
    void recalcAxis();
    void drawAll(QPainter& painter);

    MarketDataBuffer* m_buffer;
    QString m_contract;
    KLineType m_period = KLineType::D;

    // 可视状态
    int m_startIndex = 0;       // 当前可视范围起始K线索引
    int m_visibleCount = 120;   // 默认显示120根K线

    ChartAxis m_axis;
    ChartPainter m_painterFn;
    Crosshair m_crosshair;

    bool m_isDragging = false;
    QPoint m_dragStart;
    int m_dragStartIndex = 0;

    QColor m_bgColor{22, 22, 30};       // 深色背景
    QColor m_volumeAreaBg{25, 25, 35};
};
```

实现 `src/chart/KLineChart.cpp`:
```cpp
#include "KLineChart.h"
#include "data/MarketDataBuffer.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <cmath>

KLineChart::KLineChart(MarketDataBuffer* buffer, QWidget* parent)
    : QWidget(parent), m_buffer(buffer), m_painterFn(nullptr) {
    setMouseTracking(true);
    setMinimumSize(600, 400);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void KLineChart::setPeriod(KLineType type) {
    m_period = type;
    m_startIndex = 0;
    update();
}

void KLineChart::setContract(const QString& contract) {
    m_contract = contract;
    m_startIndex = 0;
    update();
}

void KLineChart::onTickUpdated(const TickData& tick) {
    if (tick.contract != m_contract) return;
    update(); // 增量刷新（只重绘最后一根K线区域）
}

void KLineChart::onKLineUpdated(const KLineData& kline) {
    if (kline.contract != m_contract || kline.type != m_period) return;
    update();
}

void KLineChart::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false); // 金融图表不抗锯齿
    drawAll(painter);
}

void KLineChart::recalcAxis() {
    m_axis.chartHeight = height();
    m_axis.chartWidth = width();
    auto klines = m_buffer->klines(m_contract, m_period);
    if (klines.isEmpty()) return;

    int totalCount = klines.size();
    m_startIndex = std::clamp(m_startIndex, 0, std::max(0, totalCount - m_visibleCount));

    // 仅对可视范围内的K线计算价格范围
    int endIdx = std::min(m_startIndex + m_visibleCount, totalCount);
    QVector<KLineData> visible(
        klines.begin() + m_startIndex,
        klines.begin() + endIdx);
    m_axis.calcPriceRange(visible);
}

void KLineChart::drawAll(QPainter& painter) {
    m_painterFn = ChartPainter(&painter);
    recalcAxis();

    auto klines = m_buffer->klines(m_contract, m_period);
    if (klines.isEmpty()) {
        painter.fillRect(rect(), m_bgColor);
        painter.setPen(QColor(120, 120, 140));
        painter.drawText(rect(), Qt::AlignCenter, "等待行情数据...");
        return;
    }

    // 背景
    m_painterFn.drawBackground(QRect(0, 0, m_axis.chartWidth, m_axis.chartHeight), m_bgColor);
    // 成交量区域背景
    int volStartY = m_axis.chartHeight * 3 / 4;
    painter.fillRect(QRect(0, volStartY, m_axis.chartWidth, m_axis.chartHeight - volStartY), m_volumeAreaBg);
    // 网格
    m_painterFn.drawGrid(m_axis, 5, 8);
    // K线
    m_painterFn.drawCandles(klines, m_axis, m_startIndex, m_visibleCount);
    // 成交量
    m_painterFn.drawVolume(klines, m_axis, m_startIndex, m_visibleCount);
    // 价格标签
    m_painterFn.drawPriceLabels(m_axis, 5);
    // 十字光标
    m_crosshair.draw(&painter, rect());
}

void KLineChart::wheelEvent(QWheelEvent* event) {
    bool scrollUp = event->angleDelta().y() > 0;
    if (scrollUp) {
        m_visibleCount = std::max(10, m_visibleCount - 10);  // 放大
    } else {
        m_visibleCount = std::min(500, m_visibleCount + 10); // 缩小
    }
    update();
}

void KLineChart::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        m_dragStart = event->pos();
        m_dragStartIndex = m_startIndex;
        setCursor(Qt::ClosedHandCursor);
    }
}

void KLineChart::mouseMoveEvent(QMouseEvent* event) {
    if (m_isDragging) {
        int dx = event->pos().x() - m_dragStart.x();
        double candleWidth = static_cast<double>(m_axis.chartWidth - m_axis.RIGHT_MARGIN) / m_visibleCount;
        int indexDelta = -static_cast<int>(dx / candleWidth);
        m_startIndex = std::clamp(m_dragStartIndex + indexDelta, 0, m_startIndex);
        update();
    } else {
        m_crosshair.moveTo(event->pos(), rect());
        // 计算十字光标对应的K线数据并发射信号
        int idx = m_axis.pixelXToIndex(event->pos().x(), m_visibleCount, m_startIndex);
        auto klines = m_buffer->klines(m_contract, m_period);
        if (idx >= 0 && idx < klines.size()) {
            const auto& k = klines[idx];
            emit crosshairInfo(QString("O:%1 H:%2 L:%3 C:%4 V:%5")
                .arg(k.open, 0, 'f', 0).arg(k.high, 0, 'f', 0)
                .arg(k.low, 0, 'f', 0).arg(k.close, 0, 'f', 0)
                .arg(k.volume, 0, 'f', 0));
        }
        update();
    }
}

void KLineChart::mouseReleaseEvent(QMouseEvent*) {
    m_isDragging = false;
    m_crosshair.hide();
    setCursor(Qt::ArrowCursor);
    update();
}

void KLineChart::resizeEvent(QResizeEvent*) { update(); }
```

- [ ] **Step 5: 编译构建**

```bash
cd C:/Dev/Claude-Code/pc_futuresmarketanalysis/build
cmake --build .
```
验证：编译成功。

- [ ] **Step 6: Commit**

```bash
git add src/chart/ChartAxis.h src/chart/ChartAxis.cpp \
        src/chart/ChartPainter.h src/chart/ChartPainter.cpp \
        src/chart/Crosshair.h src/chart/Crosshair.cpp \
        src/chart/KLineChart.h src/chart/KLineChart.cpp
git commit -m "feat: implement KLineChart with QPainter rendering, crosshair and zoom/pan"
```

---

### Task 7: 辅助图表组件

**Files:**
- Create: `src/chart/TimeSharingChart.h`, `src/chart/TimeSharingChart.cpp`
- Create: `src/chart/QuotePanel.h`, `src/chart/QuotePanel.cpp`
- Create: `src/chart/ContractList.h`, `src/chart/ContractList.cpp`
- Create: `src/chart/TradeRecord.h`, `src/chart/TradeRecord.cpp`

- [ ] **Step 1: 编写 TimeSharingChart 分时图**

头文件 `src/chart/TimeSharingChart.h`:
```cpp
#pragma once
#include <QWidget>
#include <QVector>
#include <QPointF>
#include "data/TickData.h"

class MarketDataBuffer;

// 分时走势图组件
// 实时绘制当日价格折线和均价线
class TimeSharingChart : public QWidget {
    Q_OBJECT
public:
    explicit TimeSharingChart(MarketDataBuffer* buffer, QWidget* parent = nullptr);
    void setContract(const QString& contract);

public slots:
    void onTickUpdated(const TickData& tick);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    MarketDataBuffer* m_buffer;
    QString m_contract;

    // 分时数据点
    QVector<QPointF> m_pricePoints;  // 价格曲线
    QVector<QPointF> m_avgPoints;    // 均价线
    double m_preSettle = 0.0;        // 昨结算（基准线）
    double m_maxPrice = 0.0;
    double m_minPrice = 999999.0;
    double m_totalVolume = 0.0;
    double m_totalAmount = 0.0;

    QColor m_bgColor{22, 22, 30};
    QColor m_priceColor{255, 255, 255};    // 白色价格线
    QColor m_avgColor{255, 215, 0};         // 黄色均价线
    QColor m_volumeColor{34, 197, 94, 50};  // 半透明绿成交量
};
```

实现 `src/chart/TimeSharingChart.cpp`:
```cpp
#include "TimeSharingChart.h"
#include "data/MarketDataBuffer.h"
#include <QPainter>
#include <QDateTime>

TimeSharingChart::TimeSharingChart(MarketDataBuffer* buffer, QWidget* parent)
    : QWidget(parent), m_buffer(buffer) {
    setMinimumSize(400, 200);
}

void TimeSharingChart::setContract(const QString& contract) {
    m_contract = contract;
    m_pricePoints.clear();
    m_avgPoints.clear();
    update();
}

void TimeSharingChart::onTickUpdated(const TickData& tick) {
    if (tick.contract != m_contract) return;
    m_preSettle = tick.preSettle;
    m_totalVolume += tick.volume;
    m_totalAmount += tick.lastPrice * tick.volume;
    double avgPrice = m_totalAmount / std::max(1.0, m_totalVolume);

    double x = static_cast<double>(m_pricePoints.size());
    m_pricePoints.append(QPointF(x, tick.lastPrice));
    m_avgPoints.append(QPointF(x, avgPrice));

    m_maxPrice = std::max(m_maxPrice, tick.lastPrice);
    m_minPrice = std::min(m_minPrice, tick.lastPrice);
    update();
}

void TimeSharingChart::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), m_bgColor);

    if (m_pricePoints.isEmpty()) {
        painter.setPen(QColor(120, 120, 140));
        painter.drawText(rect(), Qt::AlignCenter, "等待分时数据...");
        return;
    }

    int w = width();
    int h = height();
    int margin = 40;
    double priceRange = std::max(m_maxPrice - m_minPrice, 1.0);

    // 坐标映射 lambda
    auto priceToY = [&](double p) {
        return h - margin - (p - m_minPrice) / priceRange * (h - margin * 2);
    };
    auto idxToX = [&](int i) {
        if (m_pricePoints.size() <= 1) return w / 2.0;
        return margin + static_cast<double>(i) / (m_pricePoints.size() - 1) * (w - margin * 2);
    };

    // 绘制昨结算基准线
    if (m_preSettle > 0) {
        int baselineY = static_cast<int>(priceToY(m_preSettle));
        painter.setPen(QPen(QColor(255, 255, 255, 40), 1, Qt::DotLine));
        painter.drawLine(margin, baselineY, w - margin, baselineY);
    }

    // 绘制价格折线
    painter.setPen(QPen(m_priceColor, 1));
    for (int i = 1; i < m_pricePoints.size(); ++i) {
        painter.drawLine(QPointF(idxToX(i-1), priceToY(m_pricePoints[i-1].y())),
                         QPointF(idxToX(i),   priceToY(m_pricePoints[i].y())));
    }

    // 绘制均价线
    painter.setPen(QPen(m_avgColor, 1, Qt::DashLine));
    for (int i = 1; i < m_avgPoints.size(); ++i) {
        painter.drawLine(QPointF(idxToX(i-1), priceToY(m_avgPoints[i-1].y())),
                         QPointF(idxToX(i),   priceToY(m_avgPoints[i].y())));
    }
}
```

- [ ] **Step 2: 编写 QuotePanel 盘口面板**

头文件 `src/chart/QuotePanel.h`:
```cpp
#pragma once
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include "data/TickData.h"

// 五档盘口展示面板
class QuotePanel : public QWidget {
    Q_OBJECT
public:
    explicit QuotePanel(QWidget* parent = nullptr);
    void updateQuote(const TickData& tick);

private:
    void setupUI();
    QLabel* m_priceLabel;       // 最新价格（大字）
    QLabel* m_changeLabel;      // 涨跌幅
    QLabel* m_askLabels[5];     // 卖5~卖1
    QLabel* m_bidLabels[5];     // 买1~买5
    QLabel* m_volumeLabel;      // 成交量
    QLabel* m_oiLabel;          // 持仓量
};
```

实现 `src/chart/QuotePanel.cpp`:
```cpp
#include "QuotePanel.h"
#include <QFont>

QuotePanel::QuotePanel(QWidget* parent) : QWidget(parent) { setupUI(); }

void QuotePanel::setupUI() {
    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(2);
    layout->setContentsMargins(4, 4, 4, 4);

    // 最新价格标签
    m_priceLabel = new QLabel("----");
    m_priceLabel->setAlignment(Qt::AlignCenter);
    QFont priceFont("Arial", 28, QFont::Bold);
    m_priceLabel->setFont(priceFont);
    m_priceLabel->setStyleSheet("color: #ffffff;");
    layout->addWidget(m_priceLabel);

    // 涨跌幅
    m_changeLabel = new QLabel("0.00%");
    m_changeLabel->setAlignment(Qt::AlignCenter);
    m_changeLabel->setStyleSheet("color: #888888; font-size: 14px;");
    layout->addWidget(m_changeLabel);

    // 卖盘（红色）
    for (int i = 0; i < 5; ++i) {
        m_askLabels[i] = new QLabel(QString("卖%1 ----").arg(5 - i));
        m_askLabels[i]->setStyleSheet("color: #ef4444; font-size: 11px;");
        layout->addWidget(m_askLabels[i]);
    }

    // 分隔线
    auto* sep = new QLabel();
    sep->setFixedHeight(1);
    sep->setStyleSheet("background: #333;");
    layout->addWidget(sep);

    // 买盘（绿色）
    for (int i = 0; i < 5; ++i) {
        m_bidLabels[i] = new QLabel(QString("买%1 ----").arg(i + 1));
        m_bidLabels[i]->setStyleSheet("color: #22c55e; font-size: 11px;");
        layout->addWidget(m_bidLabels[i]);
    }

    // 成交量、持仓量
    m_volumeLabel = new QLabel("量: --");
    m_volumeLabel->setStyleSheet("color: #888; font-size: 11px;");
    layout->addWidget(m_volumeLabel);

    m_oiLabel = new QLabel("仓: --");
    m_oiLabel->setStyleSheet("color: #888; font-size: 11px;");
    layout->addWidget(m_oiLabel);

    setStyleSheet("background: #1a1a24;");
}

void QuotePanel::updateQuote(const TickData& tick) {
    m_priceLabel->setText(QString::number(tick.lastPrice, 'f', 0));
    double changeRate = (tick.lastPrice - tick.preSettle) / tick.preSettle * 100.0;
    QString changeStr = QString("%1%2%").arg(changeRate >= 0 ? "+" : "").arg(changeRate, 0, 'f', 2);
    m_changeLabel->setText(changeStr);
    m_changeLabel->setStyleSheet(
        (changeRate >= 0 ? "color: #ef4444;" : "color: #22c55e;") + QString(" font-size: 14px;"));

    for (int i = 0; i < 5; ++i) {
        m_askLabels[i]->setText(QString("卖%1 %2 %3")
            .arg(5 - i).arg(tick.askPrice[i], 0, 'f', 0).arg(static_cast<int>(tick.askVolume[i])));
        m_bidLabels[i]->setText(QString("买%1 %2 %3")
            .arg(i + 1).arg(tick.bidPrice[i], 0, 'f', 0).arg(static_cast<int>(tick.bidVolume[i])));
    }
    m_volumeLabel->setText(QString("量: %1").arg(static_cast<int>(tick.volume)));
    m_oiLabel->setText(QString("仓: %1").arg(static_cast<int>(tick.openInterest)));
}
```

- [ ] **Step 3: 编写 ContractList 合约列表**

头文件 `src/chart/ContractList.h`:
```cpp
#pragma once
#include <QWidget>
#include <QTreeWidget>
#include <QMap>

// 自选合约列表组件
class ContractList : public QWidget {
    Q_OBJECT
public:
    explicit ContractList(QWidget* parent = nullptr);
    void addContract(const QString& contract, const QString& name = "");
    void updatePrice(const QString& contract, double price, double changeRate);

signals:
    void contractClicked(const QString& contract);

private:
    QTreeWidget* m_tree;
    QMap<QString, QTreeWidgetItem*> m_items;
};
```

实现 `src/chart/ContractList.cpp`:
```cpp
#include "ContractList.h"
#include <QVBoxLayout>
#include <QHeaderView>

ContractList::ContractList(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_tree = new QTreeWidget();
    m_tree->setHeaderLabels({"合约", "最新价", "涨跌幅"});
    m_tree->setColumnWidth(0, 80);
    m_tree->setColumnWidth(1, 70);
    m_tree->setColumnWidth(2, 65);
    m_tree->setRootIsDecorated(false);
    m_tree->setStyleSheet(
        "QTreeWidget { background: #161620; color: #ccc; border: none; }"
        "QTreeWidget::item { height: 24px; }"
        "QHeaderView::section { background: #1e1e2a; color: #888; border: none; padding: 4px; }"
    );

    connect(m_tree, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem* item) {
        emit contractClicked(item->text(0));
    });

    layout->addWidget(m_tree);
}

void ContractList::addContract(const QString& contract, const QString& name) {
    auto* item = new QTreeWidgetItem();
    item->setText(0, contract);
    item->setToolTip(0, name.isEmpty() ? contract : name);
    item->setText(1, "----");
    item->setText(2, "--");
    m_tree->addTopLevelItem(item);
    m_items[contract] = item;
}

void ContractList::updatePrice(const QString& contract, double price, double changeRate) {
    if (!m_items.contains(contract)) return;
    auto* item = m_items[contract];
    item->setText(1, QString::number(price, 'f', 0));
    item->setText(2, QString("%1%").arg(changeRate, 0, 'f', 2, QChar('+')));
    item->setTextColor(1, changeRate >= 0 ? QColor(239, 68, 68) : QColor(34, 197, 94));
    item->setTextColor(2, changeRate >= 0 ? QColor(239, 68, 68) : QColor(34, 197, 94));
}
```

- [ ] **Step 4: 编写 TradeRecord 逐笔成交**

头文件 `src/chart/TradeRecord.h`:
```cpp
#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QVector>
#include "data/TickData.h"

// 逐笔成交记录组件（滚动实时刷新）
class TradeRecord : public QWidget {
    Q_OBJECT
public:
    explicit TradeRecord(QWidget* parent = nullptr);
    void addRecord(const TickData& tick, int volume, bool isBuy);

private:
    QTableWidget* m_table;
    static constexpr int MAX_ROWS = 50;
};
```

实现 `src/chart/TradeRecord.cpp`:
```cpp
#include "TradeRecord.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QDateTime>

TradeRecord::TradeRecord(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_table = new QTableWidget(0, 3);
    m_table->setHorizontalHeaderLabels({"时间", "价格", "现手"});
    m_table->setColumnWidth(0, 60);
    m_table->setColumnWidth(1, 60);
    m_table->setColumnWidth(2, 50);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionMode(QAbstractItemView::NoSelection);
    m_table->setStyleSheet(
        "QTableWidget { background: #161620; color: #ccc; border: none; }"
        "QHeaderView::section { background: #1e1e2a; color: #888; border: none; padding: 2px; font-size: 10px; }"
    );
    layout->addWidget(m_table);
}

void TradeRecord::addRecord(const TickData& tick, int volume, bool isBuy) {
    m_table->insertRow(0);
    auto timeStr = QDateTime::fromMSecsSinceEpoch(tick.timestamp).toString("HH:mm:ss");
    m_table->setItem(0, 0, new QTableWidgetItem(timeStr));
    auto* priceItem = new QTableWidgetItem(QString::number(tick.lastPrice, 'f', 0));
    priceItem->setForeground(isBuy ? QColor(239, 68, 68) : QColor(34, 197, 94));
    m_table->setItem(0, 1, priceItem);
    m_table->setItem(0, 2, new QTableWidgetItem(QString::number(volume)));

    // 限制最多50行
    while (m_table->rowCount() > MAX_ROWS) m_table->removeRow(m_table->rowCount() - 1);
}
```

- [ ] **Step 5: 编译构建**

```bash
cd C:/Dev/Claude-Code/pc_futuresmarketanalysis/build
cmake --build .
```
验证：编译成功。

- [ ] **Step 6: Commit**

```bash
git add src/chart/TimeSharingChart.h src/chart/TimeSharingChart.cpp \
        src/chart/QuotePanel.h src/chart/QuotePanel.cpp \
        src/chart/ContractList.h src/chart/ContractList.cpp \
        src/chart/TradeRecord.h src/chart/TradeRecord.cpp
git commit -m "feat: add TimeSharingChart, QuotePanel, ContractList and TradeRecord components"
```

---

### Task 8: 主窗口集成

**Files:**
- Modify: `src/app/Application.h` (添加 initMainWindow 实现)
- Create: `src/app/MainWindow.h`, `src/app/MainWindow.cpp`

- [ ] **Step 1: 编写 MainWindow**

头文件 `src/app/MainWindow.h`:
```cpp
#pragma once
#include <QMainWindow>
#include <QSplitter>

class KLineChart;
class TimeSharingChart;
class QuotePanel;
class ContractList;
class TradeRecord;
class MarketDataBuffer;
class SimDataProvider;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(MarketDataBuffer* buffer, SimDataProvider* provider,
                        QWidget* parent = nullptr);

private:
    void setupUI();
    void setupConnections();
    void initDefaultContracts();

    MarketDataBuffer* m_buffer;
    SimDataProvider* m_dataProvider;

    // UI组件
    ContractList*     m_contractList;
    KLineChart*       m_klineChart;
    TimeSharingChart* m_timeChart;
    QuotePanel*       m_quotePanel;
    TradeRecord*      m_tradeRecord;
};
```

实现 `src/app/MainWindow.cpp`:
```cpp
#include "MainWindow.h"
#include "chart/KLineChart.h"
#include "chart/TimeSharingChart.h"
#include "chart/QuotePanel.h"
#include "chart/ContractList.h"
#include "chart/TradeRecord.h"
#include "data/MarketDataBuffer.h"
#include "data/SimDataProvider.h"
#include "core/EventBus.h"
#include <QTabWidget>
#include <QHBoxLayout>

MainWindow::MainWindow(MarketDataBuffer* buffer, SimDataProvider* provider, QWidget* parent)
    : QMainWindow(parent), m_buffer(buffer), m_dataProvider(provider) {
    setWindowTitle("期货行情PC软件 - Phase 1");
    resize(1400, 900);
    setStyleSheet("QMainWindow { background: #12121a; }");
    setupUI();
    setupConnections();
    initDefaultContracts();
}

void MainWindow::setupUI() {
    auto* centralWidget = new QWidget();
    auto* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(4);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    // 左侧：自选合约列表
    m_contractList = new ContractList();
    m_contractList->setFixedWidth(200);
    mainLayout->addWidget(m_contractList);

    // 中间：图表区域
    auto* chartArea = new QWidget();
    auto* chartLayout = new QVBoxLayout(chartArea);
    chartLayout->setSpacing(4);
    chartLayout->setContentsMargins(0, 0, 0, 0);

    m_klineChart = new KLineChart(m_buffer);
    m_klineChart->setMinimumHeight(350);
    chartLayout->addWidget(m_klineChart, 3);

    // 分时图/盘口 Tab 切换
    auto* tabWidget = new QTabWidget();
    tabWidget->setStyleSheet(
        "QTabWidget::pane { background: #1a1a24; border: 1px solid #333; }"
        "QTabBar::tab { background: #12121a; color: #888; padding: 6px 16px; }"
        "QTabBar::tab:selected { background: #1a1a24; color: #fff; }"
    );
    m_timeChart = new TimeSharingChart(m_buffer);
    tabWidget->addTab(m_timeChart, "分时图");
    tabWidget->addTab(new QWidget(), "盘口");  // 占位，Phase 2 添加盘口Tab
    chartLayout->addWidget(tabWidget, 2);

    mainLayout->addWidget(chartArea, 1);

    // 右侧：盘口 + 逐笔成交
    auto* rightPanel = new QWidget();
    auto* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setSpacing(4);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    m_quotePanel = new QuotePanel();
    m_quotePanel->setMinimumWidth(220);
    rightLayout->addWidget(m_quotePanel, 1);

    m_tradeRecord = new TradeRecord();
    rightLayout->addWidget(m_tradeRecord, 1);

    rightPanel->setFixedWidth(280);
    mainLayout->addWidget(rightPanel);

    setCentralWidget(centralWidget);
}

void MainWindow::setupConnections() {
    // 合约选择 → 更新所有组件
    connect(m_contractList, &ContractList::contractClicked, this, [this](const QString& contract) {
        m_klineChart->setContract(contract);
        m_timeChart->setContract(contract);
    });

    // Tick数据 → 盘口和逐笔成交更新
    auto& bus = EventBus::instance();
    connect(&bus, &EventBus::tickUpdated, this, [this](const QString& contract, double price, double volume) {
        auto tick = m_buffer->latestTick(contract);
        m_quotePanel->updateQuote(tick);
        m_contractList->updatePrice(contract, tick.lastPrice,
            (tick.lastPrice - tick.preSettle) / tick.preSettle * 100.0);
        // 模拟逐笔成交
        bool isBuy = (price > tick.preSettle);
        m_tradeRecord->addRecord(tick, static_cast<int>(volume), isBuy);
    });
}

void MainWindow::initDefaultContracts() {
    // 添加预设自选合约
    struct ContractInfo { QString code; QString name; };
    QVector<ContractInfo> defaults = {
        {"rb2510", "螺纹钢主连"}, {"IF2506", "沪深300主力"},
        {"sc2507", "原油主力"},   {"au2508", "黄金主力"},
        {"m2509",  "豆粕主力"},   {"TA510",  "PTA主力"},
        {"FG508",  "玻璃主力"},   {"SA509",  "纯碱主力"},
    };
    for (const auto& c : defaults) {
        m_contractList->addContract(c.code, c.name);
        m_dataProvider->subscribe(c.code);
        m_buffer->addContract(c.code);
    }
    // 默认选中第一个合约
    m_klineChart->setContract("rb2510");
    m_timeChart->setContract("rb2510");
}
```

- [ ] **Step 2: 更新 Application::initMainWindow**

修改 `src/app/Application.cpp`，添加主窗口创建：
```cpp
// 在 Application.cpp 顶部添加 include
#include <QMainWindow>
#include "app/MainWindow.h"
#include "data/SimDataProvider.h"
#include "data/MarketDataBuffer.h"
#include "data/KLineAggregator.h"

// 添加成员变量（修改 Application.h 头文件）
// SimDataProvider* m_dataProvider;
// MarketDataBuffer* m_buffer;

// initMainWindow 实现：
void Application::initMainWindow() {
    auto* window = new MainWindow(m_buffer, m_dataProvider);
    window->show();
}

// 更新 initDataLayer：
void Application::initDataLayer() {
    m_dataProvider = new SimDataProvider(this);
    m_buffer = new MarketDataBuffer(2000, this);
    auto* aggregator = new KLineAggregator(m_buffer, this);

    QObject::connect(m_dataProvider, &SimDataProvider::tickReceived,
                     m_buffer, &MarketDataBuffer::onTick);
    QObject::connect(m_buffer, &MarketDataBuffer::tickUpdated,
                     aggregator, &KLineAggregator::onTick);
    m_dataProvider->start();
    spdlog::debug("数据层初始化完成");
}
```

- [ ] **Step 3: 编译构建并运行**

```bash
cd C:/Dev/Claude-Code/pc_futuresmarketanalysis/build
cmake --build .
# Windows: ./Debug/FuturesMarketPC.exe
```
验证：程序启动，显示三栏布局，模拟数据驱动K线图、分时图、盘口实时刷新。

- [ ] **Step 4: Commit**

```bash
git add src/app/MainWindow.h src/app/MainWindow.cpp src/app/Application.h src/app/Application.cpp
git commit -m "feat: integrate MainWindow with full 3-column layout and live simulated data"
```

---

### Task 9: SQLite 数据库封装

**Files:**
- Create: `src/db/Database.h`, `src/db/Database.cpp`

- [ ] **Step 1: 编写 Database 封装**

头文件 `src/db/Database.h`:
```cpp
#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <QVector>
#include "data/KLineData.h"

// SQLite 数据库封装
// Phase 1: 创建表结构，Phase 2: 历史K线存取
class Database : public QObject {
    Q_OBJECT
public:
    explicit Database(const QString& dbPath, QObject* parent = nullptr);
    ~Database();

    bool init();

    // K线存取（Phase 2 使用）
    void saveKLine(const KLineData& kline);
    QVector<KLineData> loadKLines(const QString& contract, KLineType type,
                                   int64_t from, int64_t to);

private:
    void createTables();
    QSqlDatabase m_db;
    QString m_dbPath;
};
```

实现 `src/db/Database.cpp`:
```cpp
#include "Database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <spdlog/spdlog.h>

Database::Database(const QString& dbPath, QObject* parent)
    : QObject(parent), m_dbPath(dbPath) {}

Database::~Database() { if (m_db.isOpen()) m_db.close(); }

bool Database::init() {
    m_db = QSqlDatabase::addDatabase("QSQLITE", "main_connection");
    m_db.setDatabaseName(m_dbPath);
    if (!m_db.open()) {
        spdlog::error("数据库打开失败: {}", m_db.lastError().text().toStdString());
        return false;
    }
    createTables();
    spdlog::info("数据库初始化完成: {}", m_dbPath.toStdString());
    return true;
}

void Database::createTables() {
    QSqlQuery query(m_db);
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS klines (
            contract    TEXT NOT NULL,
            type        TEXT NOT NULL,
            timestamp   INTEGER NOT NULL,
            open        REAL,
            high        REAL,
            low         REAL,
            close       REAL,
            volume      REAL,
            open_interest REAL,
            PRIMARY KEY (contract, type, timestamp)
        )
    )");
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS watchlist (
            contract TEXT PRIMARY KEY,
            name     TEXT,
            added_at INTEGER DEFAULT (strftime('%s','now'))
        )
    )");
}

void Database::saveKLine(const KLineData& kline) {
    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT OR REPLACE INTO klines
        (contract, type, timestamp, open, high, low, close, volume, open_interest)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");
    query.addBindValue(kline.contract);
    query.addBindValue(klineTypeToString(kline.type));
    query.addBindValue(kline.timestamp);
    query.addBindValue(kline.open);
    query.addBindValue(kline.high);
    query.addBindValue(kline.low);
    query.addBindValue(kline.close);
    query.addBindValue(kline.volume);
    query.addBindValue(kline.openInterest);
    if (!query.exec()) {
        spdlog::warn("K线保存失败: {}", query.lastError().text().toStdString());
    }
}

QVector<KLineData> Database::loadKLines(const QString& contract, KLineType type,
                                          int64_t from, int64_t to) {
    QVector<KLineData> result;
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT * FROM klines WHERE contract = ? AND type = ?
        AND timestamp >= ? AND timestamp <= ? ORDER BY timestamp ASC
    )");
    query.addBindValue(contract);
    query.addBindValue(klineTypeToString(type));
    query.addBindValue(from);
    query.addBindValue(to);

    if (query.exec()) {
        while (query.next()) {
            KLineData k;
            k.contract = query.value(0).toString();
            k.timestamp = query.value(2).toLongLong();
            k.open = query.value(3).toDouble();
            k.high = query.value(4).toDouble();
            k.low = query.value(5).toDouble();
            k.close = query.value(6).toDouble();
            k.volume = query.value(7).toDouble();
            k.openInterest = query.value(8).toDouble();
            k.type = type;
            k.isComplete = true;
            result.append(k);
        }
    }
    return result;
}
```

- [ ] **Step 2: 编译构建**

```bash
cd C:/Dev/Claude-Code/pc_futuresmarketanalysis/build
cmake --build .
```
验证：编译成功。

- [ ] **Step 3: Commit**

```bash
git add src/db/
git commit -m "feat: add SQLite database wrapper with klines and watchlist tables"
```

---

## 验证方案

### 编译验证
```bash
cd C:/Dev/Claude-Code/pc_futuresmarketanalysis/build
cmake --build .
```
预期：0错误，0警告。

### 运行时验证
1. 启动程序 → 主窗口正常显示，三栏布局正确
2. 左侧合约列表显示8个预设合约
3. 点击合约 → K线图和分时图切换到对应合约
4. 模拟数据250ms刷新一次 → K线图实时更新最后一根K线
5. 右侧盘口面板显示实时五档价格
6. 鼠标滚轮 → K线图缩放（可见K线数量变化）
7. 鼠标拖拽 → K线图平移
8. 鼠标悬停K线图 → 十字光标显示（Hover信息）
9. 右侧逐笔成交面板实时滚动画新记录
10. 底部状态栏显示连接状态和当前周期

### 测试检查项
- [ ] 多合约同时订阅，数据不混淆
- [ ] K线周期切换（M1/M5/M15/M30/H1/D）数据正确
- [ ] 缩放至最小/最大极限不崩溃
- [ ] 合约切换时图表正确清空并重新加载
- [ ] 程序关闭正常退出，无内存泄漏
