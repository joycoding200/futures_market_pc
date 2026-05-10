#include "ChartPainter.h"
#include "ChartAxis.h"

void ChartPainter::drawBackground(const QRect& rect, const QColor& bgColor) {
    m_painter->fillRect(rect, bgColor);
}

void ChartPainter::drawGrid(const ChartAxis& axis, int gridRows, int gridCols) {
    QPen gridPen(QColor(40, 40, 50, 80), 1, Qt::DotLine);
    m_painter->setPen(gridPen);
    for (int i = 0; i <= gridRows; ++i) {
        int y = i * (axis.chartHeight - axis.BOTTOM_MARGIN) / gridRows;
        m_painter->drawLine(0, y, axis.chartWidth - axis.RIGHT_MARGIN, y);
    }
}

void ChartPainter::drawCandles(const QVector<KLineData>& klines, const ChartAxis& axis,
                                int startIndex, int visibleCount) {
    double barWidth = static_cast<double>(axis.chartWidth - axis.RIGHT_MARGIN) / visibleCount - CANDLE_GAP * 2;
    if (barWidth < 1.0) barWidth = 1.0;

    for (int i = 0; i < visibleCount && (startIndex + i) < klines.size(); ++i) {
        const auto& k = klines[startIndex + i];
        double x = axis.klineIndexToX(i, visibleCount) + CANDLE_GAP;

        int highY = axis.priceToY(k.high);
        int lowY  = axis.priceToY(k.low);
        int openY = axis.priceToY(k.open);
        int closeY = axis.priceToY(k.close);
        int candleTop = std::min(openY, closeY);
        int candleBody = std::abs(closeY - openY);
        if (candleBody == 0) candleBody = 1;

        QColor color = k.isRising() ? upColor() : downColor();
        m_painter->setPen(QPen(color, 1));
        m_painter->setBrush(color);

        m_painter->drawLine(QPointF(x + barWidth / 2, highY),
                            QPointF(x + barWidth / 2, candleTop));
        m_painter->drawLine(QPointF(x + barWidth / 2, candleTop + candleBody),
                            QPointF(x + barWidth / 2, lowY));

        if (k.isRising()) {
            m_painter->setBrush(Qt::NoBrush);
            m_painter->drawRect(QRectF(x, candleTop, barWidth, candleBody));
        } else {
            m_painter->setBrush(color);
            m_painter->drawRect(QRectF(x, candleTop, barWidth, candleBody));
        }
    }
}

void ChartPainter::drawVolume(const QVector<KLineData>& klines, const ChartAxis& axis,
                               int startIndex, int visibleCount) {
    double barWidth = static_cast<double>(axis.chartWidth - axis.RIGHT_MARGIN) / visibleCount - CANDLE_GAP * 2;
    if (barWidth < 1.0) barWidth = 1.0;

    for (int i = 0; i < visibleCount && (startIndex + i) < klines.size(); ++i) {
        const auto& k = klines[startIndex + i];
        double x = axis.klineIndexToX(i, visibleCount) + CANDLE_GAP;
        int volumeBottom = axis.volumeToY(0);
        int volumeTop = axis.volumeToY(k.volume);
        int volumeHeight = volumeBottom - volumeTop;

        QColor color = k.isRising() ? upColor() : downColor();
        color.setAlpha(120);
        m_painter->setPen(Qt::NoPen);
        m_painter->setBrush(color);
        m_painter->drawRect(QRectF(x, volumeTop, barWidth, std::max(1, volumeHeight)));
    }
}

void ChartPainter::drawPriceLabels(const ChartAxis& axis, int labelCount) {
    m_painter->setPen(QColor(180, 180, 190));
    QFont font("Consolas", 9);
    m_painter->setFont(font);
    for (int i = 0; i <= labelCount; ++i) {
        double price = axis.priceMin + (axis.priceMax - axis.priceMin) * (labelCount - i) / labelCount;
        int y = axis.priceToY(price);
        m_painter->drawText(axis.chartWidth - axis.RIGHT_MARGIN + 5, y + 4,
                            QString::number(price, 'f', 0));
    }
}
