#pragma once
#include <QString>
#include <cstdint>

enum class KLineType {
    M1, M5, M15, M30, H1, D, W, M
};

inline QString klineTypeToString(KLineType t) {
    switch (t) {
        case KLineType::M1:  return "1m";
        case KLineType::M5:  return "5m";
        case KLineType::M15: return "15m";
        case KLineType::M30: return "30m";
        case KLineType::H1:  return "1H";
        case KLineType::D:   return "D";
        case KLineType::W:   return "W";
        case KLineType::M:   return "M";
    }
    return "D";
}

struct KLineData {
    QString   contract;
    double    open        = 0.0;
    double    high        = 0.0;
    double    low         = 0.0;
    double    close       = 0.0;
    double    volume      = 0.0;
    double    openInterest = 0.0;
    int64_t   timestamp   = 0;
    KLineType type        = KLineType::D;
    bool      isComplete  = false;

    bool isRising() const { return close >= open; }
    double range() const { return high - low; }
};

inline bool operator<(const KLineData& a, const KLineData& b) {
    return a.timestamp < b.timestamp;
};
