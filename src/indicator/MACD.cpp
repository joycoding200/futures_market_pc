#include "MACD.h"

MACD::MACD(int fast, int slow, int signal)
    : m_fast(fast), m_slow(slow), m_signal(signal) {}

void MACD::calculate(const QVector<KLineData>& klines) {
    m_dataCount = klines.size();
    m_dif.resize(m_dataCount);
    m_dea.resize(m_dataCount);
    m_bar.resize(m_dataCount);
    m_dif.fill(0);
    m_dea.fill(0);
    m_bar.fill(0);
    if (klines.size() < m_slow) return;

    QVector<double> emaFast(m_dataCount, 0);
    QVector<double> emaSlow(m_dataCount, 0);
    emaFast[m_slow - 1] = klines[m_slow - 1].close;
    emaSlow[m_slow - 1] = klines[m_slow - 1].close;

    double fastAlpha = 2.0 / (m_fast + 1);
    double slowAlpha = 2.0 / (m_slow + 1);
    for (int i = m_slow; i < m_dataCount; ++i) {
        emaFast[i] = klines[i].close * fastAlpha + emaFast[i - 1] * (1 - fastAlpha);
        emaSlow[i] = klines[i].close * slowAlpha + emaSlow[i - 1] * (1 - slowAlpha);
    }

    for (int i = 0; i < m_dataCount; ++i) {
        m_dif[i] = emaFast[i] - emaSlow[i];
    }

    int deaStart = m_slow + m_signal - 1;
    if (deaStart < m_dataCount) {
        m_dea[deaStart] = m_dif[deaStart];
        double sigAlpha = 2.0 / (m_signal + 1);
        for (int i = deaStart + 1; i < m_dataCount; ++i) {
            m_dea[i] = m_dif[i] * sigAlpha + m_dea[i - 1] * (1 - sigAlpha);
        }
    }

    for (int i = 0; i < m_dataCount; ++i) {
        m_bar[i] = 2.0 * (m_dif[i] - m_dea[i]);
    }
}

double MACD::valueAt(int lineIndex, int dataIndex) const {
    if (dataIndex < 0 || dataIndex >= m_dataCount) return 0;
    switch (lineIndex) {
        case 0: return m_bar.at(dataIndex);
        case 1: return m_dif.at(dataIndex);
        case 2: return m_dea.at(dataIndex);
    }
    return 0;
}

QColor MACD::lineColor(int lineIndex) const {
    switch (lineIndex) {
        case 0: return QColor(255, 255, 255);
        case 1: return QColor(255, 255, 255);
        case 2: return QColor(255, 215, 0);
    }
    return QColor(255,255,255);
}

QString MACD::lineName(int lineIndex) const {
    switch (lineIndex) {
        case 0: return "BAR";
        case 1: return "DIF";
        case 2: return "DEA";
    }
    return "";
}
