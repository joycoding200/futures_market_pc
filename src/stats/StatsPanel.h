#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QTabWidget>
#include "StatsCalculator.h"

class StatsPanel : public QWidget {
    Q_OBJECT
public:
    explicit StatsPanel(StatsCalculator* calc, QWidget* parent = nullptr);

public slots:
    void refresh();

private:
    void setupUI();
    void populateTable(QTableWidget* table, const QVector<ContractStats>& data);

    StatsCalculator* m_calc;
    QTabWidget* m_tabs;
    QTableWidget* m_gainerTable;
    QTableWidget* m_loserTable;
    QTableWidget* m_volumeTable;
};
