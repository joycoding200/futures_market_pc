#include "MarketDataBuffer.h"
#include <spdlog/spdlog.h>

MarketDataBuffer::MarketDataBuffer(int maxItemsPerContract, QObject* parent)
    : QObject(parent), m_maxItems(maxItemsPerContract) {}

void MarketDataBuffer::onTick(const TickData& tick) {
    if (!tick.isValid()) return;
    m_latestTicks[tick.contract] = tick;
    emit tickUpdated(tick);
}

void MarketDataBuffer::onKLine(const KLineData& kline) {
    QString key = makeKey(kline.contract, kline.type);
    auto& deq = m_klines[key];

    if (!deq.empty() && deq.back().timestamp == kline.timestamp) {
        deq.back() = kline;
    } else {
        deq.push_back(kline);
    }

    while (static_cast<int>(deq.size()) > m_maxItems) {
        deq.pop_front();
    }
    emit klineUpdated(kline);
}

TickData MarketDataBuffer::latestTick(const QString& contract) const {
    return m_latestTicks.value(contract);
}

QVector<KLineData> MarketDataBuffer::klines(const QString& contract, KLineType type, int count) const {
    QString key = makeKey(contract, type);
    const auto& deq = m_klines.value(key);
    if (count < 0 || count >= static_cast<int>(deq.size())) {
        return QVector<KLineData>(deq.begin(), deq.end());
    }
    auto start = deq.end() - count;
    return QVector<KLineData>(start, deq.end());
}

KLineData MarketDataBuffer::latestKLine(const QString& contract, KLineType type) const {
    QString key = makeKey(contract, type);
    const auto& deq = m_klines.value(key);
    return deq.empty() ? KLineData() : deq.back();
}

QStringList MarketDataBuffer::subscribedContracts() const { return m_contracts; }
void MarketDataBuffer::addContract(const QString& contract) {
    if (!m_contracts.contains(contract)) m_contracts.append(contract);
}
