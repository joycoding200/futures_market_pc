#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QVector>
#include "data/TickData.h"

class TradeRecord : public QWidget {
    Q_OBJECT
public:
    explicit TradeRecord(QWidget* parent = nullptr);
    void addRecord(const TickData& tick, int volume, bool isBuy);

private:
    QTableWidget* m_table;
    static constexpr int MAX_ROWS = 50;
};
