#include "KLineAggregator.h"
#include "MarketDataBuffer.h"
#include <spdlog/spdlog.h>

KLineAggregator::KLineAggregator(MarketDataBuffer* buffer, QObject* parent)
    : QObject(parent), m_buffer(buffer) {}

void KLineAggregator::onTick(const TickData& tick) {
    static const KLineType types[] = {
        KLineType::M1, KLineType::M5, KLineType::M15, KLineType::M30,
        KLineType::H1, KLineType::D, KLineType::W, KLineType::M
    };
    for (auto type : types) {
        aggregateKLine(tick, type);
    }
}

void KLineAggregator::aggregateKLine(const TickData& tick, KLineType type) {
    QString key = tick.contract + ":" + klineTypeToString(type);
    int64_t periodStart = roundToPeriod(tick.timestamp, type);
    auto& current = m_currentKlines[static_cast<int>(type)][key];

    if (current.timestamp != periodStart) {
        if (current.timestamp > 0) {
            current.isComplete = true;
            m_buffer->onKLine(current);
        }
        current = KLineData{};
        current.contract = tick.contract;
        current.type = type;
        current.timestamp = periodStart;
        current.open = tick.lastPrice;
        current.high = tick.lastPrice;
        current.low = tick.lastPrice;
    }

    current.close = tick.lastPrice;
    current.high = std::max(current.high, tick.lastPrice);
    current.low = std::min(current.low, tick.lastPrice);
    current.volume += tick.volume;
    current.openInterest = tick.openInterest;
    current.isComplete = false;

    m_buffer->onKLine(current);
}

int64_t KLineAggregator::roundToPeriod(int64_t timestamp, KLineType type) {
    int minutes = periodMinutes(type);
    int64_t msPerMinute = 60000LL;
    int64_t periodMs = static_cast<int64_t>(minutes) * msPerMinute;

    if (type == KLineType::D) {
        QDateTime dt = QDateTime::fromMSecsSinceEpoch(timestamp);
        QDateTime dayStart(dt.date(), QTime(0, 0));
        return dayStart.toMSecsSinceEpoch();
    }
    return (timestamp / periodMs) * periodMs;
}

int KLineAggregator::periodMinutes(KLineType type) {
    switch (type) {
        case KLineType::M1:  return 1;
        case KLineType::M5:  return 5;
        case KLineType::M15: return 15;
        case KLineType::M30: return 30;
        case KLineType::H1:  return 60;
        case KLineType::D:   return 1440;
        case KLineType::W:   return 10080;
        case KLineType::M:   return 43200;
    }
    return 1440;
}
