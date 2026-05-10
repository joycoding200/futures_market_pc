# Phase 2: 技术指标分析 — 设计文档

> 日期：2026-05-10 | 状态：已确认

## 1. 概述

在 Phase 1 行情展示基础上，为 K 线图添加专业技术指标分析功能。实现 4 个核心指标（MA/MACD/KDJ/RSI），采用混合渲染模式（MA 叠加主图，其余独立副图）。

## 2. 指标基类设计

```cpp
enum class IndicatorRenderType { Overlay, SubChart };

class IndicatorBase {
public:
    virtual ~IndicatorBase() = default;
    virtual QString name() const = 0;
    virtual void calculate(const QVector<KLineData>& klines) = 0;
    virtual double valueAt(int index) const = 0;
    virtual IndicatorRenderType renderType() const = 0;
    virtual int lineCount() const = 0;          // 有几条线/柱
    virtual QColor lineColor(int i) const = 0;   // 第 i 条线的颜色
    virtual QString lineName(int i) const = 0;   // 第 i 条线的名称
};
```

## 3. 四个指标规格

### 3.1 MA (移动平均线) — Overlay

- 参数：5/10/20/60 周期
- 计算：`MA(n) = sum(close, last n) / n`
- 渲染：4 条彩色折线叠加在 K 线主图上
- 颜色：MA5=白, MA10=黄, MA20=紫, MA60=蓝

### 3.2 MACD (指数平滑异同移动平均线) — SubChart

- 参数：快线12, 慢线26, 信号9
- 计算：
  - `EMA12 = EMA(close, 12)`, `EMA26 = EMA(close, 26)`
  - `DIF = EMA12 - EMA26`
  - `DEA = EMA(DIF, 9)`
  - `BAR = 2*(DIF - DEA)`
- 渲染：红/绿柱状图 (BAR) + DIF 白线 + DEA 黄线
- Y 轴范围动态计算

### 3.3 KDJ (随机指标) — SubChart

- 参数：RSV周期9, K平滑3, D平滑3
- 计算：
  - `RSV = (close - low9) / (high9 - low9) * 100`
  - `K = 2/3*K_prev + 1/3*RSV`
  - `D = 2/3*D_prev + 1/3*K`
  - `J = 3*K - 2*D`
- 渲染：K 白线 + D 黄线 + J 紫线
- Y 轴固定 0~100，画 20/80 超卖超买参考线

### 3.4 RSI (相对强弱指标) — SubChart

- 参数：6/12/24 周期
- 计算：
  - `RS = avgGain(n) / avgLoss(n)`
  - `RSI = 100 - 100/(1+RS)`
- 渲染：3 条 RSI 曲线
- Y 轴固定 0~100，画 30/70 参考线

## 4. 界面集成

### 4.1 副图区域

在 KLineChart 中新增一个可选的**指标副图面板**，位于 K 线主图下方、分时图上方：

- 默认显示 MA（叠加在主图） + MACD（副图）
- 副图区右上角显示指标名称下拉菜单，切换 MACD/KDJ/RSI
- 副图高度约占 KLineChart 总高度的 25%

### 4.2 改动 KLineChart

在 `drawAll()` 中新增步骤：
1. 绘制背景网格（已有）
2. 绘制 K 线实体 + 成交量（已有）
3. **绘制 Overlay 指标线（MA）** — 新增
4. 绘制价格标签（已有）
5. **绘制副图区域分隔线 + 副图指标** — 新增

### 4.3 指标管理

KLineChart 新增成员：
```cpp
QVector<IndicatorBase*> m_indicators;    // 所有加载的指标
IndicatorBase* m_activeSubIndicator;     // 当前激活的副图指标
bool m_showIndicators = true;            // 是否显示指标区域
```

## 5. 文件结构

```
src/indicator/
├── IndicatorBase.h        # 基类
├── MA.h / MA.cpp          # 移动平均线
├── MACD.h / MACD.cpp      # MACD
├── KDJ.h / KDJ.cpp        # KDJ
└── RSI.h / RSI.cpp        # RSI
```

CHANGES:
- `src/chart/KLineChart.h/cpp` — 集成指标绘制逻辑
- `src/chart/ChartPainter.h/cpp` — 新增 drawIndicatorLines() 方法
- `CMakeLists.txt` — 添加 indicator/ 源文件
