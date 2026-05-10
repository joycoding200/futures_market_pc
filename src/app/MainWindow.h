#pragma once
#include <QMainWindow>
#include <QSplitter>

class KLineChart;
class TimeSharingChart;
class QuotePanel;
class ContractList;
class TradeRecord;
class MarketDataBuffer;
class SimDataProvider;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(MarketDataBuffer* buffer, SimDataProvider* provider,
                        QWidget* parent = nullptr);

private:
    void setupUI();
    void setupConnections();
    void initDefaultContracts();

    MarketDataBuffer* m_buffer;
    SimDataProvider* m_dataProvider;

    ContractList*     m_contractList;
    KLineChart*       m_klineChart;
    TimeSharingChart* m_timeChart;
    QuotePanel*       m_quotePanel;
    TradeRecord*      m_tradeRecord;
};
