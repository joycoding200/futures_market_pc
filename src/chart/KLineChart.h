#pragma once
#include <QWidget>
#include <QVector>
#include "data/KLineData.h"
#include "data/TickData.h"
#include "ChartAxis.h"
#include "ChartPainter.h"
#include "Crosshair.h"
#include "indicator/IndicatorBase.h"

class MarketDataBuffer;

class KLineChart : public QWidget {
    Q_OBJECT
public:
    explicit KLineChart(MarketDataBuffer* buffer, QWidget* parent = nullptr);

    void setPeriod(KLineType type);
    KLineType period() const { return m_period; }
    void setContract(const QString& contract);
    void setupIndicators();
    void setSubIndicator(IndicatorBase* indicator);

signals:
    void crosshairInfo(const QString& info);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

public slots:
    void onTickUpdated(const TickData& tick);
    void onKLineUpdated(const KLineData& kline);

private:
    void recalcAxis();
    void drawAll(QPainter& painter);

    MarketDataBuffer* m_buffer;
    QString m_contract;
    KLineType m_period = KLineType::D;

    int m_startIndex = 0;
    int m_visibleCount = 120;

    ChartAxis m_axis;
    Crosshair m_crosshair;

    bool m_isDragging = false;
    QPoint m_dragStart;
    int m_dragStartIndex = 0;

    QColor m_bgColor{22, 22, 30};
    QColor m_volumeAreaBg{25, 25, 35};
    QVector<IndicatorBase*> m_indicators;
    IndicatorBase* m_activeSubIndicator = nullptr;
    int m_subChartHeight = 0;
};
