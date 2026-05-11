# 项目结构

```
pc_futuresmarketanalysis/
├── CMakeLists.txt              # 构建配置 (37源文件)
├── vcpkg.json                  # vcpkg 依赖声明
├── README.md                   # 项目说明
├── verify_instruction.md       # 编译与验证指南
├── build.sh                    # 构建脚本
│
├── src/
│   ├── main.cpp                # 入口: 创建 Application 并 run()
│   │
│   ├── app/                    # 应用层
│   │   ├── Application.h/cpp   # 启动编排: 日志→EventBus→数据→UI
│   │   └── MainWindow.h/cpp    # 主窗口: 三栏布局 + 菜单栏
│   │
│   ├── core/                   # 核心基础设施
│   │   ├── EventBus.h/cpp      # 全局事件总线 (单例)
│   │   └── Config.h/cpp        # QSettings 配置封装 (单例)
│   │
│   ├── data/                   # 数据层
│   │   ├── TickData.h          # Tick 数据结构 + 五档盘口
│   │   ├── KLineData.h         # K线数据结构 + 8周期枚举
│   │   ├── MarketDataProvider.h # 抽象数据源接口
│   │   ├── SimDataProvider.h/cpp # 模拟行情 (随机游走算法)
│   │   ├── CTPDataProvider.h/cpp # CTP 真实行情 (条件编译)
│   │   ├── MarketDataBuffer.h/cpp # 内存数据缓存 (deque)
│   │   └── KLineAggregator.h/cpp  # Tick→K线周期聚合
│   │
│   ├── chart/                  # 图表展示层
│   │   ├── KLineChart.h/cpp    # K线主图 (QPainter 自绘)
│   │   ├── ChartAxis.h/cpp     # 坐标轴计算 (价格/时间)
│   │   ├── ChartPainter.h/cpp  # 绘图工具 (蜡烛/折线/柱状)
│   │   ├── Crosshair.h/cpp     # 十字光标 + 浮窗信息
│   │   ├── TimeSharingChart.h/cpp # 分时走势图
│   │   ├── QuotePanel.h/cpp    # 五档盘口面板
│   │   ├── ContractList.h/cpp  # 自选合约列表
│   │   └── TradeRecord.h/cpp   # 逐笔成交记录
│   │
│   ├── indicator/              # 技术指标
│   │   ├── IndicatorBase.h     # 指标抽象基类
│   │   ├── MA.h/cpp            # 移动平均线 (5/10/20/60)
│   │   ├── MACD.h/cpp          # MACD (12/26/9)
│   │   ├── KDJ.h/cpp           # KDJ (9/3/3)
│   │   └── RSI.h/cpp           # RSI (6/12/24)
│   │
│   ├── system/                 # 系统管理
│   │   ├── SettingsDialog.h/cpp # 设置对话框 (行情+数据源标签)
│   │   ├── ContractManager.h/cpp # 合约管理对话框
│   │   └── AboutDialog.h/cpp   # 关于对话框
│   │
│   └── db/                     # 数据库
│       └── Database.h/cpp      # SQLite 封装 (CREATE TABLE + INSERT)
│
├── tests/                      # 测试 (骨架级别)
│   ├── test_data_layer.cpp     # TickData/KLineData 基础验证
│   ├── test_kline_chart.cpp    # (空壳)
│   └── test_main_window.cpp    # (空壳)
│
├── third_party/
│   ├── ctp/                    # CTP SDK (头文件 + DLL)
│   └── spdlog/                 # spdlog (FetchContent 自动拉取)
│
├── docs/
│   ├── build-experience.md     # 构建经验记录
│   ├── review-report.md        # 代码审查报告 (10/10 修复)
│   └── superpowers/
│       ├── specs/              # 设计规格 (4个)
│       └── plans/              # 实现计划 (5个)
│
└── referrences/                # 参考资料
```

## 模块统计

| 模块 | 文件数 | 职责 |
|------|--------|------|
| app/ | 4 | 启动 + 主窗口 |
| core/ | 4 | 事件 + 配置 |
| data/ | 12 | 数据源 + 缓冲 + 聚合 |
| chart/ | 14 | 图表展示 |
| indicator/ | 8 | 技术指标计算 |
| system/ | 6 | 系统管理 UI |
| db/ | 2 | 数据库 |
| **合计** | **50** | |
