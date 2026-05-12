#include "StatsPanel.h"
#include <QVBoxLayout>
#include <QHeaderView>

StatsPanel::StatsPanel(StatsCalculator* calc, QWidget* parent)
    : QWidget(parent), m_calc(calc) {
    setupUI();
}

void StatsPanel::setupUI() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_tabs = new QTabWidget();
    m_tabs->setStyleSheet(
        "QTabWidget::pane { background: #161620; border: 1px solid #333; }"
        "QTabBar::tab { background: #12121a; color: #888; padding: 4px 12px; font-size: 11px; }"
        "QTabBar::tab:selected { background: #1a1a24; color: #fff; }"
    );

    auto createTable = []() {
        auto* t = new QTableWidget(0, 4);
        t->setHorizontalHeaderLabels({QStringLiteral("合约"), QStringLiteral("最新价"), QStringLiteral("涨跌幅"), QStringLiteral("成交量")});
        t->setColumnWidth(0, 60);
        t->setColumnWidth(1, 65);
        t->setColumnWidth(2, 65);
        t->setColumnWidth(3, 65);
        t->verticalHeader()->setVisible(false);
        t->setSelectionMode(QAbstractItemView::SingleSelection);
        t->setEditTriggers(QAbstractItemView::NoEditTriggers);
        t->setStyleSheet(
            "QTableWidget { background: #161620; color: #ccc; border: none; }"
            "QHeaderView::section { background: #1e1e2a; color: #888; border: none; padding: 2px; font-size: 10px; }"
        );
        return t;
    };

    m_gainerTable = createTable();
    m_loserTable = createTable();
    m_volumeTable = createTable();

    m_tabs->addTab(m_gainerTable, QStringLiteral("涨幅榜"));
    m_tabs->addTab(m_loserTable, QStringLiteral("跌幅榜"));
    m_tabs->addTab(m_volumeTable, QStringLiteral("成交量榜"));

    layout->addWidget(m_tabs);
}

void StatsPanel::refresh() {
    populateTable(m_gainerTable, m_calc->topGainers(20));
    populateTable(m_loserTable, m_calc->topLosers(20));
    populateTable(m_volumeTable, m_calc->topVolume(20));
}

void StatsPanel::populateTable(QTableWidget* table, const QVector<ContractStats>& data) {
    table->setRowCount(data.size());
    for (int i = 0; i < data.size(); ++i) {
        const auto& s = data[i];

        auto* codeItem = new QTableWidgetItem(s.contract);
        table->setItem(i, 0, codeItem);

        auto* priceItem = new QTableWidgetItem(QString::number(s.lastPrice, 'f', 0));
        table->setItem(i, 1, priceItem);

        QString changeStr = QString("%1%").arg(s.changeRate, 0, 'f', 2);
        auto* changeItem = new QTableWidgetItem(changeStr);
        // 红涨绿跌
        if (s.changeRate > 0) {
            changeItem->setForeground(QColor(239, 68, 68));
        } else if (s.changeRate < 0) {
            changeItem->setForeground(QColor(34, 197, 94));
        }
        table->setItem(i, 2, changeItem);

        auto* volItem = new QTableWidgetItem(QString::number(static_cast<int>(s.volume)));
        table->setItem(i, 3, volItem);
    }
}
