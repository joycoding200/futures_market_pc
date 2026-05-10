#include "MainWindow.h"
#include "chart/KLineChart.h"
#include "chart/TimeSharingChart.h"
#include "chart/QuotePanel.h"
#include "chart/ContractList.h"
#include "chart/TradeRecord.h"
#include "data/MarketDataBuffer.h"
#include "data/SimDataProvider.h"
#include "core/EventBus.h"
#include <QTabWidget>
#include <QHBoxLayout>

MainWindow::MainWindow(MarketDataBuffer* buffer, SimDataProvider* provider, QWidget* parent)
    : QMainWindow(parent), m_buffer(buffer), m_dataProvider(provider) {
    setWindowTitle("期货行情PC软件 - Phase 1");
    resize(1400, 900);
    setStyleSheet("QMainWindow { background: #12121a; }");
    setupUI();
    setupConnections();
    initDefaultContracts();
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

    rightPanel->setFixedWidth(280);
    mainLayout->addWidget(rightPanel);

    setCentralWidget(centralWidget);
}

void MainWindow::setupConnections() {
    connect(m_contractList, &ContractList::contractClicked, this, [this](const QString& contract) {
        m_klineChart->setContract(contract);
        m_timeChart->setContract(contract);
    });

    auto& bus = EventBus::instance();
    connect(&bus, &EventBus::tickUpdated, this, [this](const QString& contract, double price, double volume) {
        auto tick = m_buffer->latestTick(contract);
        m_quotePanel->updateQuote(tick);
        m_contractList->updatePrice(contract, tick.lastPrice,
            (tick.lastPrice - tick.preSettle) / tick.preSettle * 100.0);
        bool isBuy = (price > tick.preSettle);
        m_tradeRecord->addRecord(tick, static_cast<int>(volume), isBuy);
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
