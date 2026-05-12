#pragma once
#include "IndicatorBase.h"

class MACD : public IndicatorBase {
public:
    MACD(int fast = 12, int slow = 26, int signal = 9);

    QString name() const override { return "MACD"; }
    IndicatorRenderType renderType() const override { return IndicatorRenderType::SubChart; }

    void calculate(const QVector<KLineData>& klines) override;

    int lineCount() const override { return 3; }
    double valueAt(int lineIndex, int dataIndex) const override;
    QColor lineColor(int lineIndex) const override;
    QString lineName(int lineIndex) const override;

    bool isHistogram(int lineIndex) const override { return lineIndex == 0; }

private:
    int m_fast, m_slow, m_signal;
    QVector<double> m_bar;
    QVector<double> m_dif;
    QVector<double> m_dea;
};
