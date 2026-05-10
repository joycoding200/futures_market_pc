#include "TimeSharingChart.h"
#include "data/MarketDataBuffer.h"
#include <QPainter>
#include <QDateTime>

TimeSharingChart::TimeSharingChart(MarketDataBuffer* buffer, QWidget* parent)
    : QWidget(parent), m_buffer(buffer) {
    setMinimumSize(400, 200);
}

void TimeSharingChart::setContract(const QString& contract) {
    m_contract = contract;
    m_pricePoints.clear();
    m_avgPoints.clear();
    update();
}

void TimeSharingChart::onTickUpdated(const TickData& tick) {
    if (tick.contract != m_contract) return;
    m_preSettle = tick.preSettle;
    m_totalVolume += tick.volume;
    m_totalAmount += tick.lastPrice * tick.volume;
    double avgPrice = m_totalAmount / std::max(1.0, m_totalVolume);

    double x = static_cast<double>(m_pricePoints.size());
    m_pricePoints.append(QPointF(x, tick.lastPrice));
    m_avgPoints.append(QPointF(x, avgPrice));

    m_maxPrice = std::max(m_maxPrice, tick.lastPrice);
    m_minPrice = std::min(m_minPrice, tick.lastPrice);
    update();
}

void TimeSharingChart::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), m_bgColor);

    if (m_pricePoints.isEmpty()) {
        painter.setPen(QColor(120, 120, 140));
        painter.drawText(rect(), Qt::AlignCenter, "等待分时数据...");
        return;
    }

    int w = width();
    int h = height();
    int margin = 40;
    double priceRange = std::max(m_maxPrice - m_minPrice, 1.0);

    auto priceToY = [&](double p) {
        return h - margin - (p - m_minPrice) / priceRange * (h - margin * 2);
    };
    auto idxToX = [&](int i) {
        if (m_pricePoints.size() <= 1) return w / 2.0;
        return margin + static_cast<double>(i) / (m_pricePoints.size() - 1) * (w - margin * 2);
    };

    if (m_preSettle > 0) {
        int baselineY = static_cast<int>(priceToY(m_preSettle));
        painter.setPen(QPen(QColor(255, 255, 255, 40), 1, Qt::DotLine));
        painter.drawLine(margin, baselineY, w - margin, baselineY);
    }

    painter.setPen(QPen(m_priceColor, 1));
    for (int i = 1; i < m_pricePoints.size(); ++i) {
        painter.drawLine(QPointF(idxToX(i-1), priceToY(m_pricePoints[i-1].y())),
                         QPointF(idxToX(i),   priceToY(m_pricePoints[i].y())));
    }

    painter.setPen(QPen(m_avgColor, 1, Qt::DashLine));
    for (int i = 1; i < m_avgPoints.size(); ++i) {
        painter.drawLine(QPointF(idxToX(i-1), priceToY(m_avgPoints[i-1].y())),
                         QPointF(idxToX(i),   priceToY(m_avgPoints[i].y())));
    }
}
