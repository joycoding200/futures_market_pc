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

    m_distLabel = new QLabel("大单:0  中单:0  小单:0");
    m_distLabel->setStyleSheet("color: #888; font-size: 10px; padding: 2px;");
    m_distLabel->setAlignment(Qt::AlignRight);
    layout->addWidget(m_distLabel);
}

void TradeRecord::addRecord(const TickData& tick, int volume) {
    m_table->insertRow(0);

    // 时间列
    auto timeStr = QDateTime::fromMSecsSinceEpoch(tick.timestamp).toString("HH:mm:ss");
    m_table->setItem(0, 0, new QTableWidgetItem(timeStr));

    // 价格列
    auto* priceItem = new QTableWidgetItem(QString::number(tick.lastPrice, 'f', 0));
    m_table->setItem(0, 1, priceItem);

    // 现手列：按成交量大小区分颜色
    // 大单(>=100手): 紫色, 中单(30-99手): 黄色, 小单(<30手): 默认灰色
    auto* volItem = new QTableWidgetItem(QString::number(volume));
    if (volume >= VOL_LARGE) {
        volItem->setForeground(QColor(180, 130, 255));   // 紫色大单
        QFont boldFont = volItem->font();
        boldFont.setBold(true);
        volItem->setFont(boldFont);
    } else if (volume >= VOL_MEDIUM) {
        volItem->setForeground(QColor(255, 215, 0));     // 黄色中单
    }
    m_table->setItem(0, 2, volItem);

    while (m_table->rowCount() > MAX_ROWS) m_table->removeRow(m_table->rowCount() - 1);
}

void TradeRecord::updateVolumeDist(int large, int medium, int small) {
    m_distLabel->setText(QString("大单:%1  中单:%2  小单:%3")
        .arg(large).arg(medium).arg(small));
}
