# 测试现状

## 当前测试覆盖

### 测试文件 (3个，骨架级别)

| 文件 | 内容 | 框架 |
|------|------|------|
| `tests/test_data_layer.cpp` | TickData 有效性、KLineData 涨跌判断 | `assert()` 裸用 |
| `tests/test_kline_chart.cpp` | 空壳 | - |
| `tests/test_main_window.cpp` | 空壳 | - |

### 覆盖情况

| 模块 | 测试覆盖 | 状态 |
|------|----------|------|
| data/TickData | `isValid()`, 涨跌判断 | 基础覆盖 |
| data/KLineData | `isRising()`, `range()` | 基础覆盖 |
| data/MarketDataProvider | 无 | ❌ |
| data/SimDataProvider | 无 | ❌ |
| data/CTPDataProvider | 无 | ❌ |
| data/MarketDataBuffer | 无 | ❌ |
| data/KLineAggregator | 无 | ❌ |
| chart/KLineChart | 无 | ❌ |
| indicator/MA | 无 | ❌ |
| indicator/MACD | 无 | ❌ |
| indicator/KDJ | 无 | ❌ |
| indicator/RSI | 无 | ❌ |
| core/Config | 无 | ❌ |
| core/EventBus | 无 | ❌ |
| system/SettingsDialog | 无 | ❌ |

## 问题

1. **无测试框架**：未集成 Google Test、Catch2 或 QTest
2. **无 CMake 集成**：`tests/` 未加入构建系统（无 `enable_testing()` / `add_test()`）
3. **测试为空壳**：3 个文件中 2 个没有实际测试
4. **核心逻辑未覆盖**：随机游走算法、K 线聚合、指标计算等均无测试
5. **无 Qt 测试**：UI 组件完全未覆盖（可使用 QTest 进行 widget 测试）

## 建议的测试策略

1. 引入 Google Test 或 Catch2 作为测试框架
2. 将 `tests/` 集成到 CMake 构建 (`enable_testing()` + `add_test()`)
3. 优先测试核心算法：SimDataProvider::randomWalk、KLineAggregator、各指标 calculate()
4. Qt UI 组件使用 QTest 进行功能测试
5. 目标覆盖率：数据层/指标层 > 80%
