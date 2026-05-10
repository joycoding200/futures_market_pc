#pragma once
#include <QString>
#include <cstdint>
#include <array>

struct TickData {
    QString   contract;
    double    lastPrice    = 0.0;
    double    openPrice    = 0.0;
    double    highPrice    = 0.0;
    double    lowPrice     = 0.0;
    double    volume       = 0.0;
    double    openInterest = 0.0;
    double    preSettle    = 0.0;

    std::array<double, 5> bidPrice  = {};
    std::array<double, 5> bidVolume = {};
    std::array<double, 5> askPrice  = {};
    std::array<double, 5> askVolume = {};

    int64_t   timestamp    = 0;

    bool isValid() const { return contract.isEmpty() == false && lastPrice > 0; }
};
