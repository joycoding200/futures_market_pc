#pragma once
#include <QObject>
#include <QMap>
#include <QVector>
#include <deque>
#include "TickData.h"
#include "KLineData.h"

class MarketDataBuffer : public QObject {
    Q_OBJECT
public:
    explicit MarketDataBuffer(int maxItemsPerContract = 2000, QObject* parent = nullptr);

    void onTick(const TickData& tick);
    void onKLine(const KLineData& kline);

    TickData latestTick(const QString& contract) const;
    QVector<KLineData> klines(const QString& contract, KLineType type, int count = -1) const;
    KLineData latestKLine(const QString& contract, KLineType type) const;

    QStringList subscribedContracts() const;
    void addContract(const QString& contract);

signals:
    void tickUpdated(const TickData& tick);
    void klineUpdated(const KLineData& kline);

private:
    int m_maxItems;
    QMap<QString, TickData> m_latestTicks;
    QMap<QString, std::deque<KLineData>> m_klines;
    QStringList m_contracts;

    static QString makeKey(const QString& contract, KLineType type) {
        return contract + ":" + klineTypeToString(type);
    }
};
