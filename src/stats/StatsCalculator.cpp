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
    if (s.preSettle > 0) {
        s.amplitude = (s.highPrice - s.lowPrice) / s.preSettle * 100.0;
    }
    if (s.exchange.isEmpty()) s.exchange = extractExchange(tick.contract);

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
    if (count < list.size()) list.resize(count);
    return list;
}

QVector<ContractStats> StatsCalculator::topLosers(int count) const {
    auto list = m_stats.values().toVector();
    std::sort(list.begin(), list.end(),
              [](const ContractStats& a, const ContractStats& b) { return a.changeRate < b.changeRate; });
    if (count < list.size()) list.resize(count);
    return list;
}

QVector<ContractStats> StatsCalculator::topVolume(int count) const {
    auto list = m_stats.values().toVector();
    std::sort(list.begin(), list.end(),
              [](const ContractStats& a, const ContractStats& b) { return a.volume > b.volume; });
    if (count < list.size()) list.resize(count);
    return list;
}

QVector<ContractStats> StatsCalculator::allStats(SortField sortBy, Qt::SortOrder order) const {
    auto list = m_stats.values().toVector();
    std::sort(list.begin(), list.end(), [=](const ContractStats& a, const ContractStats& b) {
        bool lessThan = false;
        switch (sortBy) {
            case SortField::Contract:   lessThan = a.contract < b.contract; break;
            case SortField::Price:      lessThan = a.lastPrice < b.lastPrice; break;
            case SortField::ChangeRate: lessThan = a.changeRate < b.changeRate; break;
            case SortField::Volume:     lessThan = a.volume < b.volume; break;
            case SortField::Amplitude:  lessThan = a.amplitude < b.amplitude; break;
        }
        return order == Qt::DescendingOrder ? !lessThan : lessThan;
    });
    return list;
}

ContractStats StatsCalculator::statsFor(const QString& contract) const {
    return m_stats.value(contract);
}

QVector<ContractStats> StatsCalculator::filter(const QString& keyword, const QString& exchange, int direction) const {
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
