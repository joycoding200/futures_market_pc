#pragma once
#include <QMainWindow>
#include <QStatusBar>

class KLineChart;
class TimeSharingChart;
class QuotePanel;
class ContractList;
class TradeRecord;
class MarketDataBuffer;
class MarketDataProvider;
class StatsCalculator;
class StatsPanel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(MarketDataBuffer* buffer, MarketDataProvider* provider,
                        QWidget* parent = nullptr);

private:
    void setupUI();
    void setupConnections();
    void initDefaultContracts();
    void openSettings();
    void openContractManager();
    void openAbout();

    MarketDataBuffer* m_buffer;
    MarketDataProvider* m_dataProvider;

    ContractList*     m_contractList;
    KLineChart*       m_klineChart;
    TimeSharingChart* m_timeChart;
    QuotePanel*       m_quotePanel;
    TradeRecord*      m_tradeRecord;
    QStatusBar*       m_statusBar;
    StatsCalculator*  m_statsCalc = nullptr;
    StatsPanel*       m_statsPanel = nullptr;
};
