#pragma once
#include <QObject>
#include <QMap>
#include "TickData.h"
#include "KLineData.h"

class MarketDataBuffer;

class KLineAggregator : public QObject {
    Q_OBJECT
public:
    explicit KLineAggregator(MarketDataBuffer* buffer, QObject* parent = nullptr);

public slots:
    void onTick(const TickData& tick);

private:
    void aggregateKLine(const TickData& tick, KLineType type);
    int64_t roundToPeriod(int64_t timestamp, KLineType type);
    int periodMinutes(KLineType type);

    MarketDataBuffer* m_buffer;
    QMap<QString, KLineData> m_currentKlines[8];
};
