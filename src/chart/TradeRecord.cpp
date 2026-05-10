#include "TradeRecord.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QDateTime>

TradeRecord::TradeRecord(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_table = new QTableWidget(0, 3);
    m_table->setHorizontalHeaderLabels({"时间", "价格", "现手"});
    m_table->setColumnWidth(0, 60);
    m_table->setColumnWidth(1, 60);
    m_table->setColumnWidth(2, 50);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionMode(QAbstractItemView::NoSelection);
    m_table->setStyleSheet(
        "QTableWidget { background: #161620; color: #ccc; border: none; }"
        "QHeaderView::section { background: #1e1e2a; color: #888; border: none; padding: 2px; font-size: 10px; }"
    );
    layout->addWidget(m_table);
}

void TradeRecord::addRecord(const TickData& tick, int volume, bool isBuy) {
    m_table->insertRow(0);
    auto timeStr = QDateTime::fromMSecsSinceEpoch(tick.timestamp).toString("HH:mm:ss");
    m_table->setItem(0, 0, new QTableWidgetItem(timeStr));
    auto* priceItem = new QTableWidgetItem(QString::number(tick.lastPrice, 'f', 0));
    priceItem->setForeground(isBuy ? QColor(239, 68, 68) : QColor(34, 197, 94));
    m_table->setItem(0, 1, priceItem);
    m_table->setItem(0, 2, new QTableWidgetItem(QString::number(volume)));

    while (m_table->rowCount() > MAX_ROWS) m_table->removeRow(m_table->rowCount() - 1);
}
