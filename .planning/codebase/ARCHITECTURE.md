# 架构分析

## 整体架构

```
┌──────────────────────────────────────┐
│           Application (app/)         │
│  启动顺序: initLogging → EventBus    │
│           → DataLayer → MainWindow    │
│  依赖注入: Provider + Buffer → UI     │
└────────┬─────────────────────────────┘
         │
    ┌────┴────┐
    │         │
┌───▼──┐  ┌──▼────────────────────┐
│ core/│  │       data/             │
│EventBus│  │ MarketDataProvider (抽象) │
│Config │  │  ├─ SimDataProvider      │
└───┬───┘  │  ├─ CTPDataProvider      │
    │      │  ├─ MarketDataBuffer      │
    │      │  └─ KLineAggregator       │
    │      └────────┬──────────────────┘
    │               │ Qt signals/slots
    │   ┌───────────┼───────────┐
    │   │           │           │
┌───▼───▼──┐ ┌─────▼──────┐ ┌─▼────────┐
│  chart/  │ │ indicator/  │ │ system/  │
│KLineChart│ │ IndicatorBase│ │SettingsDlg│
│TimeChart │ │ MA/MACD/KDJ │ │ContractMgr│
│QuotePanel│ │   /RSI      │ │AboutDialog│
│Crosshair │ │             │ │           │
│ContractLst│ │             │ │           │
│TradeRecrd│ │             │ │           │
└──────────┘ └─────────────┘ └──────────┘
```

## 核心设计模式

### 1. EventBus（全局事件总线）
- 单例模式 (Meyer's Singleton)
- `EventBus::instance()` 全局访问
- 用于组件间解耦通信（Tick 更新、合约选择等）

### 2. 策略模式（数据源）
- `MarketDataProvider` 抽象接口
- `SimDataProvider` / `CTPDataProvider` 可互换实现
- 通过 `providerName()` 运行时识别

### 3. 观察者模式（Qt 信号/槽）
- `MarketDataBuffer` 发出 `tickUpdated` / `klineUpdated`
- UI 组件（KLineChart, TimeSharingChart, QuotePanel）订阅

### 4. 模板方法模式（技术指标）
- `IndicatorBase` 定义计算流程接口
- 子类实现 `calculate()`, `valueAt()`, `lineCount()` 等

## 数据流

```
SimDataProvider::onTimerTick()
  → generateTick() → emit tickReceived(TickData)
  → MarketDataBuffer::onTick()
  → KLineAggregator::onTick() → 聚合并 K 线
  → emit tickUpdated / klineUpdated
  → UI 组件更新
```

## 构建配置

- CTP 支持通过 `-DWITH_CTP=ON` 条件编译
- 默认 OFF，仅模拟数据源编译
- CTP 头文件和库位于 `third_party/ctp/`
