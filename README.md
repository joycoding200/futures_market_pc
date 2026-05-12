# 期货行情PC软件

面向国内期货市场的 Windows 桌面行情分析软件，基于 **Qt 6.x / C++17** 构建，支持实时行情展示、技术指标分析和 CTP 行情接入。

## 系统要求

- Windows 10 或更高版本
- Qt 6.11.0+ (MinGW 64-bit)
- MinGW 1.1.3.10+

## 快速开始

```bash
# 确保 MinGW 在 PATH 中
export PATH="/c/Dev/Qt/Tools/mingw1310_64/bin:$PATH"

# 执行构建脚本
bash build.sh
```

构建产物：`bin/FuturesMarketPC.exe`，直接双击运行。

**首次启动**：默认加载 8 个预设合约，使用模拟数据源 250ms 刷新。如需真实行情，在 `系统 → 系统设置 → 数据源` 切换到 CTP。

## 主要功能

### 行情查看
- **合约列表**（左侧 200px）：点击切换合约，顶部搜索框实时过滤，下拉框按交易所筛选
- **K线图**（中央）：QPainter 自绘，滚轮缩放、拖拽平移、十字光标（悬停显示 O/H/L/C/V）
- **周期切换**：1m / 5m / 15m / 30m / 1H / 日K / 周K / 月K
- **分时图**（中央下方）：实时价格折线 + 黄色均价线 + 昨结算虚线
- **盘口面板**（右侧）：五档买卖盘实时刷新，涨跌幅红绿色标识
- **逐笔成交**（右侧）：按成交量大小着色 — 紫色粗体 ≥100手 / 黄色 30-99手 / 灰色 <30手，底部大中小单计数

### 技术指标
- **MA 均线**：叠加主图，MA5(白) / MA10(黄) / MA20(紫) / MA60(蓝)
- **MACD**：副图红绿柱 + DIF 白线 + DEA 黄线
- **KDJ**：K/D/J 三线，固定 0~100 范围，20/80 超卖超买参考线
- **RSI**：RSI6/12/24 三线，固定 0~100 范围，30/70 参考线

### 数据统计（Phase 5）
- **涨幅榜 / 跌幅榜 / 成交量榜**：右侧统计面板三个 Tab 切换，涨跌幅红涨绿跌着色
- **合约筛选**：按代码关键词搜索 + 按交易所分类过滤
- **成交量分布**：逐笔成交底部实时显示大/中/小单笔数

### 系统管理
- **系统设置**：刷新间隔、K线数量、模拟参数、数据源类型（Sim/CTP）
- **合约管理**：添加/删除自选合约
- **关于**：版本与 Qt 版本信息

## 项目结构

```
src/
├── app/       Application, MainWindow
├── core/      EventBus, Config
├── data/      MarketDataProvider, SimDataProvider, CTPDataProvider, KLineAggregator
├── chart/     KLineChart, TimeSharingChart, QuotePanel, ContractList, TradeRecord
├── indicator/ MA, MACD, KDJ, RSI
├── stats/     StatsCalculator, StatsPanel
├── system/    SettingsDialog, ContractManager, AboutDialog
└── db/        Database (SQLite)
```

## 技术栈

Qt 6.x / C++17 / QPainter 自绘 / SQLite / spdlog / CTP v6.6+

## 常见问题

**Q: 启动后无数据？**
检查是否已订阅合约（`系统 → 合约管理`），默认 8 个预设合约会自动加载。

**Q: K线图空白？**
模拟数据源需积累数据。等待 1-2 分钟即可看到完整 K 线。

**Q: CTP 连接失败？**
确认 `系统设置 → 数据源` 中 BrokerID 和行情地址正确，且 CTP SDK DLL 在程序同目录。

**Q: 如何切换数据源？**
`系统 → 系统设置 → 数据源` 标签页选择 Sim 或 CTP。切换后需重启程序。
