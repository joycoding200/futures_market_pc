#pragma once
#include <QWidget>
#include <QVector>
#include <QPointF>
#include "data/TickData.h"

class MarketDataBuffer;

class TimeSharingChart : public QWidget {
    Q_OBJECT
public:
    explicit TimeSharingChart(MarketDataBuffer* buffer, QWidget* parent = nullptr);
    void setContract(const QString& contract);

public slots:
    void onTickUpdated(const TickData& tick);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    MarketDataBuffer* m_buffer;
    QString m_contract;

    QVector<QPointF> m_pricePoints;
    QVector<QPointF> m_avgPoints;
    double m_preSettle = 0.0;
    double m_maxPrice = 0.0;
    double m_minPrice = 999999.0;
    double m_totalVolume = 0.0;
    double m_totalAmount = 0.0;

    QColor m_bgColor{22, 22, 30};
    QColor m_priceColor{255, 255, 255};
    QColor m_avgColor{255, 215, 0};
    QColor m_volumeColor{34, 197, 94, 50};
};
