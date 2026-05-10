# 期货行情PC软件 — 架构设计文档

> 日期：2026-05-10 | 状态：已确认

## 1. 项目概述

面向国内期货市场的**桌面行情分析与交易软件**，类似文华财经/博易大师。核心技术路线：**Qt 6.x / C++17**，模块化分层架构，先行情后交易的渐进式交付。

**目标市场**：国内期货（上海期货交易所、大连商品交易所、郑州商品交易所、中国金融期货交易所、上海国际能源交易中心），对接 CTP 标准接口。

## 2. 架构总体设计

### 2.1 模块化分层架构

```
┌─────────────────────────────────────────────┐
│              展示层 (Presentation)            │
│  K线图 / 分时图 / 盘口 / 合约列表 / 交易面板    │
├─────────────────────────────────────────────┤
│              业务层 (Business)                │
│  行情分析引擎 / 技术指标计算 / 交易引擎          │
├─────────────────────────────────────────────┤
│              数据层 (Data)                    │
│  MarketDataAdapter / 数据缓存 / SQLite持久化   │
├─────────────────────────────────────────────┤
│              基础层 (Core)                    │
│  EventBus / Config / 窗口管理 / 日志           │
└─────────────────────────────────────────────┘
```

### 2.2 模块间通信

- **同层通信**：Qt Signal/Slot 机制
- **跨层通信**：数据层→展示层通过 EventBus 分发，展示层→数据层通过抽象接口调用
- **跨线程**：CTP 数据接收在独立线程，通过信号槽线程安全投递

## 3. 技术选型

| 类别 | 选型 | 说明 |
|------|------|------|
| 框架 | Qt 6.x / C++17 | 原生性能，信号槽天然适配实时行情 |
| 图表 | Qt QPainter 自绘 + QCustomPlot | K线/分时自绘，指标副图用QCustomPlot |
| 行情协议 | CTP v6.6+ / Esunny / HTTP REST | 多数源适配，抽象接口统一 |
| 构建 | CMake + vcpkg | 跨平台构建，依赖管理 |
| 存储 | SQLite | 历史K线、自选合约、配置 |
| 日志 | spdlog | 高性能C++日志库 |

## 4. 数据层设计

### 4.1 数据源适配器模式

```
                  MarketDataProvider (抽象接口)
                  ├── subscribe(contract, type)
                  ├── unsubscribe(contract)
                  ├── signal onTick(TickData)
                  └── signal onKLine(KLineData)
                         │
        ┌────────────────┼────────────────┐
        ▼                ▼                ▼
  CTPProvider      HttpProvider      SimProvider
 (TCP长连接)       (REST轮询)        (本地模拟)
```

### 4.2 核心数据结构

```cpp
struct TickData {
    string   contract;
    double   lastPrice, volume, openInterest;
    double   bidPrice[5], bidVolume[5];
    double   askPrice[5], askVolume[5];
    int64_t  timestamp;      // 毫秒时间戳
};

struct KLineData {
    string   contract;
    double   open, high, low, close, volume, openInterest;
    int64_t  timestamp;
    enum     KLineType { M1, M5, M15, M30, H1, D, W, M };
};
```

### 4.3 数据流

```
CTP/HTTP/Sim → MarketDataAdapter → 归一化TickData/KLineData
    → Memory RingBuffer (实时缓存)
    → EventBus (信号广播)
    → KLineAggregator (Tick→K线聚合)
    → SQLite (历史持久化)
    → UI组件 (按需订阅)
```

## 5. 界面布局

### 5.1 主界面（三栏式经典布局）

- **左侧 200px**：自选合约列表，涨跌红绿色标识
- **中央**：K线图(上) + 指标副图(中) + 分时/盘口Tab(下)
- **右侧 280px**：五档盘口(上) + 逐笔成交(下)

### 5.2 核心组件清单

| 组件 | 说明 | 渲染方式 |
|------|------|----------|
| KLineChart | K线图/美国线/收盘线 | QPainter自绘 |
| TimeSharingChart | 分时走势图 | QPainter自绘 |
| QuotePanel | 五档盘口 | Qt原生控件 |
| ContractList | 自选合约 | QTreeView |
| TradeRecord | 逐笔成交 | QTableView |

## 6. K线图表组件设计

### 6.1 组件架构

```
KLineChart
├── ChartAxis      坐标轴（价格/时间/成交量）
├── ChartPainter   K线绘制 + 成交量柱 + MA叠加
├── Crosshair      十字光标 + 浮窗信息
├── DataManager    可视范围计算 + 数据缓存
└── Interaction    缩放/滚轮/拖拽/周期切换
```

### 6.2 渲染流程

1. EventBus 接收新 Tick → 更新最后一根K线数据
2. 判断最新K线是否超出可视范围
3. 计算可视范围内K线数量 → 数据索引切片
4. 遍历可视K线 → QPainter 逐根绘制（含成交量柱）
5. 叠加 MA 均线 → 像素级折线连接
6. QWidget::paintEvent() 触发

### 6.3 性能优化

- 可视裁剪：仅计算和绘制屏幕内可见的K线
- 增量更新：实时行情只重绘最后一根K线区域
- 双缓冲：消除画面闪烁
- 降采样：缩放时K线过密自动合并

### 6.4 交互功能

- **缩放**：鼠标滚轮调整 visibleCount
- **平移**：鼠标拖拽调整 startIndex
- **十字光标**：悬停显示 O/H/L/C/V 浮窗
- **周期切换**：1m/5m/15m/30m/1H/日/周/月

## 7. 项目目录结构

```
pc_futuresmarketanalysis/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── app/Application.h/cpp
│   ├── core/EventBus.h/cpp, Config.h/cpp
│   ├── data/
│   │   ├── MarketDataProvider.h, CTPDataProvider.h/cpp
│   │   ├── HttpDataProvider.h/cpp, SimDataProvider.h/cpp
│   │   ├── MarketDataBuffer.h/cpp, TickData.h, KLineData.h
│   │   └── KLineAggregator.h/cpp
│   ├── chart/
│   │   ├── KLineChart.h/cpp, ChartAxis.h/cpp
│   │   ├── ChartPainter.h/cpp, Crosshair.h/cpp
│   │   ├── TimeSharingChart.h/cpp, QuotePanel.h/cpp
│   │   ├── ContractList.h/cpp, TradeRecord.h/cpp
│   ├── indicator/
│   │   ├── IndicatorBase.h, MA.h/cpp, MACD.h/cpp
│   │   ├── KDJ.h/cpp, RSI.h/cpp, BOLL.h/cpp
│   │   └── WR.h/cpp, OBV.h/cpp
│   ├── trade/TradeManager.h/cpp, OrderPanel.h/cpp, PositionPanel.h/cpp
│   ├── system/LoginDialog.h/cpp, SettingsPage.h/cpp, ContractManager.h/cpp
│   └── db/Database.h/cpp
├── resources/
├── third_party/
└── tests/
```

## 8. 开发计划

### Phase 1: 行情展示（当前阶段）
- 项目骨架 + CMake 构建系统
- 数据层：MarketDataProvider 抽象 + SimDataProvider 模拟数据
- K线图表组件（QPainter自绘）
- 分时图 + 盘口面板 + 合约列表
- 主界面布局集成

### Phase 2: 行情分析
- 技术指标基类 + MA/MACD/KDJ/RSI/BOLL/WR/OBV
- 副图区域集成 + 图形绘制工具
- SQLite 历史K线存储

### Phase 3: 交易功能
- CTP 交易接口接入
- 下单/撤单面板 + 持仓/资金查询

### Phase 4: 系统管理
- 用户登录、数据源配置、合约管理、打包发布

## 9. 关键实施要点

- **刷新频率**：QTimer 250ms 驱动刷新（可配置）
- **内存管理**：环形缓冲区限制最大K线条数（2000根）
- **多线程**：CTP 数据接收独立线程 → Qt信号槽主线程渲染
- **断线重连**：CTP 连接断开自动重连，超时退订
- **异常处理**：关键操作确认对话框，spdlog 日志记录
