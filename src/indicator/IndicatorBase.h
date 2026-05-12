#pragma once
#include <QString>
#include <QColor>
#include <QVector>
#include "data/KLineData.h"

enum class IndicatorRenderType {
    Overlay,
    SubChart
};

class IndicatorBase {
public:
    virtual ~IndicatorBase() = default;

    virtual QString name() const = 0;
    virtual IndicatorRenderType renderType() const = 0;

    virtual void calculate(const QVector<KLineData>& klines) = 0;

    virtual int lineCount() const = 0;

    virtual double valueAt(int lineIndex, int dataIndex) const = 0;

    virtual QColor lineColor(int lineIndex) const = 0;
    virtual QString lineName(int lineIndex) const = 0;

    virtual bool isHistogram(int lineIndex) const { return false; }

    virtual bool hasFixedRange() const { return false; }
    virtual double rangeMin() const { return 0; }
    virtual double rangeMax() const { return 0; }

    int dataCount() const { return m_dataCount; }

protected:
    int m_dataCount = 0;
};
