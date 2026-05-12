#include "KLineChart.h"
#include "data/MarketDataBuffer.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <cmath>
#include "indicator/MA.h"
#include "indicator/MACD.h"
#include "indicator/KDJ.h"
#include "indicator/RSI.h"

KLineChart::KLineChart(MarketDataBuffer* buffer, QWidget* parent)
    : QWidget(parent), m_buffer(buffer) {
    setMouseTracking(true);
    setMinimumSize(600, 400);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setupIndicators();
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
    ChartPainter pf(&painter);
    recalcAxis();

    auto klines = m_buffer->klines(m_contract, m_period);
    if (klines.isEmpty()) {
        painter.fillRect(rect(), m_bgColor);
        painter.setPen(QColor(120, 120, 140));
        painter.drawText(rect(), Qt::AlignCenter, "等待行情数据...");
        return;
    }

    for (auto* ind : m_indicators) {
        ind->calculate(klines);
    }

    int mainChartH = m_subChartHeight > 0 ? height() - m_subChartHeight : height();

    pf.drawBackground(QRect(0, 0, width(), mainChartH), m_bgColor);

    int volStartY = mainChartH * 3 / 4;
    painter.fillRect(QRect(0, volStartY, width(), mainChartH - volStartY), m_volumeAreaBg);

    m_axis.chartHeight = mainChartH;
    pf.drawGrid(m_axis, 5, 8);
    pf.drawCandles(klines, m_axis, m_startIndex, m_visibleCount);
    pf.drawVolume(klines, m_axis, m_startIndex, m_visibleCount);

    // Draw Overlay indicators (MA lines on main chart)
    for (auto* ind : m_indicators) {
        if (ind->renderType() == IndicatorRenderType::Overlay) {
            for (int li = 0; li < ind->lineCount(); ++li) {
                QVector<double> vals(klines.size());
                for (int i = 0; i < klines.size(); ++i) vals[i] = ind->valueAt(li, i);
                pf.drawIndicatorOverlay(vals, m_axis, ind->lineColor(li), m_startIndex, m_visibleCount);
            }
        }
    }

    pf.drawPriceLabels(m_axis, 5);

    // Draw SubChart indicator
    if (m_activeSubIndicator && m_subChartHeight > 0) {
        QRect subRect(0, mainChartH + 1, width(), m_subChartHeight - 22);

        painter.setPen(QPen(QColor(60, 60, 80), 1));
        painter.drawLine(0, mainChartH, width(), mainChartH);
        painter.fillRect(subRect, QColor(18, 18, 28));

        painter.setPen(QColor(150, 150, 170));
        QFont labelFont("Arial", 9);
        painter.setFont(labelFont);
        painter.drawText(subRect.left() + 6, subRect.top() + 12, m_activeSubIndicator->name());

        for (int li = 0; li < m_activeSubIndicator->lineCount(); ++li) {
            QVector<double> vals(klines.size());
            for (int i = 0; i < klines.size(); ++i) vals[i] = m_activeSubIndicator->valueAt(li, i);

            int endIdx = std::min(m_startIndex + m_visibleCount, (int)vals.size());
            double rMin = m_activeSubIndicator->hasFixedRange() ? m_activeSubIndicator->rangeMin()
                         : *std::min_element(vals.begin() + m_startIndex, vals.begin() + endIdx);
            double rMax = m_activeSubIndicator->hasFixedRange() ? m_activeSubIndicator->rangeMax()
                         : *std::max_element(vals.begin() + m_startIndex, vals.begin() + endIdx);
            if (rMax == rMin) rMax = rMin + 1;

            if (m_activeSubIndicator->isHistogram(li)) {
                pf.drawIndicatorHistogram(vals, m_visibleCount, m_startIndex,
                    pf.upColor(), pf.downColor(), subRect, rMin, rMax);
            } else {
                pf.drawIndicatorLine(vals, m_visibleCount, m_startIndex,
                    m_activeSubIndicator->lineColor(li), subRect, rMin, rMax);
            }
        }
    }

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
        auto klines = m_buffer->klines(m_contract, m_period);
        int maxStart = std::max(0, klines.size() - m_visibleCount);
        m_startIndex = std::clamp(m_dragStartIndex + indexDelta, 0, maxStart);
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

void KLineChart::setupIndicators() {
    m_indicators.append(new MA());
    m_indicators.append(new MACD());
    m_indicators.append(new KDJ());
    m_indicators.append(new RSI());
    m_activeSubIndicator = m_indicators[1];  // MACD
    m_subChartHeight = 120;
}

void KLineChart::setSubIndicator(IndicatorBase* indicator) {
    if (indicator && indicator->renderType() == IndicatorRenderType::SubChart) {
        m_activeSubIndicator = indicator;
        update();
    }
}
