#pragma once
#include <QPainter>
#include <QVector>
#include "data/KLineData.h"

class ChartAxis;

class ChartPainter {
public:
    explicit ChartPainter(QPainter* painter) : m_painter(painter) {}

    void drawBackground(const QRect& rect, const QColor& bgColor);
    void drawGrid(const ChartAxis& axis, int gridRows, int gridCols);
    void drawCandles(const QVector<KLineData>& klines, const ChartAxis& axis,
                     int startIndex, int visibleCount);
    void drawVolume(const QVector<KLineData>& klines, const ChartAxis& axis,
                    int startIndex, int visibleCount);
    void drawPriceLabels(const ChartAxis& axis, int labelCount = 5);

    void drawIndicatorOverlay(const QVector<double>& values, const ChartAxis& axis,
                              const QColor& color, int startIndex, int visibleCount);
    void drawIndicatorLine(const QVector<double>& values, int visibleCount, int startIndex,
                           const QColor& color, const QRect& subRect, double minVal, double maxVal);
    void drawIndicatorHistogram(const QVector<double>& values, int visibleCount, int startIndex,
                                const QColor& upColor, const QColor& downColor,
                                const QRect& subRect, double minVal, double maxVal);

    QColor upColor()   const { return QColor(239, 68, 68); }
    QColor downColor() const { return QColor(34, 197, 94); }

private:
    QPainter* m_painter;
    static constexpr int CANDLE_GAP = 1;
};
