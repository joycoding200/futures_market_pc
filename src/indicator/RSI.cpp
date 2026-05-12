#include "RSI.h"

RSI::RSI() {}

void RSI::calculate(const QVector<KLineData>& klines) {
    m_dataCount = klines.size();
    for (int i = 0; i < 3; ++i) {
        m_values[i].resize(m_dataCount);
        m_values[i].fill(50);
        calcRSI(klines, m_periods[i], i);
    }
}

double RSI::valueAt(int lineIndex, int dataIndex) const {
    if (lineIndex < 0 || lineIndex >= 3 || dataIndex < 0 || dataIndex >= m_dataCount) return 0;
    return m_values[lineIndex].at(dataIndex);
}

QColor RSI::lineColor(int lineIndex) const {
    return (lineIndex >= 0 && lineIndex < 3) ? m_colors[lineIndex] : QColor(255,255,255);
}

QString RSI::lineName(int lineIndex) const {
    if (lineIndex >= 0 && lineIndex < 3)
        return QString("RSI%1").arg(m_periods[lineIndex]);
    return "";
}

void RSI::calcRSI(const QVector<KLineData>& klines, int period, int lineIdx) {
    if (klines.size() < period + 1) return;

    double avgGain = 0, avgLoss = 0;
    for (int i = 1; i <= period; ++i) {
        double diff = klines[i].close - klines[i - 1].close;
        if (diff >= 0) avgGain += diff;
        else avgLoss -= diff;
    }
    avgGain /= period;
    avgLoss /= period;

    m_values[lineIdx][period] = (avgLoss == 0) ? 100.0
        : 100.0 - 100.0 / (1.0 + avgGain / avgLoss);

    for (int i = period + 1; i < m_dataCount; ++i) {
        double diff = klines[i].close - klines[i - 1].close;
        double gain = (diff > 0) ? diff : 0;
        double loss = (diff < 0) ? -diff : 0;
        avgGain = (avgGain * (period - 1) + gain) / period;
        avgLoss = (avgLoss * (period - 1) + loss) / period;
        m_values[lineIdx][i] = (avgLoss == 0) ? 100.0
            : 100.0 - 100.0 / (1.0 + avgGain / avgLoss);
    }
}
