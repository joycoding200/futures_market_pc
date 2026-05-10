#pragma once
#include "IndicatorBase.h"

class RSI : public IndicatorBase {
public:
    RSI();

    QString name() const override { return "RSI"; }
    IndicatorRenderType renderType() const override { return IndicatorRenderType::SubChart; }

    void calculate(const QVector<KLineData>& klines) override;

    int lineCount() const override { return 3; }
    double valueAt(int lineIndex, int dataIndex) const override;
    QColor lineColor(int lineIndex) const override;
    QString lineName(int lineIndex) const override;

    bool hasFixedRange() const override { return true; }
    double rangeMin() const override { return 0; }
    double rangeMax() const override { return 100; }

private:
    void calcRSI(const QVector<KLineData>& klines, int period, int lineIdx);

    QVector<double> m_values[3];
    int m_periods[3] = {6, 12, 24};
    QColor m_colors[3] = {
        QColor(255, 255, 255),
        QColor(255, 215, 0),
        QColor(180, 130, 255)
    };
};
