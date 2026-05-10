#include "KLineChart.h"
#include "data/MarketDataBuffer.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <cmath>

KLineChart::KLineChart(MarketDataBuffer* buffer, QWidget* parent)
    : QWidget(parent), m_buffer(buffer), m_painterFn(nullptr) {
    setMouseTracking(true);
    setMinimumSize(600, 400);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void KLineChart::setPeriod(KLineType type) {
    m_period = type;
    m_startIndex = 0;
    update();
}

void KLineChart::setContract(const QString& contract) {
    m_contract = contract;
    m_startIndex = 0;
    update();
}

void KLineChart::onTickUpdated(const TickData& tick) {
    if (tick.contract != m_contract) return;
    update();
}

void KLineChart::onKLineUpdated(const KLineData& kline) {
    if (kline.contract != m_contract || kline.type != m_period) return;
    update();
}

void KLineChart::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    drawAll(painter);
}

void KLineChart::recalcAxis() {
    m_axis.chartHeight = height();
    m_axis.chartWidth = width();
    auto klines = m_buffer->klines(m_contract, m_period);
    if (klines.isEmpty()) return;

    int totalCount = klines.size();
    m_startIndex = std::clamp(m_startIndex, 0, std::max(0, totalCount - m_visibleCount));

    int endIdx = std::min(m_startIndex + m_visibleCount, totalCount);
    QVector<KLineData> visible(
        klines.begin() + m_startIndex,
        klines.begin() + endIdx);
    m_axis.calcPriceRange(visible);
}

void KLineChart::drawAll(QPainter& painter) {
    m_painterFn = ChartPainter(&painter);
    recalcAxis();

    auto klines = m_buffer->klines(m_contract, m_period);
    if (klines.isEmpty()) {
        painter.fillRect(rect(), m_bgColor);
        painter.setPen(QColor(120, 120, 140));
        painter.drawText(rect(), Qt::AlignCenter, "等待行情数据...");
        return;
    }

    m_painterFn.drawBackground(QRect(0, 0, m_axis.chartWidth, m_axis.chartHeight), m_bgColor);

    int volStartY = m_axis.chartHeight * 3 / 4;
    painter.fillRect(QRect(0, volStartY, m_axis.chartWidth, m_axis.chartHeight - volStartY), m_volumeAreaBg);

    m_painterFn.drawGrid(m_axis, 5, 8);
    m_painterFn.drawCandles(klines, m_axis, m_startIndex, m_visibleCount);
    m_painterFn.drawVolume(klines, m_axis, m_startIndex, m_visibleCount);
    m_painterFn.drawPriceLabels(m_axis, 5);
    m_crosshair.draw(&painter, rect());
}

void KLineChart::wheelEvent(QWheelEvent* event) {
    bool scrollUp = event->angleDelta().y() > 0;
    if (scrollUp) {
        m_visibleCount = std::max(10, m_visibleCount - 10);
    } else {
        m_visibleCount = std::min(500, m_visibleCount + 10);
    }
    update();
}

void KLineChart::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        m_dragStart = event->pos();
        m_dragStartIndex = m_startIndex;
        setCursor(Qt::ClosedHandCursor);
    }
}

void KLineChart::mouseMoveEvent(QMouseEvent* event) {
    if (m_isDragging) {
        int dx = event->pos().x() - m_dragStart.x();
        double candleWidth = static_cast<double>(m_axis.chartWidth - m_axis.RIGHT_MARGIN) / m_visibleCount;
        int indexDelta = -static_cast<int>(dx / candleWidth);
        m_startIndex = std::clamp(m_dragStartIndex + indexDelta, 0, m_startIndex);
        update();
    } else {
        m_crosshair.moveTo(event->pos(), rect());
        int idx = m_axis.pixelXToIndex(event->pos().x(), m_visibleCount, m_startIndex);
        auto klines = m_buffer->klines(m_contract, m_period);
        if (idx >= 0 && idx < klines.size()) {
            const auto& k = klines[idx];
            emit crosshairInfo(QString("O:%1 H:%2 L:%3 C:%4 V:%5")
                .arg(k.open, 0, 'f', 0).arg(k.high, 0, 'f', 0)
                .arg(k.low, 0, 'f', 0).arg(k.close, 0, 'f', 0)
                .arg(k.volume, 0, 'f', 0));
        }
        update();
    }
}

void KLineChart::mouseReleaseEvent(QMouseEvent*) {
    m_isDragging = false;
    m_crosshair.hide();
    setCursor(Qt::ArrowCursor);
    update();
}

void KLineChart::resizeEvent(QResizeEvent*) { update(); }
