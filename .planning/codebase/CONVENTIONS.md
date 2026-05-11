# 代码规范

## 命名风格

| 元素 | 风格 | 示例 |
|------|------|------|
| 类名 | PascalCase | `KLineChart`, `MarketDataProvider` |
| 成员变量 | `m_` 前缀 + camelCase | `m_dataProvider`, `m_startIndex` |
| 成员方法 | camelCase | `setContract()`, `onTickUpdated()` |
| 静态方法 | camelCase | `instance()`, `makeKey()` |
| 局部变量 | camelCase | `currentPrice`, `chartLayout` |
| 枚举 | PascalCase (C++ enum class) | `KLineType::M1`, `EventType::TickUpdate` |
| 常量 | UPPER_SNAKE_CASE | `DEFAULT_INTERVAL_MS` |
| 头文件保护 | `#pragma once` | - |

## 文件组织

- 每个类单独一个 `.h` + `.cpp` 文件
- 按功能模块分目录 (app/core/data/chart/indicator/system/db)
- 头文件包含路径使用相对于 `src/` 的路径 (如 `#include "data/TickData.h"`)
- 目录名小写单词

## Qt 惯例

- 所有 QObject 子类使用 `Q_OBJECT` 宏
- 信号声明在 `signals:` 段，槽声明在 `public slots:` 或 `private slots:`
- 跨组件通信优先使用信号/槽，不使用直接回调
- UI 构造函数中 `parent` 参数确保 Qt 自动管理生命周期

## 内存管理

- QObject 子类依赖 Qt parent-child 机制自动释放
- 非 QObject 对象：`Config` 使用 `std::unique_ptr<QSettings>`
- 跨组件传递的数据指针以裸指针为主（不拥有所有权）

## 设计模式

| 模式 | 应用 |
|------|------|
| 单例 (Singleton) | `EventBus`, `Config` (Meyer's Singleton) |
| 策略 (Strategy) | `MarketDataProvider` → `SimDataProvider` / `CTPDataProvider` |
| 模板方法 (Template Method) | `IndicatorBase` → `MA`, `MACD`, `KDJ`, `RSI` |
| 观察者 (Observer) | Qt 信号/槽机制 |

## 日志

- 使用 spdlog 宏：`spdlog::info()`, `spdlog::error()`
- 日志消息使用中文（如 `"SimDataProvider: 订阅合约 {}"`）
- 关键状态变更需记录（启动、停止、连接、错误）

## 注释

- 极简风格，无冗余注释
- 中文注释出现在关键入口处（如 `main.cpp`）
- 头文件中类/方法以清晰命名自解释，不额外注释
