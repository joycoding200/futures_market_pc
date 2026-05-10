# 期货行情PC软件

面向国内期货市场的桌面行情分析软件，基于 **Qt 6.x / C++17** 构建，支持实时行情展示、技术指标分析和 CTP 行情接入。

## 功能

### Phase 1 — 行情展示
- 实时行情数据接收与推送（Sim/CTP 双数据源）
- K线图（QPainter 自绘）：缩放、平移、十字光标、周期切换
- 分时走势图：价格折线 + 均价线 + 昨结算基线
- 五档盘口面板 + 逐笔成交记录
- 自选合约列表管理

### Phase 2 — 技术指标
- MA 移动平均线 (5/10/20/60) — 叠加主图
- MACD 指数平滑 (12/26/9) — DIF/DEA/BAR 柱状副图
- KDJ 随机指标 (9/3/3) — K/D/J 三线副图
- RSI 相对强弱 (6/12/24) — 超买超卖参考线

### Phase 3 — CTP 行情接入
- CTPDataProvider：封装 CTP 行情 API，实时接收交易所行情
- 数据源切换：Sim（开发模拟）/ CTP（生产行情）

### Phase 4 — 系统管理
- 系统设置对话框：刷新间隔、K线数量、模拟参数、数据源配置
- 合约管理：添加/删除自选合约
- 关于对话框

## 架构

```
Core(EventBus/Config) → Data(MarketDataProvider → SimDataProvider/CTPDataProvider)
                      → Chart(KLineChart/TimeSharingChart/QuotePanel)
                      → Indicator(MA/MACD/KDJ/RSI)
                      → System(SettingsDialog/ContractManager/AboutDialog)
                      → App(MainWindow)
```

## 技术栈

| 类别 | 选型 |
|------|------|
| 框架 | Qt 6.x / C++17 |
| 图表 | Qt QPainter 自绘 + QCustomPlot |
| 行情协议 | CTP v6.6+ / 本地模拟 |
| 构建 | CMake + vcpkg |
| 存储 | SQLite |
| 日志 | spdlog |

## 编译与运行

详见 [verify_instruction.md](./verify_instruction.md)

```bash
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

## 项目结构

```
pc_futuresmarketanalysis/
├── src/
│   ├── app/          # Application + MainWindow
│   ├── core/         # EventBus + Config
│   ├── data/         # MarketDataProvider / SimDataProvider / CTPDataProvider / Buffer / Aggregator
│   ├── chart/        # KLineChart / TimeSharingChart / QuotePanel / ContractList / TradeRecord
│   ├── indicator/    # IndicatorBase / MA / MACD / KDJ / RSI
│   ├── system/       # SettingsDialog / ContractManager / AboutDialog
│   └── db/           # SQLite
├── third_party/ctp/  # CTP SDK
├── docs/superpowers/ # 设计文档与实施方案
└── CMakeLists.txt
```
