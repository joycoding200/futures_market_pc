#include "ChartAxis.h"
#include <algorithm>
#include <cmath>

void ChartAxis::calcPriceRange(const QVector<KLineData>& visibleKlines) {
    if (visibleKlines.isEmpty()) return;
    priceMax = visibleKlines.first().high;
    priceMin = visibleKlines.first().low;
    volumeMax = visibleKlines.first().volume;
    for (const auto& k : visibleKlines) {
        priceMax = std::max(priceMax, k.high);
        priceMin = std::min(priceMin, k.low);
        volumeMax = std::max(volumeMax, k.volume);
    }
    if (priceMax == priceMin) priceMax += 1.0;
}

int ChartAxis::priceToY(double price, double margin) const {
    double range = priceMax - priceMin;
    double padding = range * margin;
    double adjustedMax = priceMax + padding;
    double adjustedMin = priceMin - padding;
    double ratio = (price - adjustedMin) / (adjustedMax - adjustedMin);
    return static_cast<int>((chartHeight - BOTTOM_MARGIN) * (1.0 - ratio));
}

int ChartAxis::volumeToY(double volume) const {
    if (volumeMax <= 0) return chartHeight - BOTTOM_MARGIN;
    int volumeAreaHeight = chartHeight / 4;
    double ratio = volume / volumeMax;
    return chartHeight - BOTTOM_MARGIN - static_cast<int>(ratio * volumeAreaHeight);
}

double ChartAxis::klineIndexToX(int index, int visibleCount) const {
    double candleWidth = static_cast<double>(chartWidth - RIGHT_MARGIN) / visibleCount;
    return index * candleWidth;
}

int ChartAxis::pixelXToIndex(double pixelX, int visibleCount, int startIndex) const {
    double candleWidth = static_cast<double>(chartWidth - RIGHT_MARGIN) / visibleCount;
    return startIndex + static_cast<int>(pixelX / candleWidth);
}
