#pragma once
#include <QTimer>
#include <QRandomGenerator>
#include <QMap>
#include "MarketDataProvider.h"
#include "KLineAggregator.h"

class SimDataProvider : public MarketDataProvider {
    Q_OBJECT
public:
    explicit SimDataProvider(QObject* parent = nullptr);
    ~SimDataProvider() override;

    void subscribe(const QString& contract) override;
    void unsubscribe(const QString& contract) override;
    void subscribeAll(const QVector<QString>& contracts) override;
    void requestHistory(const QString& contract, KLineType type,
                        int count) override;
    void start() override;
    void stop() override;
    QString providerName() const override { return "Simulated"; }

private slots:
    void onTimerTick();

private:
    TickData generateTick(const QString& contract);
    double randomWalk(double currentPrice, double volatility, double min, double max);

    QTimer* m_timer;
    QMap<QString, double> m_prices;
    QMap<QString, double> m_opens;
    QRandomGenerator m_rng;
    static constexpr int DEFAULT_INTERVAL_MS = 250;
    static constexpr double DEFAULT_VOLATILITY = 2.0;
};
