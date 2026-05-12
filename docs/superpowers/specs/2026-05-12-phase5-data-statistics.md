# Phase 5: 数据统计与筛选 — 设计文档

> 日期：2026-05-12 | 状态：已确认

## 1. 概述

补充原始需求中的"简单数据统计与筛选功能"。为期货行情软件添加实时数据统计面板和多维度筛选能力，帮助用户快速识别市场热点和异常交易。

## 2. 功能设计

### 2.1 数据统计面板

在主界面新增可切换的**统计面板**（位于右侧面板 Tab 区域或底部状态区），提供：

| 统计项 | 说明 | 计算方式 |
|--------|------|----------|
| 涨跌幅排行 | 当前订阅合约按涨跌幅排序 | `(lastPrice - preSettle) / preSettle * 100%` |
| 成交量排名 | 按当日累计成交量排序 | 取 `TickData.volume` 累加 |
| 持仓量变化 | 持仓量较昨日的增减 | `openInterest - preOpenInterest` |
| 振幅统计 | 当日振幅 = (最高-最低)/昨结算 | `(highPrice - lowPrice) / preSettle * 100%` |

### 2.2 筛选功能

在合约列表上方添加筛选栏：

- **关键词搜索**：QLineEdit 输入框，按合约代码/名称实时过滤
- **交易所筛选**：QComboBox 下拉选择（全部/CFFEX/SHFE/DCE/CZCE/INE）
- **涨跌方向**：仅显示上涨/下跌/平盘合约
- **排序方式**：点击列表表头排序（按代码、价格、涨跌幅）

### 2.3 成交量分布统计

在逐笔成交区域新增成交量分布摘要：

- 大单/中单/小单交易笔数统计（实时滚动计数）
- 简单的饼图或条形指示（可用 QProgressBar 或 QPainter 自绘）

## 3. 界面集成

### 3.1 右侧面板重构

```
右侧面板 (280px)
├── QuotePanel (五档盘口)        — 固定顶部
├── TradeRecord (逐笔成交)       — 可折叠
├── VolumeDistribution (成交量分布) — 新增，紧凑型
└── StatsPanel (统计面板)        — 新增，Tab 切换
```

### 3.2 合约列表增强

ContractList 顶部新增 QLineEdit 搜索框，表头支持点击排序。

## 4. 数据结构

```cpp
// 合约统计快照（按合约聚合）
struct ContractStats {
    QString  contract;
    QString  name;
    double   lastPrice;
    double   changeRate;      // 涨跌幅 %
    double   volume;           // 累计成交量
    double   openInterest;     // 最新持仓
    double   amplitude;        // 振幅 %
    double   highPrice;
    double   lowPrice;
    QString  exchange;         // 交易所前缀
};

// 成交量分布统计
struct VolumeDistribution {
    int largeCount  = 0;      // ≥100 手
    int mediumCount = 0;      // 30-99 手
    int smallCount  = 0;      // <30 手
};
```

## 5. 文件结构

```
src/stats/
├── ContractStats.h/cpp       # 合约统计数据模型
├── StatsPanel.h/cpp          # 统计面板 UI
├── StatsCalculator.h/cpp     # 统计计算引擎
└── VolumeDistribution.h/cpp  # 成交量分布组件（可选合并到 TradeRecord）

CHANGES:
- src/chart/ContractList.h/cpp — 搜索栏 + 表头排序
- src/app/MainWindow.h/cpp    — 集成统计面板
- CMakeLists.txt              — 添加 stats/ 源文件
```

## 6. 性能考虑

- 统计数据基于内存中的 TickData 计算，无需额外数据源
- 统计刷新频率跟随行情刷新（250ms），使用增量更新策略
- 排序仅在用户切换排序方式时触发，默认按添加顺序
- 高亮涨跌色与行情盘口保持一致（红涨绿跌）
