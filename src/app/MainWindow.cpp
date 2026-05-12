#include "MainWindow.h"
#include "chart/KLineChart.h"
#include "chart/TimeSharingChart.h"
#include "chart/QuotePanel.h"
#include "chart/ContractList.h"
#include "chart/TradeRecord.h"
#include "data/MarketDataBuffer.h"
#include "data/MarketDataProvider.h"
#include "core/EventBus.h"
#include "system/SettingsDialog.h"
#include "system/ContractManager.h"
#include "system/AboutDialog.h"
#include "stats/StatsCalculator.h"
#include "stats/StatsPanel.h"
#include <QTabWidget>
#include <QMenuBar>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>

MainWindow::MainWindow(MarketDataBuffer* buffer, MarketDataProvider* provider, QWidget* parent)
    : QMainWindow(parent), m_buffer(buffer), m_dataProvider(provider) {
    setWindowTitle("期货行情PC软件");
    resize(1400, 900);
    setStyleSheet("QMainWindow { background: #12121a; }");

    // 统计引擎 — 在 setupUI 前创建，供 StatsPanel 使用
    m_statsCalc = new StatsCalculator(this);
    connect(m_buffer, &MarketDataBuffer::tickUpdated,
            m_statsCalc, &StatsCalculator::onTick);

    setupUI();
    setupConnections();
    initDefaultContracts();

    auto* menuBar = new QMenuBar(this);
    auto* sysMenu = menuBar->addMenu("系统");
    sysMenu->addAction("系统设置...", this, &MainWindow::openSettings);
    sysMenu->addAction("合约管理...", this, &MainWindow::openContractManager);
    sysMenu->addAction("关于...", this, &MainWindow::openAbout);
    setMenuBar(menuBar);

    // 状态栏
    m_statusBar = statusBar();
    m_statusBar->showMessage("数据源: Sim | 就绪");
}

void MainWindow::setupUI() {
    auto* centralWidget = new QWidget();
    auto* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(4);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    m_contractList = new ContractList();
    m_contractList->setFixedWidth(200);
    mainLayout->addWidget(m_contractList);

    auto* chartArea = new QWidget();
    auto* chartLayout = new QVBoxLayout(chartArea);
    chartLayout->setSpacing(4);
    chartLayout->setContentsMargins(0, 0, 0, 0);

    m_klineChart = new KLineChart(m_buffer);
    m_klineChart->setMinimumHeight(350);
    chartLayout->addWidget(m_klineChart, 3);

    // 工具栏：周期选择 + 指标选择
    auto* toolbar = new QHBoxLayout();
    auto* periodLabel = new QLabel("周期:");
    periodLabel->setStyleSheet("color: #888; font-size: 11px;");
    toolbar->addWidget(periodLabel);
    auto* periodCombo = new QComboBox();
    periodCombo->addItems({"1m", "5m", "15m", "30m", "1H", "日K", "周K", "月K"});
    periodCombo->setCurrentIndex(5); // 默认日K
    periodCombo->setStyleSheet("QComboBox { background: #1e1e2a; color: #ccc; border: 1px solid #333; padding: 2px 8px; font-size: 11px; }");
    toolbar->addWidget(periodCombo);
    toolbar->addSpacing(12);
    auto* indLabel = new QLabel("指标:");
    indLabel->setStyleSheet("color: #888; font-size: 11px;");
    toolbar->addWidget(indLabel);
    auto* indicatorCombo = new QComboBox();
    indicatorCombo->addItems({"MACD", "KDJ", "RSI"});
    indicatorCombo->setStyleSheet("QComboBox { background: #1e1e2a; color: #ccc; border: 1px solid #333; padding: 2px 8px; font-size: 11px; }");
    toolbar->addWidget(indicatorCombo);
    toolbar->addStretch();
    chartLayout->addLayout(toolbar);

    // 周期切换
    QObject::connect(periodCombo, &QComboBox::currentIndexChanged, this, [this](int idx) {
        static const KLineType types[] = {KLineType::M1, KLineType::M5, KLineType::M15, KLineType::M30, KLineType::H1, KLineType::D, KLineType::W, KLineType::M};
        m_klineChart->setPeriod(types[idx]);
    });

    // 指标切换
    QObject::connect(indicatorCombo, &QComboBox::currentIndexChanged, this, [this](int idx) {
        auto& indicators = m_klineChart->indicators();
        // idx 0=MACD, 1=KDJ, 2=RSI; m_indicators布局: [0]=MA, [1]=MACD, [2]=KDJ, [3]=RSI
        if (idx + 1 < indicators.size())
            m_klineChart->setSubIndicator(indicators[idx + 1]);
    });

    auto* tabWidget = new QTabWidget();
    tabWidget->setStyleSheet(
        "QTabWidget::pane { background: #1a1a24; border: 1px solid #333; }"
        "QTabBar::tab { background: #12121a; color: #888; padding: 6px 16px; }"
        "QTabBar::tab:selected { background: #1a1a24; color: #fff; }"
    );
    m_timeChart = new TimeSharingChart(m_buffer);
    tabWidget->addTab(m_timeChart, "分时图");
    tabWidget->addTab(new QWidget(), "盘口");
    chartLayout->addWidget(tabWidget, 2);

    mainLayout->addWidget(chartArea, 1);

    auto* rightPanel = new QWidget();
    auto* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setSpacing(4);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    m_quotePanel = new QuotePanel();
    m_quotePanel->setMinimumWidth(220);
    rightLayout->addWidget(m_quotePanel, 1);

    m_tradeRecord = new TradeRecord();
    rightLayout->addWidget(m_tradeRecord, 1);

    m_statsPanel = new StatsPanel(m_statsCalc);
    m_statsPanel->setMinimumWidth(220);
    rightLayout->addWidget(m_statsPanel, 1);

    connect(m_statsCalc, &StatsCalculator::statsUpdated,
            m_statsPanel, &StatsPanel::refresh);
    connect(m_statsCalc, &StatsCalculator::statsUpdated, this, [this]() {
        auto dist = m_statsCalc->volumeDist();
        m_tradeRecord->updateVolumeDist(dist.largeCount, dist.mediumCount, dist.smallCount);
    });

    rightPanel->setFixedWidth(280);
    mainLayout->addWidget(rightPanel);

    setCentralWidget(centralWidget);
}

void MainWindow::setupConnections() {
    connect(m_contractList, &ContractList::contractClicked, this, [this](const QString& contract) {
        m_klineChart->setContract(contract);
        m_timeChart->setContract(contract);
        m_statusBar->showMessage(QString("合约: %1 | 周期: %2").arg(contract, "日K"));
    });

    // 十字光标信息 → 状态栏
    connect(m_klineChart, &KLineChart::crosshairInfo, this, [this](const QString& info) {
        m_statusBar->showMessage(info, 5000);
    });

    // K线图和分时图订阅 Buffer 实时数据
    connect(m_buffer, &MarketDataBuffer::tickUpdated,
            m_klineChart, &KLineChart::onTickUpdated);
    connect(m_buffer, &MarketDataBuffer::klineUpdated,
            m_klineChart, &KLineChart::onKLineUpdated);
    connect(m_buffer, &MarketDataBuffer::tickUpdated,
            m_timeChart, &TimeSharingChart::onTickUpdated);

    // 盘口、合约列表、逐笔成交通过 EventBus 更新
    auto& bus = EventBus::instance();
    connect(&bus, &EventBus::tickUpdated, this, [this](const QString& contract, double price, double volume) {
        auto tick = m_buffer->latestTick(contract);
        if (!tick.isValid()) return;
        m_quotePanel->updateQuote(tick);
        double changeRate = (tick.lastPrice - tick.preSettle) / tick.preSettle * 100.0;
        m_contractList->updatePrice(contract, tick.lastPrice, changeRate);
        m_tradeRecord->addRecord(tick, static_cast<int>(volume));
    });
}

void MainWindow::initDefaultContracts() {
    struct ContractInfo { QString code; QString name; };
    QVector<ContractInfo> defaults = {
        {"rb2510", "螺纹钢主连"}, {"IF2506", "沪深300主力"},
        {"sc2507", "原油主力"},   {"au2508", "黄金主力"},
        {"m2509",  "豆粕主力"},   {"TA510",  "PTA主力"},
        {"FG508",  "玻璃主力"},   {"SA509",  "纯碱主力"},
    };
    for (const auto& c : defaults) {
        m_contractList->addContract(c.code, c.name);
        m_dataProvider->subscribe(c.code);
        m_buffer->addContract(c.code);
    }
    m_klineChart->setContract("rb2510");
    m_timeChart->setContract("rb2510");
}

void MainWindow::openSettings() {
    SettingsDialog dlg(this);
    dlg.exec();
}

void MainWindow::openContractManager() {
    ContractManager dlg(this);
    dlg.exec();
}

void MainWindow::openAbout() {
    AboutDialog dlg(this);
    dlg.exec();
}
