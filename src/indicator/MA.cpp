#include "MA.h"

MA::MA() {}

void MA::calculate(const QVector<KLineData>& klines) {
    m_dataCount = klines.size();
    for (int i = 0; i < 4; ++i) {
        m_values[i].resize(m_dataCount);
        m_values[i].fill(0);
        calcMA(klines, m_periods[i], i);
    }
}

double MA::valueAt(int lineIndex, int dataIndex) const {
    if (lineIndex < 0 || lineIndex >= 4) return 0;
    if (dataIndex < 0 || dataIndex >= m_dataCount) return 0;
    return m_values[lineIndex].at(dataIndex);
}

QColor MA::lineColor(int lineIndex) const {
    return (lineIndex >= 0 && lineIndex < 4) ? m_colors[lineIndex] : QColor(255,255,255);
}

QString MA::lineName(int lineIndex) const {
    if (lineIndex >= 0 && lineIndex < 4)
        return QString("MA%1").arg(m_periods[lineIndex]);
    return "";
}

void MA::calcMA(const QVector<KLineData>& klines, int period, int lineIdx) {
    if (klines.size() < period) return;
    double sum = 0;
    for (int i = 0; i < period - 1; ++i) {
        sum += klines[i].close;
        m_values[lineIdx][i] = 0;
    }
    for (int i = period - 1; i < klines.size(); ++i) {
        sum += klines[i].close;
        if (i >= period) sum -= klines[i - period].close;
        m_values[lineIdx][i] = sum / period;
    }
}
