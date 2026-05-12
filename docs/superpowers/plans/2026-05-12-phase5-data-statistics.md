# Phase 5: 数据统计与筛选 — 实施方案

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task.

**Goal:** 补充原始需求中的"简单数据统计与筛选功能"，包括涨跌幅排行、成交量排名、合约筛选、成交量分布统计。

**Architecture:** 新增 stats/ 模块，基于内存 TickData 增量计算统计指标 → StatsPanel 展示排名 → 增强 ContractList 搜索/筛选能力。

**Tech Stack:** Qt 6.x / C++17 (延续现有技术栈)

---

### Task 1: 统计数据结构与计算引擎

**Files:**
- Create: `src/stats/StatsCalculator.h`
- Create: `src/stats/StatsCalculator.cpp`

- [ ] **Step 1: 创建 StatsCalculator.h**

```cpp
#pragma once
#include <QObject>
#include <QMap>
#include <QVector>
#include <QString>
#include "data/TickData.h"

// 单合约统计快照
struct ContractStats {
    QString  contract;
    double   lastPrice    = 0.0;
    double   preSettle    = 0.0;
    double   changeRate   = 0.0;   // 涨跌幅 %
    double   volume       = 0.0;
    double   openInterest = 0.0;
    double   highPrice    = 0.0;
    double   lowPrice     = 999999.0;
    double   amplitude    = 0.0;   // 振幅 %
    QString  exchange;             // 从合约代码前缀提取

    double change() const { return lastPrice - preSettle; }
};

// 成交量分布统计
struct VolumeDistribution {
    int largeCount  = 0;   // >= 100 手
    int mediumCount = 0;   // 30-99 手
    int smallCount  = 0;   // <30 手
    int totalCount  = 0;

    void addTrade(int volume) {
        if (volume >= 100) largeCount++;
        else if (volume >= 30) mediumCount++;
        else smallCount++;
        totalCount++;
    }
};

enum class SortField { Contract, Price, ChangeRate, Volume, Amplitude };

// 统计计算引擎 — 从 TickData 流增量更新合约统计
class StatsCalculator : public QObject {
    Q_OBJECT
public:
    explicit StatsCalculator(QObject* parent = nullptr);

    // 更新 Tick → 内部增量计算
    void onTick(const TickData& tick);

    // 查询接口
    QVector<ContractStats> topGainers(int count = 10) const;   // 涨幅Top N
    QVector<ContractStats> topLosers(int count = 10) const;    // 跌幅Top N
    QVector<ContractStats> topVolume(int count = 10) const;    // 成交量Top N
    QVector<ContractStats> allStats(SortField sortBy = SortField::ChangeRate,
                                     Qt::SortOrder order = Qt::DescendingOrder) const;
    ContractStats statsFor(const QString& contract) const;
    VolumeDistribution volumeDist() const { return m_volDist; }

    // 筛选
    QVector<ContractStats> filter(QString keyword, QString exchange = "",
                                   int direction = 0) const; // -1跌 0全部 +1涨

signals:
    void statsUpdated();  // 统计刷新后发射

private:
    QString extractExchange(const QString& contract) const;
    QMap<QString, ContractStats> m_stats;
    VolumeDistribution m_volDist;
};
```

- [ ] **Step 2: 实现 StatsCalculator.cpp**

```cpp
#include "StatsCalculator.h"
#include <algorithm>

StatsCalculator::StatsCalculator(QObject* parent) : QObject(parent) {}

void StatsCalculator::onTick(const TickData& tick) {
    if (!tick.isValid()) return;

    auto& s = m_stats[tick.contract];
    s.contract    = tick.contract;
    s.lastPrice   = tick.lastPrice;
    s.preSettle   = tick.preSettle;
    s.changeRate  = (tick.lastPrice - tick.preSettle) / tick.preSettle * 100.0;
    s.volume     += tick.volume;     // 累加成交量
    s.openInterest = tick.openInterest;
    s.highPrice   = std::max(s.highPrice, tick.lastPrice);
    s.lowPrice    = std::min(s.lowPrice, tick.lastPrice);
    s.amplitude   = (s.highPrice - s.lowPrice) / tick.preSettle * 100.0;
    if (s.exchange.isEmpty()) s.exchange = extractExchange(tick.contract);

    // 模拟逐笔成交量分布（每个Tick视为一笔）
    m_volDist.addTrade(static_cast<int>(tick.volume));

    emit statsUpdated();
}

QString StatsCalculator::extractExchange(const QString& contract) const {
    // 根据合约代码前缀推断交易所
    if (contract.startsWith("IF") || contract.startsWith("IC") ||
        contract.startsWith("IH") || contract.startsWith("IM")) return "CFFEX";
    if (contract.startsWith("rb") || contract.startsWith("hc") ||
        contract.startsWith("wr") || contract.startsWith("ss")) return "SHFE";
    if (contract.startsWith("m") || contract.startsWith("y") ||
        contract.startsWith("a") || contract.startsWith("p") ||
        contract.startsWith("c") || contract.startsWith("cs")) return "DCE";
    if (contract.startsWith("TA") || contract.startsWith("MA") ||
        contract.startsWith("FG") || contract.startsWith("SA") ||
        contract.startsWith("CF") || contract.startsWith("SR")) return "CZCE";
    if (contract.startsWith("sc") || contract.startsWith("lu")) return "INE";
    return "OTHER";
}

QVector<ContractStats> StatsCalculator::topGainers(int count) const {
    auto list = m_stats.values().toVector();
    std::sort(list.begin(), list.end(),
              [](const ContractStats& a, const ContractStats& b) { return a.changeRate > b.changeRate; });
    return list.mid(0, count);
}

QVector<ContractStats> StatsCalculator::topLosers(int count) const {
    auto list = m_stats.values().toVector();
    std::sort(list.begin(), list.end(),
              [](const ContractStats& a, const ContractStats& b) { return a.changeRate < b.changeRate; });
    return list.mid(0, count);
}

QVector<ContractStats> StatsCalculator::topVolume(int count) const {
    auto list = m_stats.values().toVector();
    std::sort(list.begin(), list.end(),
              [](const ContractStats& a, const ContractStats& b) { return a.volume > b.volume; });
    return list.mid(0, count);
}

QVector<ContractStats> StatsCalculator::allStats(SortField sortBy, Qt::SortOrder order) const {
    auto list = m_stats.values().toVector();
    std::sort(list.begin(), list.end(), [=](const ContractStats& a, const ContractStats& b) {
        int cmp = 0;
        switch (sortBy) {
            case SortField::Contract:   cmp = a.contract.compare(b.contract); break;
            case SortField::Price:      cmp = (a.lastPrice > b.lastPrice) - (a.lastPrice < b.lastPrice); break;
            case SortField::ChangeRate: cmp = (a.changeRate > b.changeRate) - (a.changeRate < b.changeRate); break;
            case SortField::Volume:     cmp = (a.volume > b.volume) - (a.volume < b.volume); break;
            case SortField::Amplitude:  cmp = (a.amplitude > b.amplitude) - (a.amplitude < b.amplitude); break;
        }
        return order == Qt::DescendingOrder ? cmp > 0 : cmp < 0;
    });
    return list;
}

ContractStats StatsCalculator::statsFor(const QString& contract) const {
    return m_stats.value(contract);
}

QVector<ContractStats> StatsCalculator::filter(QString keyword, QString exchange, int direction) const {
    QVector<ContractStats> result;
    for (const auto& s : m_stats) {
        if (!keyword.isEmpty() && !s.contract.contains(keyword, Qt::CaseInsensitive)) continue;
        if (!exchange.isEmpty() && s.exchange != exchange) continue;
        if (direction > 0 && s.changeRate <= 0) continue;   // 仅上涨
        if (direction < 0 && s.changeRate >= 0) continue;   // 仅下跌
        result.append(s);
    }
    return result;
}
```

- [ ] **Step 3: Commit**

```bash
git add src/stats/
git commit -m "feat: add StatsCalculator for real-time contract statistics"
```

---

### Task 2: 成交量分布面板

**Files:**
- Modify: `src/chart/TradeRecord.h` — 新增成交量分布摘要区
- Modify: `src/chart/TradeRecord.cpp` — 实现分布摘要渲染

- [ ] **Step 1: 修改 TradeRecord**

在 TradeRecord 底部添加成交量分布摘要行：

```cpp
// TradeRecord.h 新增成员：
    void updateVolumeDist(int large, int medium, int small);
    QLabel* m_distLabel;

// TradeRecord.cpp 构造函数底部：
    m_distLabel = new QLabel("大:0  中:0  小:0");
    m_distLabel->setStyleSheet("color: #888; font-size: 10px; padding: 2px;");
    m_distLabel->setAlignment(Qt::AlignRight);
    layout->addWidget(m_distLabel);

// 实现：
void TradeRecord::updateVolumeDist(int large, int medium, int small) {
    m_distLabel->setText(QString("大:<span style='color:#b482ff'>%1</span>  "
                                  "中:<span style='color:#ffd700'>%2</span>  "
                                  "小:<span style='color:#888'>%3</span>")
        .arg(large).arg(medium).arg(small));
}
```

- [ ] **Step 2: Commit**

```bash
git add src/chart/TradeRecord.h src/chart/TradeRecord.cpp
git commit -m "feat: add volume distribution summary to TradeRecord panel"
```

---

### Task 3: ContractList 搜索与筛选增强

**Files:**
- Modify: `src/chart/ContractList.h`
- Modify: `src/chart/ContractList.cpp`

- [ ] **Step 1: 添加搜索栏和交易所筛选**

在 ContractList 顶部添加搜索/筛选控件：

```cpp
// ContractList.h 新增成员：
    QLineEdit* m_searchBox;
    QComboBox* m_exchangeFilter;

// ContractList.cpp setupUI 中添加：
    auto* filterRow = new QHBoxLayout();
    m_searchBox = new QLineEdit();
    m_searchBox->setPlaceholderText("搜索合约...");
    m_searchBox->setClearButtonEnabled(true);
    m_exchangeFilter = new QComboBox();
    m_exchangeFilter->addItems({"全部", "CFFEX", "SHFE", "DCE", "CZCE", "INE"});
    filterRow->addWidget(m_searchBox);
    filterRow->addWidget(m_exchangeFilter);
    layout->insertLayout(0, filterRow);

    connect(m_searchBox, &QLineEdit::textChanged, this, &ContractList::applyFilter);
    connect(m_exchangeFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this]() { applyFilter(); });
```

- [ ] **Step 2: 实现过滤逻辑**

```cpp
void ContractList::applyFilter() {
    QString keyword = m_searchBox->text().trimmed();
    QString exchange = m_exchangeFilter->currentText();
    if (exchange == "全部") exchange.clear();

    for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
        auto* item = m_tree->topLevelItem(i);
        bool match = item->text(0).contains(keyword, Qt::CaseInsensitive);
        // 隐藏不匹配的项
        item->setHidden(!match);
    }
}
```

- [ ] **Step 3: 表头点击排序**

```cpp
// 构造函数中：
    m_tree->setSortingEnabled(true);
    m_tree->header()->setSortIndicatorShown(true);
```

- [ ] **Step 4: Commit**

```bash
git add src/chart/ContractList.h src/chart/ContractList.cpp
git commit -m "feat: add search, exchange filter and column sorting to ContractList"
```

---

### Task 4: StatsPanel 统计面板

**Files:**
- Create: `src/stats/StatsPanel.h`
- Create: `src/stats/StatsPanel.cpp`

- [ ] **Step 1: 创建 StatsPanel**

提供三个视图的 Tab 切换：涨幅榜 / 跌幅榜 / 成交量榜

```cpp
// StatsPanel.h
#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QTabWidget>
#include "StatsCalculator.h"

class StatsPanel : public QWidget {
    Q_OBJECT
public:
    explicit StatsPanel(StatsCalculator* calc, QWidget* parent = nullptr);

public slots:
    void refresh();

private:
    void populateTable(QTableWidget* table, const QVector<ContractStats>& data);

    StatsCalculator* m_calc;
    QTabWidget* m_tabs;
    QTableWidget* m_gainerTable;
    QTableWidget* m_loserTable;
    QTableWidget* m_volumeTable;
};
```

- [ ] **Step 2: 实现 StatsPanel**

每个 QTableWidget 列：合约代码 | 最新价 | 涨跌幅 | 振幅 | 成交量。
涨跌行高亮（红涨绿跌背景色）。

- [ ] **Step 3: Commit**

```bash
git add src/stats/StatsPanel.h src/stats/StatsPanel.cpp
git commit -m "feat: add StatsPanel with gainer/loser/volume ranking tabs"
```

---

### Task 5: MainWindow 集成 + CMakeLists

**Files:**
- Modify: `src/app/MainWindow.h`
- Modify: `src/app/MainWindow.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: 集成到右侧面板**

在 MainWindow 右侧面板中添加 StatsPanel 和 VolumeDistribution：

```cpp
// 右侧面板布局调整为：
// 五档盘口 → 逐笔成交 → 成交量分布 → 统计面板(Tab)
```

- [ ] **Step 2: 连接 StatsCalculator 数据流**

```cpp
// 在 setupConnections 中：
    m_statsCalc = new StatsCalculator(this);
    connect(m_buffer, &MarketDataBuffer::tickUpdated,
            m_statsCalc, &StatsCalculator::onTick);
    connect(m_statsCalc, &StatsCalculator::statsUpdated,
            m_statsPanel, &StatsPanel::refresh);
    // 成交量分布同步
    connect(m_statsCalc, &StatsCalculator::statsUpdated, this, [this]() {
        auto dist = m_statsCalc->volumeDist();
        m_tradeRecord->updateVolumeDist(dist.largeCount, dist.mediumCount, dist.smallCount);
    });
```

- [ ] **Step 3: 更新 CMakeLists.txt**

添加 stats/ 源文件和头文件。

- [ ] **Step 4: Commit**

```bash
git add src/app/ CMakeLists.txt
git commit -m "feat: integrate StatsPanel and volume distribution into MainWindow"
```

---

## 验证方案

### 编译验证
```bash
cd build && cmake --build .
```
预期：0错误。

### 运行时验证
1. 启动程序 → 右侧面板显示统计 Tab（涨幅榜/跌幅榜/成交量榜）
2. 多个合约数据运行一段时间 → 涨幅榜实时更新排序
3. 合约列表上方搜索框输入 "rb" → 仅显示螺纹钢合约
4. 交易所筛选选 "CFFEX" → 仅显示金融期货
5. 逐笔成交底部显示大/中/小单计数，实时累加
6. 点击合约列表表头 → 按列排序
7. 右侧统计面板双击某合约 → K线图切换到该合约

### 测试检查项
- [ ] 统计计算正确（涨跌幅公式、成交量累加）
- [ ] 排序方向正确（涨幅从高到低、跌幅从低到高）
- [ ] 筛选组合使用（搜索+交易所+涨跌方向）正确
- [ ] 无合约时统计面板显示空状态，不崩溃
- [ ] 成交量分布计数器溢出处理（总计上限）
