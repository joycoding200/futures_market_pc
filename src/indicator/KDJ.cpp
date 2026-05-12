#include "KDJ.h"
#include <algorithm>

KDJ::KDJ(int rsvPeriod, int kSmooth, int dSmooth)
    : m_n(rsvPeriod), m_kSmooth(kSmooth), m_dSmooth(dSmooth) {}

void KDJ::calculate(const QVector<KLineData>& klines) {
    m_dataCount = klines.size();
    m_k.resize(m_dataCount); m_k.fill(50);
    m_d.resize(m_dataCount); m_d.fill(50);
    m_j.resize(m_dataCount); m_j.fill(50);
    if (klines.size() < m_n) return;

    for (int i = m_n - 1; i < m_dataCount; ++i) {
        double high = klines[i].high, low = klines[i].low;
        for (int j = i - m_n + 1; j <= i; ++j) {
            high = std::max(high, klines[j].high);
            low = std::min(low, klines[j].low);
        }
        double rsv = (high == low) ? 50.0 : (klines[i].close - low) / (high - low) * 100.0;

        if (i == m_n - 1) {
            m_k[i] = rsv;
            m_d[i] = rsv;
        } else {
            m_k[i] = (m_kSmooth - 1.0) / m_kSmooth * m_k[i - 1] + 1.0 / m_kSmooth * rsv;
            m_d[i] = (m_dSmooth - 1.0) / m_dSmooth * m_d[i - 1] + 1.0 / m_dSmooth * m_k[i];
        }
        m_j[i] = 3.0 * m_k[i] - 2.0 * m_d[i];
    }
}

double KDJ::valueAt(int lineIndex, int dataIndex) const {
    if (dataIndex < 0 || dataIndex >= m_dataCount) return 0;
    switch (lineIndex) {
        case 0: return m_k.at(dataIndex);
        case 1: return m_d.at(dataIndex);
        case 2: return m_j.at(dataIndex);
    }
    return 0;
}

QColor KDJ::lineColor(int lineIndex) const {
    switch (lineIndex) {
        case 0: return QColor(255, 255, 255);
        case 1: return QColor(255, 215, 0);
        case 2: return QColor(180, 130, 255);
    }
    return QColor(255,255,255);
}

QString KDJ::lineName(int lineIndex) const {
    switch (lineIndex) {
        case 0: return "K";
        case 1: return "D";
        case 2: return "J";
    }
    return "";
}
