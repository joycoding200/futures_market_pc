#pragma once
#include <QVector>
#include "data/KLineData.h"

struct ChartAxis {
    double priceMin = 0.0;
    double priceMax = 0.0;
    double volumeMax = 0.0;

    int chartHeight = 600;
    int chartWidth = 800;

    void calcPriceRange(const QVector<KLineData>& visibleKlines);
    int priceToY(double price, double margin = 0.1) const;
    int volumeToY(double volume) const;
    double klineIndexToX(int index, int visibleCount) const;
    int pixelXToIndex(double pixelX, int visibleCount, int startIndex) const;

    static constexpr int RIGHT_MARGIN = 60;
    static constexpr int BOTTOM_MARGIN = 30;
};
