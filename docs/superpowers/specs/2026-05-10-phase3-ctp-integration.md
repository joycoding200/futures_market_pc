# Phase 3: CTP 行情接入 — 设计文档

> 日期：2026-05-10 | 状态：已确认

## 1. 概述

接入 CTP 期货行情接口，实现从期货公司前置机获取实时行情数据。与现有 SimDataProvider 并行运行，通过系统设置切换数据源。

## 2. CTPDataProvider 设计

继承 `MarketDataProvider`，封装 CTP MD API：

- 构造函数读取 Config 中的 brokerId、userId、password、frontAddr
- `start()` → 创建 MdApi 实例 → 注册回调 → 连接 → 登录 → 订阅
- `OnRtnDepthMarketData()` → 转 TickData → emit tickReceived()
- `stop()` → 退订 → 断开 → Release

## 3. TickData 映射

CTP 五档行情字段 → TickData 完全映射，字段名一一对应。

## 4. Application 数据源选择

`initDataLayer()` 根据 `Config::dataSourceType()` 选择 provider。

## 5. 文件结构

```
src/data/CTPDataProvider.h/cpp    (新增)
src/app/Application.cpp           (修改: provider选择)
CMakeLists.txt                    (修改: 条件编译CTP)
third_party/ctp/                  (用户提供SDK，gitignore)
```
