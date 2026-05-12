#pragma once
#include "IndicatorBase.h"

class MA : public IndicatorBase {
public:
    MA();

    QString name() const override { return "MA"; }
    IndicatorRenderType renderType() const override { return IndicatorRenderType::Overlay; }

    void calculate(const QVector<KLineData>& klines) override;

    int lineCount() const override { return 4; }
    double valueAt(int lineIndex, int dataIndex) const override;
    QColor lineColor(int lineIndex) const override;
    QString lineName(int lineIndex) const override;

private:
    void calcMA(const QVector<KLineData>& klines, int period, int lineIdx);

    QVector<double> m_values[4];
    int m_periods[4] = {5, 10, 20, 60};
    QColor m_colors[4] = {
        QColor(255, 255, 255),
        QColor(255, 215, 0),
        QColor(180, 130, 255),
        QColor(100, 149, 237)
    };
};
