#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QVector>
#include "data/TickData.h"

class TradeRecord : public QWidget {
    Q_OBJECT
public:
    explicit TradeRecord(QWidget* parent = nullptr);
    void addRecord(const TickData& tick, int volume);
    void updateVolumeDist(int large, int medium, int small);

private:
    QTableWidget* m_table;
    QLabel* m_distLabel;
    static constexpr int MAX_ROWS = 50;
    // 成交量分档阈值
    static constexpr int VOL_LARGE = 100;   // 大单阈值
    static constexpr int VOL_MEDIUM = 30;   // 中单阈值
};
