# Phase 4: 系统管理 — 设计文档

> 日期：2026-05-10 | 状态：已确认

## 1. 概述

为期货行情PC软件添加系统管理功能，包括系统参数设置、数据源配置、合约订阅管理和关于窗口。

## 2. 组件设计

### 2.1 SettingsDialog

QDialog + QTabWidget，两个标签页：

**行情标签页：**
- 刷新间隔 (ms) — QSpinBox，默认250，范围50-2000
- K线最大数量 — QSpinBox，默认2000，范围100-10000
- 模拟基准价 — QDoubleSpinBox，默认4000
- 波动幅度 — QDoubleSpinBox，默认2.0

**数据源标签页：**
- 数据源类型 — QComboBox: "模拟数据(Sim)" / "CTP" / "HTTP"
- CTP BrokerID — QLineEdit（当选择CTP时显示）
- CTP 行情地址 — QLineEdit（当选择CTP时显示）
- 当前状态 — QLabel 只读显示

所有配置读写 `Config::instance()`，点击确定写入 QSettings。

### 2.2 ContractManager

QDialog:
- QListWidget 显示当前订阅合约（代码 + 名称）
- QLineEdit(合约代码) + QLineEdit(合约名称) + QPushButton("添加")
- 每个列表项右侧有删除按钮
- 合约列表通过 Config::defaultContracts() 持久化（逗号分隔）

### 2.3 AboutDialog

QDialog:
- 软件名称: "期货行情PC软件"
- 版本: 0.2.0
- Qt 版本: 运行时获取
- 技术栈: Qt 6.x / C++17 / QPainter自绘

### 2.4 MainWindow 菜单栏

```cpp
menubar = new QMenuBar();
系统 → 系统设置 / 合约管理 / 关于
```

## 3. Config 新增字段

```cpp
QString dataSourceType() const;      // "Sim"
void setDataSourceType(const QString&);
QString ctpBrokerId() const;
void setCtpBrokerId(const QString&);
QString ctpAddress() const;
void setCtpAddress(const QString&);
```

## 4. 文件结构

```
src/system/
├── SettingsDialog.h/cpp
├── ContractManager.h/cpp
└── AboutDialog.h/cpp
```

CHANGES:
- `src/core/Config.h/cpp` — 新数据源字段
- `src/app/MainWindow.h/cpp` — 菜单栏
- `CMakeLists.txt` — system/ 源文件
