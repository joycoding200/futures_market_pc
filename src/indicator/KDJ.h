#pragma once
#include "IndicatorBase.h"

class KDJ : public IndicatorBase {
public:
    KDJ(int rsvPeriod = 9, int kSmooth = 3, int dSmooth = 3);

    QString name() const override { return "KDJ"; }
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
    int m_n, m_kSmooth, m_dSmooth;
    QVector<double> m_k, m_d, m_j;
};
