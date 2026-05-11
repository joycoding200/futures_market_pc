# 期货行情PC软件

面向国内期货市场的桌面行情分析软件，提供实时行情展示、技术指标分析和 CTP 行情接入。

## 核心价值

让期货交易者能够在桌面端**实时查看行情数据**，通过 K 线图和技术指标**辅助交易决策**，支持**模拟和 CTP 双数据源**切换。

## 技术栈

Qt 6.x / C++17 / CMake + vcpkg / SQLite / spdlog / CTP v6.6+

## 架构概述

```
Application → EventBus + Config (core/)
            → MarketDataProvider → SimDataProvider / CTPDataProvider (data/)
            → MarketDataBuffer → KLineAggregator (data/)
            → KLineChart / TimeSharingChart / QuotePanel (chart/)
            → MA / MACD / KDJ / RSI (indicator/)
            → SettingsDialog / ContractManager / AboutDialog (system/)
```

## 阶段规划

| 阶段 | 名称 | 状态 |
|------|------|------|
| Phase 1 | 行情展示 | ✅ 完成 |
| Phase 2 | 技术指标 | ✅ 完成 |
| Phase 3 | CTP 行情接入 | ✅ 完成 |
| Phase 4 | 系统管理 | ✅ 完成 |
| Phase 5 | 测试体系 | 📋 待规划 |
